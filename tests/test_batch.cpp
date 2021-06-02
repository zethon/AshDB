#include <random>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "Test.h"

#include "../include/ashdb/ashdb.h"
#include "../include/ashdb/primitives.h"

namespace data = boost::unit_test::data;

#include "Person.h"

using StringDB = ashdb::AshDB<std::string>;

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
    BOOST_TEST(db->startIndex().has_value());
    BOOST_TEST(*(db->startIndex()) == 0);
    BOOST_TEST(db->lastIndex().has_value());
    BOOST_TEST(*(db->lastIndex()) == 99);
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

BOOST_AUTO_TEST_CASE(batch_read1)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_read1")).string();

    ashdb::Options options;
    options.filesize_max = 375; // should fit 4 100 character strings

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    for (auto x = 0; x < 12; ++x)
    {
        BOOST_TEST(db->write(e100Chars) == ashdb::WriteStatus::OK);
    }

    BOOST_TEST(db->size() == 12);

    const auto& segmentIndices = db->segmentIndices();
    BOOST_TEST(segmentIndices.size() == 3);
    BOOST_TEST(segmentIndices[0].size() == 4);
    BOOST_TEST(segmentIndices[1].size() == 4);
    BOOST_TEST(segmentIndices[2].size() == 4);

    StringDB::Batch batch1 = db->read(6, 4); // across segments
    BOOST_TEST(batch1.size() == 4);
    for (const auto& str : batch1)
    {
        BOOST_TEST(str == e100Chars);
    }

    StringDB::Batch batch2 = db->read(5, 2); // within segment
    BOOST_TEST(batch2.size() == 2);
    for (const auto& str : batch2)
    {
        BOOST_TEST(str == e100Chars);
    }

    StringDB::Batch batch3 = db->read(9, 3); // to the end of a segment
    BOOST_TEST(batch3.size() == 3);
    for (const auto& str : batch3)
    {
        BOOST_TEST(str == e100Chars);
    }

    StringDB::Batch batch4 = db->read(8, 3); // from start of segment
    BOOST_TEST(batch4.size() == 3);
    for (const auto& str : batch4)
    {
        BOOST_TEST(str == e100Chars);
    }

    StringDB::Batch batch5 = db->read(8, 4); // entire segment
    BOOST_TEST(batch5.size() == 4);
    for (const auto& str : batch5)
    {
        BOOST_TEST(str == e100Chars);
    }

    StringDB::Batch batch6 = db->read(1, 9); // span multiple segments
    BOOST_TEST(batch6.size() == 9);
    for (const auto& str : batch6)
    {
        BOOST_TEST(str == e100Chars);
    }
}

BOOST_AUTO_TEST_CASE(batch_read_write_multiple_files)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_read_write_multiple_files")).string();

    ashdb::Options options;
    options.filesize_max = 100;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    for (auto i = 0u; i < 200; ++i)
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
    BOOST_TEST(db->size() == 200);
    BOOST_TEST(db->startIndex().has_value());
    BOOST_TEST(*db->startIndex() == 0);
    BOOST_TEST(db->lastIndex().has_value());
    BOOST_TEST(*db->lastIndex() == 199);

    db->close();
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->size() == 200);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 99);
    std::uint32_t i = static_cast<std::uint32_t>(distrib(gen));

    // test 100 random records
    for (auto idx = 0; idx < 100; ++idx)
    {
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

BOOST_AUTO_TEST_CASE(batch_trim)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_errors")).string();

    ashdb::Options options;
    options.filesize_max = 100;
    options.database_max = 300;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    StringDB::Batch batch;
    batch.push_back(e100Chars);
    batch.push_back(e100Chars);
    batch.push_back(e100Chars);

    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 2);
    BOOST_TEST(db->startSegmentNumber() == 1);
    BOOST_TEST(db->activeSegmentNumber() == 3);

    BOOST_CHECK_THROW(batch = db->read(0,10), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(batch_errors)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_errors")).string();

    ashdb::Options options;
    options.filesize_max = 100;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_CHECK_THROW(auto status = db->read(10,100), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(empty_batch_write)
{
    auto tempFolder = (ashdb::test::tempFolder("empty_batch_write")).string();

    auto db = std::make_unique<StringDB>(tempFolder, ashdb::Options{});
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->write(StringDB::Batch{}) == ashdb::WriteStatus::OK);
}

BOOST_AUTO_TEST_SUITE_END()
