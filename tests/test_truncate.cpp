#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../include/ashdb/ashdb.h"
#include "../include/ashdb/status.h"

#include "Test.h"
#include "Person.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(truncate_suite)

BOOST_AUTO_TEST_CASE(simple_truncate)
{
    auto tempFolder = ashdb::test::tempFolder("simple_truncate").string();

    ashdb::Options options;
    options.filesize_max = 1024;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p = project::Person::CreatePerson(i);
        batch.push_back(p);
    }
    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 100);

    db->truncate(50);
    BOOST_TEST(db->size() == 50);
    BOOST_CHECK_THROW(auto temp = db->read(75), std::runtime_error);

    batch.clear();
    for (auto i = 0u; i < 50; ++i)
    {
        project::Person p = project::Person::CreatePerson(i*2);
        batch.push_back(p);
    }
    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 100);

    for (auto i = 0u; i < 50; ++i)
    {
        project::Person testp = project::Person::CreatePerson(i);
        project::Person db_p = db->read(i);
        BOOST_TEST((testp == db_p));
    }

    for (auto i = 50u; i < 100; ++i)
    {
        project::Person testp = project::Person::CreatePerson((i-50)*2);
        project::Person db_p = db->read(i);
        BOOST_TEST((testp == db_p));
    }
}

BOOST_AUTO_TEST_SUITE_END()