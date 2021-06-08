#pragma once

#include <iostream>
#include <sstream>
#include <atomic>
#include <optional>
#include <vector>

#include <boost/filesystem.hpp>

#include "options.h"
#include "primitives.h"
#include "status.h"

namespace ashdb
{

namespace bfs = boost::filesystem;

using SegmentIndices = std::vector<std::vector<std::size_t>>;

constexpr auto INDEX_EXTENSION = "idx";
constexpr auto VALIDCHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvqxyz0123456789-_";

std::string BuildFilename(const std::string& folder,
                          const std::string& prefix,
                          const std::string& extension,
                          std::uint64_t fileindex);

std::vector<std::size_t> ReadIndexFile(const std::string& filename);

template<class ThingT>
class AshDB
{

public:
    using Batch = std::vector<ThingT>;
    using BatchIterator = typename std::vector<ThingT>::const_iterator;
    using IndexDetails = std::tuple<std::size_t, std::size_t>;

    AshDB(const std::string& folder, const Options& options)
        : _dbfolder{ folder },
          _options{ options }
    {
        _startIndex.reset();
        _lastIndex.reset();
    }

    [[maybe_unused]] OpenStatus open();
    void close();

    [[maybe_unused]]
    WriteStatus write(const ThingT& thing);

    [[maybe_unused]]
    WriteStatus write(const Batch& batch);

    [[maybe_unused]]
    ThingT read(std::size_t index);

    [[nodiscard]]
    Batch read(std::size_t index, std::size_t count);

    // deletes all records starting at, and includins, `startIndex
    // so that the last index in the database will be `startIndex-1`
    void truncate(std::size_t startIndex);

    // delete everything
    void reset();

    // returns the accessor boundaries ot the database, for example
    // if we use "db->at(i)", these functions tell us the range of "i"
    std::optional<std::size_t> startIndex() const { return _startIndex; }
    std::optional<std::size_t> lastIndex() const { return _lastIndex; }

    // returns the size of all the "data-0001.dat" files on the disk
    // but does NOT include the size of the corresponding index
    // files (i.e. "data-0001.datidx")
    std::uint64_t databaseSize() const;

    // returns the number of records in the database
    std::size_t size() const;

    // data files have names like "data-0001.dat", the numbers returned
    // by these methods represent the "0001" portion of the filename
    std::uint16_t startSegmentNumber() const { return _startSegmentNumber; }
    std::uint16_t activeSegmentNumber() const { return _activeSegmentNumber; }

    // returns the full path of the file tobench which the next record will
    // be written
    std::string activeDataFile() const
    {
        return buildDataFilename(_activeSegmentNumber);
    }

    // returns the full path of the file to which the next index entry
    // will be written
    std::string activeIndexFile() const
    {
        return buildIndexFilename(_activeSegmentNumber);
    }

    // records the vector of vector that keeps track of all the records
    // and their offsets
    const SegmentIndices segmentIndices() const { return _segmentIndices; }

private:
    // scans the database folder to establish the min and max records for
    // the data files. For example, if we have the files data-00002.dat,
    // data-00003.dat, data-00004.dat, then it will set the start and
    // active record numbers to 2 and 4 respectively
    void findFileBoundaries();

    // find the boundaries of the accessor methods, i.e. we access
    // the records in the database through `at(i)` and this will
    // find the valid range of values of `i`. NOTE: This should
    // be called AFTER `_segmentIndices` has been setup
    void findIndexBoundaries();

    // move the index forward by (1) incrementing the active record number,
    // (2) deleting the oldest index and data files if necessary and (3)
    // cleaning up `_startIndex` and `_lastIndex` as needed
    void updateIndexing();

    // writes the given offset to the current index file and will update
    // "_segmentIndices" accordingly
    void writeIndexEntry(std::size_t offset);

    // writes records to the current data file until the begin == end or
    // until the dat file exceeds the max file size
    void writeBatchUntilFull(BatchIterator& begin, BatchIterator end);

    std::string buildDataFilename(std::uint16_t x) const;
    std::string buildIndexFilename(std::uint16_t x) const;

    IndexDetails findIndexDetails(std::size_t index);

    //////////////////////////////////////////////
    // private variables
    const std::string       _dbfolder;
    const Options           _options;

    SegmentIndices             _segmentIndices;

    // the first and last accessors of the records using `at()` or `operator[]`
    std::optional<std::size_t>  _startIndex = 0;
    std::optional<std::size_t>  _lastIndex = 0;

    // the current first and last file numbers
    std::uint16_t           _startSegmentNumber = 0;
    std::uint16_t           _activeSegmentNumber = 0;

    std::mutex              _readWriteMutex;

    std::atomic_bool        _open = false;
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

    bfs::path dbpath{ _dbfolder };
    if (!bfs::exists(dbpath))
    {
        if (_options.create_if_missing)
        {
            bfs::create_directories(dbpath);
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
    _segmentIndices.clear();
    for (auto i = _startSegmentNumber; i <= _activeSegmentNumber; ++i)
    {
        const auto indexFilename = buildIndexFilename(i);
        if (bfs::exists(indexFilename))
        {
            _segmentIndices.push_back({});
            _segmentIndices.back() = ashdb::ReadIndexFile(indexFilename);
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
        return WriteStatus::DATABASE_NOT_OPEN;
    }

    const auto datafile = this->activeDataFile();
    auto destfilesize = bfs::exists(datafile)
            ? bfs::file_size(datafile.data()) : 0u;

    // write the offset of the current filesize, since this marks the beginning
    // of *this* record
    writeIndexEntry(destfilesize);

    std::ofstream datafs;
    datafs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    datafs.open(datafile.data(), std::ios::out | std::ios::binary | std::ios::app);
    ashdb_write(datafs, thing);
    datafs.close();

    // now that we've written the data, check the filesize once again
    destfilesize = bfs::file_size(datafile.data());
    updateIndexing();
    if (_options.filesize_max > 0 && destfilesize >= _options.filesize_max)
    {
        _activeSegmentNumber++;
    }

    return WriteStatus::OK;
}

template<class ThingT>
void AshDB<ThingT>::writeBatchUntilFull(BatchIterator& begin, BatchIterator end)
{
    const auto datafile = this->activeDataFile();

    // we have to manually keep track of the file offets by using the current
    // filesize and the size of the data we're writing
    const std::size_t startingOffset = bfs::exists(datafile) ?
            bfs::file_size(datafile) : 0;
    std::size_t currentOffset = startingOffset;

    // we have two buffers: (1) for the data that we won't flush to disk until
    // we've written as much as we can into this segment, and (2) for the index
    // offsets which we also won't flush until the very end
    std::stringstream indexBuffer;
    std::stringstream buffer;

    while (begin != end)
    {
        // If the currentOffset == 0 this means this is the first time we're writing to
        // this segment. The first entry in an index file is not an offset, since it's
        // unnecessary because we know the corresponding offset is also 0, but instead
        // is the index number of the first entry in this segment
        if (currentOffset == 0)
        {
            if (_segmentIndices.size() > 1)
            {
                const auto& secondToLast = _segmentIndices.end()[-2];

                // under these conditions, `currentOffset` actually represents the
                // index number of the first item in this segment's datafile
                currentOffset = secondToLast.at(0) + secondToLast.size();
            }
        }

        _segmentIndices.back().push_back(currentOffset);
        indexBuffer.write(reinterpret_cast<char*>(&currentOffset), sizeof(currentOffset));

        ashdb_write(buffer, *begin);
        currentOffset = startingOffset + buffer.tellp();
        ++begin;

        if (BOOST_LIKELY(_startIndex.has_value()))
        {
            (*_lastIndex)++;
        }
        else
        {
            // this condition should only happen when the database is brand new
            assert(!_lastIndex.has_value());
            _startIndex = 0;
            _lastIndex = 0;
        }

        if (_options.filesize_max > 0 &&  currentOffset > _options.filesize_max)
        {
            break;
        }
    }

    if (buffer.tellp() > 0)
    {
        assert(indexBuffer.tellp() > 0);

        const std::string indexFile = activeIndexFile();
        std::ofstream indexfs;
        indexfs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        indexfs.open(indexFile.c_str(), std::ios::out | std::ios::binary | std::ios::app);
        indexfs << indexBuffer.str();
        // deepcode ignore MissingOpenCheckOnFile: `indexfs` has exceptions turned on
        indexfs.close();

        std::ofstream datsafs;
        datsafs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        datsafs.open(datafile.data(), std::ios::out | std::ios::binary | std::ios::app);
        // deepcode ignore MissingOpenCheckOnFile: `datsafs` has exceptions turned on
        datsafs << buffer.str();
    }
}

template<class ThingT>
WriteStatus AshDB<ThingT>::write(const AshDB<ThingT>::Batch& batch)
{
    std::scoped_lock lock{_readWriteMutex};
    if (!_open)
    {
        return WriteStatus::DATABASE_NOT_OPEN;
    }

    BatchIterator begin = batch.begin();
    BatchIterator end = batch.end();

    while (begin != end)
    {
        // before we get started, first see if we have to add
        // another segment index
        const auto segmentCount = (_activeSegmentNumber - _startSegmentNumber) + 1;
        if (_segmentIndices.size() == 0
            || _segmentIndices.size() < segmentCount)
        {
            assert(_segmentIndices.size() == segmentCount - 1);
            _segmentIndices.push_back({});
        }

        // write as much as we can to the current segment
        writeBatchUntilFull(begin, end);

        // in case we stopped writing because the segment file got too big
        // we need to increment the segment number
        if ((begin != end) ||
            (_options.filesize_max > 0 && (bfs::file_size(activeDataFile()) > _options.filesize_max)))
        {
            _activeSegmentNumber++;
        }

        // now see if the database is too big and we need to trim it down
        if (_options.database_max > 0
            && databaseSize() > _options.database_max)
        {
            const auto fn = buildDataFilename(_startSegmentNumber);
            const auto ifn = buildIndexFilename(_startSegmentNumber);
            bfs::remove(fn);
            bfs::remove(ifn);

            _startSegmentNumber++;
            _segmentIndices.erase(_segmentIndices.begin());
            _startIndex = _segmentIndices.front().at(0);
        }
    }

    return ashdb::WriteStatus::OK;
}

template<class ThingT>
ThingT AshDB<ThingT>::read(std::size_t index)
{
    std::scoped_lock lock{_readWriteMutex};

    auto [currentRecord, localIndex] = findIndexDetails(index);
    auto readOffset = localIndex == 0 ? 0 : _segmentIndices[currentRecord][localIndex];

    const auto dataFile = buildDataFilename(currentRecord);
    std::ifstream datafs;
    datafs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    datafs.open(dataFile.data(), std::ios_base::binary);

    datafs.seekg(readOffset);

    ThingT thing;
    ashdb_read(datafs, thing);
    datafs.close();

    return thing;
}

template<class ThingT>
auto AshDB<ThingT>::read(std::size_t index, std::size_t count) -> AshDB<ThingT>::Batch
{
    std::scoped_lock lock{_readWriteMutex};

    AshDB<ThingT>::Batch batch;
    const auto endIndex = index + count;
    auto [segment, offsetIndex] = findIndexDetails(index);

    for (auto i = index; i < (index + count);)
    {
        auto localMax = _segmentIndices[segment].size();
        if (endIndex <= (_segmentIndices[segment][0] + _segmentIndices[segment].size()))
        {
            localMax = endIndex - _segmentIndices[segment][0];
        }

        const auto datafile = this->buildDataFilename(segment);
        std::ifstream datafs;
        datafs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        datafs.open(datafile.data(), std::ios_base::binary);

        for (; offsetIndex < localMax; ++i, ++offsetIndex)
        {
            auto readOffset = offsetIndex == 0 ? 0 : _segmentIndices[segment][offsetIndex];
            datafs.seekg(readOffset);

            ThingT thing;
            ashdb_read(datafs, thing);
            batch.push_back(std::move(thing));
        }

        datafs.close();
        offsetIndex = 0;
        segment++;
    }

    return batch;
}

template<class ThingT>
void AshDB<ThingT>::truncate(std::size_t startIndex)
{
    std::scoped_lock lock{_readWriteMutex};

    auto [currentRecord, localIndex] = findIndexDetails(startIndex);

}

template<class ThingT>
void AshDB<ThingT>::reset()
{
    std::scoped_lock lock{_readWriteMutex};
}

template<class ThingT>
void AshDB<ThingT>::findFileBoundaries()
{
    bfs::path dbpath{_dbfolder};

    // early fail if the folder is empty
    if (bfs::is_empty(dbpath))
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
        if (bfs::exists(filename))
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

    _startSegmentNumber = start;
    _activeSegmentNumber = std::max(_startSegmentNumber, end);

    if (const auto datafile = activeDataFile();
        _options.filesize_max != 0
        && bfs::exists(datafile.data())
        && bfs::file_size(datafile.data()) >= _options.filesize_max)
    {
        _activeSegmentNumber++;
    }
}

template<class ThingT>
void AshDB<ThingT>::findIndexBoundaries()
{
    _startIndex.reset();
    _lastIndex.reset();

    // load the accessor boundaries
    if (_segmentIndices.size() > 0)
    {
        assert(_segmentIndices.front().size() > 0);
        _startIndex = _segmentIndices.front().at(0);

        assert(_segmentIndices.back().size() > 0);
        _lastIndex = _segmentIndices.back().front() + (_segmentIndices.back().size() - 1);
    }
}

template<class ThingT>
void AshDB<ThingT>::updateIndexing()
{
    if (!_startIndex.has_value())
    {
        assert(!_lastIndex.has_value());
        _startIndex = 0;
        _lastIndex = 0;
    }
    else
    {
        (*_lastIndex)++;
    }

    // now see if the database is too big and we need to trim it down
    if (_options.database_max > 0
        && databaseSize() > _options.database_max)
    {
        const auto fn = buildDataFilename(_startSegmentNumber);
        const auto ifn = buildIndexFilename(_startSegmentNumber);
        bfs::remove(fn);
        bfs::remove(ifn);

        _startSegmentNumber++;
        _segmentIndices.erase(_segmentIndices.begin());
        _startIndex = _segmentIndices.front().at(0);
    }
}

template<class ThingT>
void AshDB<ThingT>::writeIndexEntry(std::size_t offset)
{
    auto value = offset;

    if (value == 0)
    {
        if (_segmentIndices.size() > 0)
        {
            value = _segmentIndices.back().at(0) + _segmentIndices.back().size();
        }
    }

    std::string indexfile = activeIndexFile();
    std::ofstream indexfs;
    indexfs.exceptions(std::ofstream::badbit | std::ofstream::failbit);
    indexfs.open(indexfile.c_str(), std::ios::out | std::ios::binary | std::ios::app);
    ashdb::ashdb_write(indexfs, value);
    indexfs.close();

    const auto segmentCount = (_activeSegmentNumber - _startSegmentNumber) + 1;
    if (_segmentIndices.size() < segmentCount)
    {
        assert(_segmentIndices.size() == segmentCount - 1);
        _segmentIndices.push_back({});
    }

    _segmentIndices.back().push_back(value);
}

template<class ThingT>
std::uint64_t AshDB<ThingT>::databaseSize() const
{
    std::uint64_t retval = 0;
    for (auto i = _startSegmentNumber; i <= _activeSegmentNumber; ++i)
    {
        const auto filename = ashdb::BuildFilename(
                _dbfolder, _options.prefix, _options.extension, i);

        bfs::path filepath{filename};

        if (!bfs::exists(filepath))
        {
            assert(i == _activeSegmentNumber);
            break;
        }

        retval += static_cast<std::uint64_t>(bfs::file_size(filepath));
    }
    
    return retval;
}

template<class ThingT>
std::size_t AshDB<ThingT>::size() const
{
    if (!_open || !_startIndex.has_value())
    {
        assert(!_lastIndex.has_value());
        return 0;
    }

    return (*_lastIndex - *_startIndex) + 1;
}

// TODO: this could be improved
template<class ThingT>
typename AshDB<ThingT>::IndexDetails AshDB<ThingT>::findIndexDetails(std::size_t index)
{
    if (index < _startIndex || index > _lastIndex)
    {
        std::stringstream ss;
        ss << "index " << index << " is out of bounds";
        throw std::runtime_error(ss.str());
    }

    std::optional<std::size_t> localIndex = 0;
    std::size_t currentRecord = _startSegmentNumber;
    for (const auto& offsets : _segmentIndices)
    {
        if (index < (offsets.front() + offsets.size()))
        {
            localIndex = index - offsets.front();
            break;
        }
        else
        {
            currentRecord++;
        }
    }

    if (!localIndex.has_value())
    {
        std::stringstream ss;
        ss << "index " << index << " could not be found";
        throw std::runtime_error(ss.str());
    }

    return IndexDetails{ currentRecord, *localIndex };
}

template<class ThingT>
std::string AshDB<ThingT>::buildDataFilename(std::uint16_t x) const
{
    return ashdb::BuildFilename(_dbfolder, _options.prefix, _options.extension, x);
}

template<class ThingT>
std::string AshDB<ThingT>::buildIndexFilename(std::uint16_t x) const
{
    const auto temp = _options.extension + INDEX_EXTENSION;
    return ashdb::BuildFilename(_dbfolder, _options.prefix, temp, x);
}

}; // namespace ashdb
