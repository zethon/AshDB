#pragma once
#include <iostream>

#include <boost/test/data/test_case.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

#include "../include/ashdb/ashdb.h"

namespace std
{

std::ostream& operator<<(std::ostream& out, ashdb::OpenStatus status)
{
    out << ashdb::ToString(status);
    return out;
}

std::ostream& operator<<(std::ostream& out, ashdb::WriteStatus status)
{
    out << ashdb::ToString(status);
    return out;
}

}


namespace ashdb::test
{

boost::filesystem::path tempFolder()
{
    auto temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("ashdb%%%%%%");
    temp /= std::to_string(boost::unit_test::framework::current_test_case().p_id);
    boost::filesystem::create_directories(temp);
    return temp;
}

boost::filesystem::path tempFolder(const std::string& subfolder)
{
    auto temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("ashdb%%%%%%");
    temp /= subfolder;
    temp /= std::to_string(boost::unit_test::framework::current_test_case().p_id);
    boost::filesystem::create_directories(temp);
    return temp;
}

} // namespace ashdb::test