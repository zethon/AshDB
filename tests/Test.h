#pragma once
#include <iostream>

#include <boost/test/data/test_case.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

namespace ashdb::test
{

boost::filesystem::path tempFolder()
{
    auto temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("ash%%%%%%");
    temp /= std::to_string(boost::unit_test::framework::current_test_case().p_id);
    boost::filesystem::create_directories(temp);
    return temp;
}

boost::filesystem::path tempFolder(const std::string& subfolder)
{
    auto temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("ash%%%%%%");
    temp /= subfolder;
    temp /= std::to_string(boost::unit_test::framework::current_test_case().p_id);
    boost::filesystem::create_directories(temp);
    return temp;
}

} // namespace ashdb::test