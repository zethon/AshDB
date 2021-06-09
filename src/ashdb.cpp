#include <limits>
#include <iomanip>

#include <boost/algorithm/string.hpp>
//#include <boost/filesystem.hpp>

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

    fs::path path {folder };
    path /= filename;
    return path.string();
}

std::vector<std::size_t> ReadIndexFile(const std::string& filename)
{
    std::vector<std::size_t> retval;

    if (!fs::exists(filename))
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

} // namespace ashdb
