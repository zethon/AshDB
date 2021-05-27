#include <limits>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


#include "../include/ashdb/ashdb.h"

namespace ashdb
{

std::string BuildFilename(const std::string& folder,
                          const std::string& prefix,
                          const std::string& extension,
                          std::uint64_t fileindex)
{
    if (const auto limit = std::numeric_limits<std::uint16_t>::max(); fileindex > limit)
    {
        std::stringstream error;
        error << "file index " << fileindex << " exceeds supported limit " << limit;
        throw std::runtime_error(error.str());
    }

    std::stringstream ss;
    ss << std::setw(5) << std::setfill('0') << fileindex;

    const std::string filename = prefix + "-" + ss.str() + "." + extension;

    boost::filesystem::path path { folder };
    path /= filename;
    return path.string();
}

std::string BuildWildcard(const std::string& folder,
                          const std::string& pattern)
{
    std::string temp { pattern };
    boost::replace_all(temp, "%d", "*");

    boost::filesystem::path path { folder };
    path /= temp;
    return path.string();
}


} // namespace ashdb
