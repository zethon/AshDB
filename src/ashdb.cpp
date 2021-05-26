#include <limits>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


#include "../include/ashdb/ashdb.h"

namespace ashdb
{

std::string GenerateFilename(const std::string& folder, const std::string& pattern, std::uint64_t fileindex)
{
    if (const auto limit = std::numeric_limits<std::uint16_t>::max(); fileindex > limit)
    {
        std::stringstream error;
        error << "file index " << fileindex << " exceeds supported limit " << limit;
        throw std::runtime_error(error.str());
    }

    std::stringstream ss;
    ss << std::setw(5) << std::setfill('0') << fileindex;

    std::string temp { pattern };
    boost::replace_all(temp, "%d", ss.str());

    boost::filesystem::path path { folder };
    path /= temp;
    return path.string();
}



} // namespace ashdb
