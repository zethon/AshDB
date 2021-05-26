#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "Test.h"

#include "../include/ashdb/ashdb.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(basic)

BOOST_AUTO_TEST_CASE(file_pattern)
{
    auto filename = ashdb::GenerateFilename("/usr/data", "file-%d.dat", 57);
    BOOST_TEST(filename == "/usr/data/file-00057.dat");

    filename = ashdb::GenerateFilename("/usr/data", "file-%d.dat", 65535);
    BOOST_TEST(filename == "/usr/data/file-65535.dat");

    filename = ashdb::GenerateFilename("/usr/data", "file-%d.dat", 0);
    BOOST_TEST(filename == "/usr/data/file-00000.dat");

    BOOST_CHECK_THROW(ashdb::GenerateFilename("/usr/data", "file-%d.dat", 165535), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END() 