#include <random>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "Test.h"

#include "../include/ashdb/ashdb.h"
#include "../include/ashdb/primitives.h"

namespace data = boost::unit_test::data;

#include "Person.h"

BOOST_AUTO_TEST_SUITE(batch)

BOOST_AUTO_TEST_CASE(batch_not_open)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_not_open")).string();
    auto db = std::make_unique<project::PersonDB>(tempFolder, ashdb::Options{});
    BOOST_TEST(db->write(project::PersonDB::Batch{}) == ashdb::WriteStatus::NOT_OPEN);
}

BOOST_AUTO_TEST_CASE(batch_write_single_file)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_write1")).string();
    auto db = std::make_unique<project::PersonDB>(tempFolder, ashdb::Options{});
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p;
        p.name.first = "Firstname" + std::to_string(i);
        p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
        p.name.last = "Lastname" + std::to_string(i);
        p.age = (i % 80);
        p.salary = (i % 5) * 12345.67;
        p.married = (i % 2) == 0;

        batch.push_back(p);
    }

    auto status = db->write(batch);
    BOOST_TEST(status == ashdb::WriteStatus::OK);

    db->close();
}

BOOST_AUTO_TEST_CASE(batch_write_multiple_files)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_write1")).string();

    ashdb::Options options;
    options.filesize_max = 100;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p;
        p.name.first = "Firstname" + std::to_string(i);
        p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
        p.name.last = "Lastname" + std::to_string(i);
        p.age = (i % 80);
        p.salary = (i % 5) * 12345.67;
        p.married = (i % 2) == 0;

        batch.push_back(p);
    }

    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 100);
    BOOST_TEST(db->startIndex().has_value());
    BOOST_TEST(*db->startIndex() == 0);
    BOOST_TEST(db->lastIndex().has_value());
    BOOST_TEST(*db->lastIndex() == 99);

    db->close();
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->size() == 100);

    std::random_device rd;
    std::mt19937 gen(rd());

    // test 100 random records
    for (auto idx = 0; idx < 100; ++idx)
    {
        std::uniform_int_distribution<> distrib(0, 99);
        std::uint32_t i = static_cast<std::uint32_t>(distrib(gen));

        auto temp = db->read(i);

        project::Person p;
        p.name.first = "Firstname" + std::to_string(i);
        p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
        p.name.last = "Lastname" + std::to_string(i);
        p.age = (i % 80);
        p.salary = (i % 5) * 12345.67;
        p.married = (i % 2) == 0;

        BOOST_TEST((temp == p));
    }
}

BOOST_AUTO_TEST_SUITE_END()
