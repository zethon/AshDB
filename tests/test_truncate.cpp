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
    auto tempFolder = ashdb::test::tempFolder("simple_truncate");

    ashdb::Options options;
    options.filesize_max = 1024;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    // create the initial 100 records
    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p = project::Person::CreatePerson(i);
        batch.push_back(p);
    }
    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 100);

    // delete half, with a max filesize of 1024, this means the record is
    // in the middle of a data file
    db->truncate(50);
    BOOST_TEST(db->size() == 50);
    BOOST_CHECK_THROW(auto temp = db->read(75), std::runtime_error);

    // write a bunch of new records
    batch.clear();
    for (auto i = 0u; i < 50; ++i)
    {
        project::Person p = project::Person::CreatePerson(i*2);
        batch.push_back(p);
    }
    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 100);

    // make sure the original 50 records are still valid
    for (auto i = 0u; i < 50; ++i)
    {
        project::Person testp = project::Person::CreatePerson(i);
        project::Person db_p = db->read(i);
        BOOST_TEST((testp == db_p));
    }

    // and validate the new 50 records
    for (auto i = 50u; i < 100; ++i)
    {
        project::Person testp = project::Person::CreatePerson((i-50)*2);
        project::Person db_p = db->read(i);
        BOOST_TEST((testp == db_p));
    }
}

BOOST_AUTO_TEST_CASE(boundary)
{
    auto tempFolder = ashdb::test::tempFolder("boundary");

    ashdb::Options options;
    options.filesize_max = 1024;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    // create the initial 100 records
    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p = project::Person::CreatePerson(i);
        batch.push_back(p);
    }
    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 100);

    // with 1024 max filesize, this will means that record 45 is the
    // first in the third datafile, which means we should only two
    // after truncating the db
    db->truncate(45);
    BOOST_TEST(db->size() == 45);
    BOOST_CHECK_THROW(auto temp = db->read(75), std::runtime_error);

    // write a bunch of new records
    batch.clear();
    for (auto i = 0u; i < 10; ++i)
    {
        project::Person p = project::Person::CreatePerson(i*2);
        batch.push_back(p);
    }
    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 55);

    // make sure the original 45 records are still valid
    for (auto i = 0u; i < 45; ++i)
    {
        project::Person testp = project::Person::CreatePerson(i);
        project::Person db_p = db->read(i);
        BOOST_TEST((testp == db_p));
    }

    // and validate the new 10 records
    for (auto i = 45; i < 55; ++i)
    {
        project::Person testp = project::Person::CreatePerson((i-45)*2);
        project::Person db_p = db->read(i);
        BOOST_TEST((testp == db_p));
    }
}

BOOST_AUTO_TEST_SUITE_END()