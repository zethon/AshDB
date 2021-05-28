#pragma once

#include <iostream>

#include <boost/filesystem.hpp>

#include "options.h"
#include "primitives.h"
#include "status.h"

namespace ashdb
{

constexpr auto INDEX_EXTENSION = "idx";
constexpr auto VALIDCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvqxyz0123456789-_";

std::string BuildFilename(const std::string& folder,
                          const std::string& prefix,
                          const std::string& extension,
                          std::uint64_t fileindex);

inline std::vector<std::size_t> ReadIndexFile(const std::string& filename)
{
    std::vector<std::size_t> retval;

    if (!boost::filesystem::exists(filename))
    {
        return {};
    }

    if (std::ifstream ifs(filename.data(), std::ios_base::binary);
        ifs.is_open())
    {
        while (ifs.peek() != EOF)
        {
            std::size_t value;
            ashdb_read(ifs, value);
            retval.push_back(value);
        }
    }

    return retval;
}

using RecordIndex = std::vector<std::vector<std::size_t>>;

template<class ThingT>
class AshDB
{
    const std::string       _dbfolder;
    const Options           _options;

    RecordIndex             _recordIndex;

    std::uint16_t           _startFileNumber = 0;
    std::uint16_t           _activeFileNumber = 0;

    std::mutex              _readWriteMutex;

    std::atomic_bool        _open = false;

public:

    AshDB(const std::string& folder, const Options& options)
        : _dbfolder{ folder },
          _options{ options }
    {
        // nothing to do (yet)
    }

    OpenStatus open();
    void close();

    WriteStatus write(const ThingT& thing);
    ThingT read(std::size_t index);

    std::uint64_t databaseSize() const;

    std::uint16_t startIndex() const { return _startFileNumber; }
    std::uint16_t activeIndex() const { return _activeFileNumber; }

    std::string activeDataFile() const
    {
        return buildDataFilename(_activeFileNumber);
    }

    std::string activeIndexFile() const
    {
        return buildIndexFilename(_activeFileNumber);
    }

    const RecordIndex recordIndex() const { return _recordIndex; }

private:
    void findFileBoundaries();

    bool writeIndexEntry(std::size_t offset)
    {
        std::string temp = activeIndexFile();
        std::ofstream ofs(temp.data(), std::ios::out | std::ios::binary | std::ios::app);
        if (!ofs.is_open())
        {
            return false;
        }

        ashdb::ashdb_write(ofs, offset);

        const auto recordCount = ((_activeFileNumber - _startFileNumber) + 1);
        if (_recordIndex.size() != _activeFileNumber + 1)
        {
            assert(_recordIndex.size() == _activeFileNumber);
            _recordIndex.push_back({});
        }

        _recordIndex.back().push_back(offset);

        return true;
    }

    std::string buildDataFilename(std::uint16_t x) const
    {
        return ashdb::BuildFilename(_dbfolder, _options.prefix, _options.extension, x);
    }

    std::string buildIndexFilename(std::uint16_t x) const
    {
        const auto temp = _options.extension + INDEX_EXTENSION;
        return ashdb::BuildFilename(_dbfolder, _options.prefix, temp, x);
    }
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

    _recordIndex.clear();
    for (auto i = _startFileNumber; i <= _activeFileNumber; ++i)
    {
        const auto indexFilename = buildIndexFilename(i);
        if (boost::filesystem::exists(indexFilename))
        {
            _recordIndex.push_back({});
            _recordIndex.back() = ashdb::ReadIndexFile(indexFilename);
        }
    }

    _open = true;
    return OpenStatus::OK;
}

template<class ThingT>
void AshDB<ThingT>::close()
{
    std::scoped_lock lock{_readWriteMutex};
    _open = false;
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
    auto destfilesize = boost::filesystem::exists(datafile)
            ? boost::filesystem::file_size(datafile.data()) : 0u;

    // let's write the index before we write the data
    if (destfilesize == 0)
    {
        // this is the first entry in the index, so we need to write out the
        // index number of the item instead of the offset
        std::size_t value = 0;
        for (auto& v : _recordIndex)
        {
            value += v.size();
        }

        writeIndexEntry(value);
    }
    else
    {
        writeIndexEntry(destfilesize);
    }

    std::ofstream ofs(datafile.data(), std::ios::out | std::ios::binary | std::ios::app);
    ashdb::ashdb_write(ofs, thing);
    ofs.close();

    // now that we've written the data, check the filesize once again
    destfilesize = boost::filesystem::file_size(datafile.data());
    if (_options.filesize_max > 0 && destfilesize >= _options.filesize_max)
    {
        _activeFileNumber++;
    }

    if (_options.database_max > 0 && databaseSize() > _options.database_max)
    {
        const auto fn = ashdb::BuildFilename(
            _dbfolder, _options.prefix, _options.extension, _startFileNumber);

        boost::filesystem::remove(fn);
        _startFileNumber++;
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
    boost::filesystem::path dbpath{_dbfolder};

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

    _startFileNumber = start;
    _activeFileNumber = std::max(_startFileNumber, end);

    if (const auto datafile = activeDataFile();
        _options.filesize_max != 0
        && boost::filesystem::exists(datafile.data())
        && boost::filesystem::file_size(datafile.data()) >= _options.filesize_max)
    {
        _activeFileNumber++;
    }
}

template<class ThingT>
std::uint64_t AshDB<ThingT>::databaseSize() const
{
    std::uint64_t retval = 0;
    for (auto i = _startFileNumber; i <= _activeFileNumber; ++i)
    {
        const auto filename = ashdb::BuildFilename(
                _dbfolder, _options.prefix, _options.extension, i);

        boost::filesystem::path filepath{filename};

        if (!boost::filesystem::exists(filepath))
        {
            assert(i == _activeFileNumber);
            break;
        }

        retval += static_cast<std::uint64_t>(boost::filesystem::file_size(filepath));
    }
    return retval;
}

}; // namespace ashdb
