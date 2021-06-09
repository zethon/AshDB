#include <random>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "Test.h"

#include "../include/ashdb/ashdb.h"
#include "../include/ashdb/primitives.h"

#include "Person.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(class_tests)

BOOST_AUTO_TEST_CASE(class_write1)
{
    auto tempFolder = (ashdb::test::tempFolder("class_write1"));

    ashdb::Options options;
    options.filesize_max = 100;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    project::Person p;
    p.name.first = "Firstname";
    p.name.last = "Lastname";
    p.age = 50;
    p.salary = 123456.78;
    p.married = true;

    BOOST_TEST(db->write(p) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->size() == 1);
    db->close();

    db.reset();

    db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->size() == 1);
    auto p1 = db->read(0);
    BOOST_TEST((p == p1));
}

BOOST_AUTO_TEST_CASE(class_write2)
{
    auto tempFolder = (ashdb::test::tempFolder("class_write1"));

    ashdb::Options options;
    options.filesize_max = 100;

    auto db = std::make_unique<project::PersonDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p;
        p.name.first = "Firstname" + std::to_string(i);
        p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
        p.name.last = "Lastname" + std::to_string(i);
        p.age = (i % 80);
        p.salary = (i % 5) * 12345.67;
        p.married = (i % 2) == 0;

        BOOST_TEST(db->write(p) == ashdb::WriteStatus::OK);
        BOOST_TEST(db->size() == i + 1);
    }

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
