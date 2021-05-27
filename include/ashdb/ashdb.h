#pragma once

#include <iostream>

#include <boost/filesystem.hpp>

#include "primitives.h"

namespace ashdb
{

enum class OpenStatus
{
    OK,
    EXISTS,
    NOT_FOUND,
    INVALID_PREFIX,
    INVALID_EXTENSION,
    ALREADY_OPEN
};

inline std::string ToString(OpenStatus status)
{
    switch (status)
    {
        default:
            throw std::runtime_error("invalid OpenStatus");
        case OpenStatus::OK:
            return "OK";
        case OpenStatus::EXISTS:
            return "EXISTS";
        case OpenStatus::NOT_FOUND:
            return "NOT_FOUND";
        case OpenStatus::INVALID_PREFIX:
            return "INVALID_PREFIX";
        case OpenStatus::INVALID_EXTENSION:
            return "INVALID_EXTENSION";
        case OpenStatus::ALREADY_OPEN:
            return "ALREADY_OPEN";
    }
}

enum class WriteStatus
{
    OK,
    NOT_OPEN
};

inline std::string ToString(WriteStatus status)
{
    switch (status)
    {
        default:
            throw std::runtime_error("invalid WriteStatus");
        case WriteStatus::OK:
            return "OK";
        case WriteStatus::NOT_OPEN:
            return "EXISTS";
    }
}

struct Options
{
    bool create_if_missing = true;
    bool error_if_exists = false;

    std::uint64_t filesize_max = 0;
    std::uint64_t database_max = 0;

    std::string prefix = "data";
    std::string extension = "ash";
};

std::string BuildFilename(const std::string& folder,
                          const std::string& prefix,
                          const std::string& extension,
                          std::uint64_t fileindex);

std::string BuildWildcard(const std::string& folder,
                          const std::string& pattern);

constexpr auto INDEX_EXTENSION = "idx";
constexpr auto VALIDCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvqxyz0123456789-_";

template<class ThingT>
class AshDB
{
    const std::uint32_t _version = 1;
    
    const std::string   _dbfolder;
    const Options       _options;

    std::uint16_t       _startIndex = 0;
    std::uint16_t       _activeIndex = 0;

    std::mutex          _readWriteMutex;

    std::atomic_bool    _open = false;

public:

    AshDB(const std::string& folder, const Options& options)
        : _dbfolder{ folder },
          _options{ options }
    {
        // nothing to do (yet)
    }

    OpenStatus open();

    WriteStatus write(const ThingT& thing);
    ThingT read(std::size_t index);

    std::uint64_t databaseSize() const;

    std::uint16_t startIndex() const { return _startIndex; }
    std::uint16_t activeIndex() const { return _activeIndex; }

    std::string activeDataFile() const
    {
        return ashdb::BuildFilename(
                _dbfolder, _options.prefix, _options.extension, _activeIndex);
    }

    std::string activeIndexFile() const
    {
        return ashdb::BuildFilename(
                _dbfolder, _options.prefix, INDEX_EXTENSION, _activeIndex);

    }

    void close()
    {
        std::scoped_lock lock{_readWriteMutex};
        _open = false;
    }

private:
    void findFileBoundaries();

};

template<class ThingT>
OpenStatus AshDB<ThingT>::open()
{
    if (_open)
    {
        return OpenStatus::ALREADY_OPEN;
    }

    if (_options.prefix.size() == 0
        || _options.prefix.find_first_not_of(VALIDCHARS) != std::string::npos)
    {
        return OpenStatus::INVALID_PREFIX;
    }
    else if (_options.extension.size() == 0
        || _options.extension == ashdb::INDEX_EXTENSION
        || _options.extension.find_first_not_of(VALIDCHARS) != std::string::npos)
    {
        return OpenStatus::INVALID_EXTENSION;
    }

    std::scoped_lock lock{_readWriteMutex};

    boost::filesystem::path dbpath{ _dbfolder };
    if (!boost::filesystem::exists(dbpath))
    {
        if (_options.create_if_missing)
        {
            boost::filesystem::create_directories(dbpath);
        }
        else
        {
            return OpenStatus::NOT_FOUND;
        }
    }
    else if (_options.error_if_exists)
    {
        return OpenStatus::EXISTS;
    }

    findFileBoundaries();


    _open = true;
    return OpenStatus::OK;
}

template<class ThingT>
WriteStatus AshDB<ThingT>::write(const ThingT& thing)
{
    std::scoped_lock lock{_readWriteMutex};
    if (!_open)
    {
        return WriteStatus::NOT_OPEN;
    }

    const auto datafile = this->activeDataFile();
    std::ofstream ofs(datafile.data(), std::ios::out | std::ios::binary | std::ios::app);
    ashdb::ashdb_write(ofs, thing);
    ofs.close();

    if (_options.filesize_max > 0
        && boost::filesystem::file_size(datafile.data()) >= _options.filesize_max)
    {
        _activeIndex++;
    }

    if (_options.database_max > 0 && databaseSize() > _options.database_max)
    {
        const auto fn = ashdb::BuildFilename(
            _dbfolder, _options.prefix, _options.extension, _startIndex);

        boost::filesystem::remove(fn);
        _startIndex++;
    }

    return WriteStatus::OK;
}

template<class ThingT>
ThingT AshDB<ThingT>::read(std::size_t index)
{
    std::scoped_lock lock{_readWriteMutex};
    return {};
}

template<class ThingT>
void AshDB<ThingT>::findFileBoundaries()
{
    boost::filesystem::path dbpath{ _dbfolder };

    // early fail if the folder is empty
    if (boost::filesystem::is_empty(dbpath))
    {
        return;
    }

    // there is no portable glob method, so for now we're going to use this
    // brute force and ugly method to get the boundaries
    bool found = false;
    std::uint16_t start = 0;
    std::uint16_t end = 0;

    for (std::uint16_t i = 0u; i < std::numeric_limits<std::uint16_t>::max(); ++i)
    {
        auto filename = ashdb::BuildFilename(_dbfolder, _options.prefix, _options.extension, i);
        if (boost::filesystem::exists(filename))
        {
            if (!found)
            {
                start = i;
                found = true;
            }
        }
        else if (found)
        {
            assert(i > 0);
            end = i - 1;
            break;
        }
    }

    _startIndex = start;
    _activeIndex = std::max(_startIndex, end);

    if (const auto datafile = activeDataFile();
        _options.filesize_max != 0 && boost::filesystem::file_size(datafile.data()) >= _options.filesize_max)
    {
        _activeIndex++;
    }
}

template<class ThingT>
std::uint64_t AshDB<ThingT>::databaseSize() const
{
    std::uint64_t retval = 0;
    for (auto i = _startIndex; i <= _activeIndex; ++i)
    {
        const auto filename = ashdb::BuildFilename(
                _dbfolder, _options.prefix, _options.extension, i);

        retval += static_cast<std::uint64_t>(boost::filesystem::file_size(filename));
    }
    return retval;
}

}; // namespace ashdb
