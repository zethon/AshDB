#pragma once

namespace ashdb
{

struct Options
{
    std::uint64_t filesize_max = 0;
    std::uint64_t database_max = 0;
};

template<class ThingT>
class AshDB
{
    std::uint32_t   _version = 1;
    
    const std::string _dbfolder;
    const std::string _filepattern;

    const std::uint64_t _filesizeMax = 0;
    const std::uint64_t _databaseMax = 0;

public:
    AshDB(const std::string& folder, const std::string& pattern, const Options& options)
        : _dbfolder{ folder },
          _filepattern{ pattern },
          _filesizeMax{ options.filesize_max },
          _databaseMax{ options.database_max }
    {
        // nothing to do (yet)
    }


    std::uint32_t version() const 
    { 
        return _version; 
    }

    void write(const ThingT& thing)
    {
    }

    void close()
    {
    }

};

}; // namespace ashdb
