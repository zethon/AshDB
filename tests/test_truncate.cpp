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
    auto db = std::make_unique<project::PersonDB>(tempFolder, ashdb::Options{});
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
}

BOOST_AUTO_TEST_SUITE_END()