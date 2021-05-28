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

using IndexRecord = std::vector<std::vector<std::size_t>>;

template<class ThingT>
class AshDB
{
    const std::string       _dbfolder;
    const Options           _options;

    IndexRecord             _indexRecord;

    // the first and last accessors of the records using `at()` or `operator[]`
    std::optional<std::size_t>  _startIndex = 0;
    std::optional<std::size_t>  _lastIndex = 0;

    // the current first and last file numbers
    std::uint16_t           _startFileNumber = 0;
    std::uint16_t           _activeFileNumber = 0;

    std::mutex              _readWriteMutex;

    std::atomic_bool        _open = false;

public:

    AshDB(const std::string& folder, const Options& options)
        : _dbfolder{ folder },
          _options{ options }
    {
        _startIndex.reset();
        _lastIndex.reset();
    }

    OpenStatus open();
    void close();

    WriteStatus write(const ThingT& thing);
    ThingT read(std::size_t index);

    // returns the accessor boundaries ot the database, for example
    // if we use "db->at(i)", these functions tell us the range of "i"
    auto startIndex() const { return _startIndex; }
    auto lastIndex() const { return _lastIndex; }

    // returns the size of all the "data-0001.dat" files on the disk
    // but does NOT include the size of the corresponding index
    // files (i.e. "data-0001.datidx")
    std::uint64_t databaseSize() const;

    // returns the number of records in the database
    std::size_t size() const
    {
        if (!_open || !_startIndex.has_value())
        {
            assert(!_lastIndex.has_value());
            return 0;
        }

        return (*_lastIndex - *_startIndex) + 1;
    }

    // data files have names like "data-0001.dat", the numbers returned
    // by these methods represent the "0001" portion of the filename
    std::uint16_t startRecordNumber() const { return _startFileNumber; }
    std::uint16_t activeRecordNumber() const { return _activeFileNumber; }

    // returns the full path of the file to which the next record will
    // be written
    std::string activeDataFile() const
    {
        return buildDataFilename(_activeFileNumber);
    }

    // returns the full path of the file to which the next index entry
    // will be written
    std::string activeIndexFile() const
    {
        return buildIndexFilename(_activeFileNumber);
    }

    // records the vector of vector that keeps track of all the records
    // and their offsets
    const IndexRecord indexRecord() const { return _indexRecord; }

private:
    void findFileBoundaries();

    // find the boundaries of the accessor methods, i.e. we access
    // the records in the database through `at(i)` and this will
    // find the valid range of values of `i`. NOTE: This should
    // be called AFTER `_indexRecord` has been setup
    void findIndexBoundaries();

    bool writeIndexEntry(std::size_t offset)
    {
        std::string temp = activeIndexFile();
        std::ofstream ofs(temp.data(), std::ios::out | std::ios::binary | std::ios::app);
        if (!ofs.is_open())
        {
            return false;
        }

        ashdb::ashdb_write(ofs, offset);
        ofs.close();

        const auto recordCount = ((_activeFileNumber - _startFileNumber) + 1);
        if (_indexRecord.size() < recordCount)
        {
            assert(_indexRecord.size() == recordCount - 1);
            _indexRecord.push_back({});
        }

        _indexRecord.back().push_back(offset);

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

    // load the record-index
    _indexRecord.clear();
    for (auto i = _startFileNumber; i <= _activeFileNumber; ++i)
    {
        const auto indexFilename = buildIndexFilename(i);
        if (boost::filesystem::exists(indexFilename))
        {
            _indexRecord.push_back({});
            _indexRecord.back() = ashdb::ReadIndexFile(indexFilename);
        }
    }

    findIndexBoundaries();

    _open = true;
    return OpenStatus::OK;
}

template<class ThingT>
void AshDB<ThingT>::close()
{
    std::scoped_lock lock{_readWriteMutex};
    _startIndex.reset();
    _lastIndex.reset();
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
        auto value = 0;
        if (_indexRecord.size() > 0)
        {
            value = _indexRecord.back().at(0) + _indexRecord.back().size();
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

    if (_options.database_max > 0
        && databaseSize() > _options.database_max)
    {
        const auto fn = buildDataFilename(_startFileNumber);
        const auto ifn = buildIndexFilename(_startFileNumber);
        boost::filesystem::remove(fn);
        boost::filesystem::remove(ifn);

        _startFileNumber++;
        _indexRecord.erase(_indexRecord.begin());
    }

    // we should be able to assume that `_indexRecord` has some values now
    // TODO: this can probably be refactored such that each item is updated
    //       with the `+` operator individually, which might be faster than
    //       what we're doing here, but this will do for now
    _startIndex = _indexRecord.front().at(0);
    _lastIndex = _indexRecord.back().front() + (_indexRecord.back().size() - 1);

    return WriteStatus::OK;
}

template<class ThingT>
ThingT AshDB<ThingT>::read(std::size_t index)
{
    std::scoped_lock lock{_readWriteMutex};
    if (index < _startIndex || index > _lastIndex)
    {
        std::stringstream ss;
        ss << "index " << index << " is out of bounds";
        throw std::runtime_error(ss.str());
    }

    // TODO: this is awful and needs to be refactored, but I just want to get
    // the unit tests in place, so it will do for now
    std::optional<std::size_t> readOffset;
    auto currentRecord = _startFileNumber;
    for (const auto& offsets : _indexRecord)
    {
        if (index < (offsets.front() + offsets.size()))
        {
            auto localIndex = index - offsets.front();
            if (localIndex == 0)
            {
                // the first offset of the record-index tells us the index number
                // of the first item in that index file and not the offset, we can
                // safely assume the offset of the first item in a data file is always 0
                readOffset = 0;
            }
            else
            {
                readOffset = offsets.at(localIndex);
            }
            break;
        }
        else
        {
            currentRecord++;
        }
    }

    if (readOffset.has_value())
    {
        const auto dataFile = buildDataFilename(currentRecord);
        if (std::ifstream ifs(dataFile.data(), std::ios_base::binary); ifs.is_open())
        {
            ifs.seekg(*readOffset);

            ThingT thing;
            ashdb::ashdb_read(ifs, thing);
            return thing;
        }
    }

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
void AshDB<ThingT>::findIndexBoundaries()
{
    _startIndex.reset();
    _lastIndex.reset();

    // load the accessor boundaries
    if (_indexRecord.size() > 0)
    {
        assert(_indexRecord.front().size() > 0);
        _startIndex = _indexRecord.front().at(0);

        assert(_indexRecord.back().size() > 0);
        _lastIndex = _indexRecord.back().front() + (_indexRecord.back().size() - 1);
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
