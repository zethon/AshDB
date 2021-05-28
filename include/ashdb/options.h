#pragma once

namespace ashdb
{
    
struct Options
{
    bool create_if_missing = true;
    bool error_if_exists = false;

    std::uint64_t filesize_max = 0;
    std::uint64_t database_max = 0;

    std::string prefix = "data";
    std::string extension = "ash";
};

} // namespace ashdb