#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../include/ashdb/ashdb.h"
#include "../include/ashdb/status.h"

#include "Test.h"
#include "Person.h"

namespace data = boost::unit_test::data;

using StringDB = ashdb::AshDB<std::string>;


BOOST_AUTO_TEST_SUITE(basic)

#ifndef _WINDOWS
BOOST_AUTO_TEST_CASE(file_pattern)
{
    auto filename = ashdb::BuildFilename("/usr/data", "file", "dat", 57);
    BOOST_TEST(filename == "/usr/data/file-00057.dat");

    filename = ashdb::BuildFilename("/usr/data", "file", "dat", 65535);
    BOOST_TEST(filename == "/usr/data/file-65535.dat");

    filename = ashdb::BuildFilename("/usr/data", "file", "dat", 0);
    BOOST_TEST(filename == "/usr/data/file-00000.dat");

    BOOST_CHECK_THROW(ashdb::BuildFilename("/usr/data", "file", "dat", 165535), std::runtime_error);
}
#endif

BOOST_AUTO_TEST_CASE(status_to_string_tests)
{
    ashdb::OpenStatus status;

    BOOST_TEST(ashdb::ToString(ashdb::OpenStatus::OK) == "OK");
    BOOST_TEST(ashdb::ToString(ashdb::OpenStatus::EXISTS) == "EXISTS");
    BOOST_TEST(ashdb::ToString(ashdb::OpenStatus::NOT_FOUND) == "NOT_FOUND");
    BOOST_TEST(ashdb::ToString(ashdb::OpenStatus::INVALID_PREFIX) == "INVALID_PREFIX");
    BOOST_TEST(ashdb::ToString(ashdb::OpenStatus::INVALID_EXTENSION) == "INVALID_EXTENSION");
    BOOST_TEST(ashdb::ToString(ashdb::OpenStatus::ALREADY_OPEN) == "ALREADY_OPEN");

    BOOST_TEST(ashdb::ToString(ashdb::WriteStatus::OK) == "OK");
    BOOST_TEST(ashdb::ToString(ashdb::WriteStatus::DATABASE_NOT_OPEN) == "DATABASE_NOT_OPEN");
}

BOOST_AUTO_TEST_CASE(db_open)
{
    auto tempFolder = ashdb::test::tempFolder("db_open");

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    ashdb::AshDB<std::string> db{tempFolder, options};
    BOOST_TEST(db.open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db.startSegmentNumber() == 0);
    BOOST_TEST(db.activeSegmentNumber() == 0);

    options.error_if_exists = true;
    ashdb::AshDB<std::string> db2{tempFolder, options};
    BOOST_TEST(db2.open() == ashdb::OpenStatus::EXISTS);
    BOOST_TEST(db2.startSegmentNumber() == 0);
    BOOST_TEST(db2.activeSegmentNumber() == 0);
}

BOOST_AUTO_TEST_CASE(db_open_error)
{
    // boost's temp_directory_path() will create the folder it's returning
    // so in this case we'll use a subfolder that we know doesn't exists
    auto tempFolder =
            std::filesystem::path{ashdb::test::tempFolder("db_open_error")} / "new_db";

    ashdb::Options options;
    options.create_if_missing = false;

    auto db = std::make_unique<StringDB>(tempFolder.string(), options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::NOT_FOUND);

    options.create_if_missing = true;
    db = std::make_unique<StringDB>(tempFolder.string(), options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    options.error_if_exists = true;
    db = std::make_unique<StringDB>(tempFolder.string(), options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::EXISTS);
}

BOOST_AUTO_TEST_CASE(db_open_failed)
{
    auto tempFolder = (ashdb::test::tempFolder("db_open_failed"));

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    options.extension = "idx";
    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::INVALID_EXTENSION);

    options.extension.clear();
    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::INVALID_EXTENSION);

    options.extension = "$.!";
    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::INVALID_EXTENSION);

    options.extension = "dat";
    options.prefix.clear();
    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::INVALID_PREFIX);

    options.prefix = "$1.";
    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::INVALID_PREFIX);

    db = std::make_unique<StringDB>(tempFolder, ashdb::Options{});
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->open() == ashdb::OpenStatus::ALREADY_OPEN);
}

BOOST_AUTO_TEST_CASE(db_write1)
{
    auto tempFolder = (ashdb::test::tempFolder("db_write1"));

    ashdb::Options options;
    options.filesize_max = 10;

    std::unique_ptr<StringDB> db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::DATABASE_NOT_OPEN);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->startSegmentNumber() == 0);
    BOOST_TEST(db->activeSegmentNumber() == 1);
    BOOST_TEST(db->segmentIndices().size() == 1);
    BOOST_TEST(db->segmentIndices().at(0).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 0));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->activeSegmentNumber() == 2);
    BOOST_TEST(db->segmentIndices().size() == 2);
    BOOST_TEST(db->segmentIndices().at(1).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 1));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->activeSegmentNumber() == 3);
    BOOST_TEST(db->segmentIndices().size() == 3);
    BOOST_TEST(db->segmentIndices().at(2).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 2));

    db->close();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    BOOST_TEST(db->activeSegmentNumber() == 3);
    BOOST_TEST(db->segmentIndices().size() == 3);
    BOOST_TEST(db->segmentIndices().at(0).size() == 1);
    BOOST_TEST(db->segmentIndices().at(1).size() == 1);
    BOOST_TEST(db->segmentIndices().at(2).size() == 1);
    BOOST_TEST(db->startSegmentNumber() == 0);
    BOOST_TEST(db->startIndex().has_value());
    BOOST_TEST(*(db->startIndex()) == 0);
    BOOST_TEST(db->lastIndex().has_value());
    BOOST_TEST(*(db->lastIndex()) == 2);
}

BOOST_AUTO_TEST_CASE(db_write2)
{
    static const char* piStr = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";
    auto tempFolder = (ashdb::test::tempFolder("db_write2"));

    ashdb::Options options;
    options.filesize_max = 1536;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // writes in 0
    BOOST_TEST(db->startSegmentNumber() == 0);
    BOOST_TEST(db->activeSegmentNumber() == 0);
    BOOST_TEST(db->segmentIndices().size() == 1);
    BOOST_TEST(db->segmentIndices().at(0).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 0));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 0
    BOOST_TEST(db->activeSegmentNumber() == 1);
    BOOST_TEST(db->segmentIndices().size() == 1);
    BOOST_TEST(db->segmentIndices().at(0).size() == 2);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 1));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 1
    BOOST_TEST(db->activeSegmentNumber() == 1);
    BOOST_TEST(db->segmentIndices().size() == 2);
    BOOST_TEST(db->segmentIndices().at(1).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 2));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 1
    BOOST_TEST(db->activeSegmentNumber() == 2);
    BOOST_TEST(db->segmentIndices().size() == 2);
    BOOST_TEST(db->segmentIndices().at(1).size() == 2);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 3));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 2
    BOOST_TEST(db->activeSegmentNumber() == 2);
    BOOST_TEST(db->segmentIndices().size() == 3);
    BOOST_TEST(db->segmentIndices().at(2).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));

    db->close();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->activeSegmentNumber() == 2);
    BOOST_TEST(db->segmentIndices().size() == 3);
    BOOST_TEST(db->segmentIndices().at(0).size() == 2);
    BOOST_TEST(db->segmentIndices().at(1).size() == 2);
    BOOST_TEST(db->segmentIndices().at(2).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));
}

BOOST_AUTO_TEST_CASE(db_write3)
{
    static const char *piStr = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";
    auto tempFolder = (ashdb::test::tempFolder("db_write3"));

    ashdb::Options options;
    options.filesize_max = 256;
    options.database_max = 3500;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->activeSegmentNumber() == 0);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 0
    BOOST_TEST(db->activeSegmentNumber() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 0));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 1
    BOOST_TEST(db->activeSegmentNumber() == 2);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 1));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 2
    BOOST_TEST(db->activeSegmentNumber() == 3);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 2));

    // db is too big, so delete the first file
    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 3
    BOOST_TEST(db->startSegmentNumber() == 1);
    BOOST_TEST(db->activeSegmentNumber() == 4);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 1));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 3));

    db.reset();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->startSegmentNumber() == 1);
    BOOST_TEST(db->activeSegmentNumber() == 4);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 1));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 3));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 4
    BOOST_TEST(db->startSegmentNumber() == 2);
    BOOST_TEST(db->activeSegmentNumber() == 5);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 2));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));

    db.reset();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->startSegmentNumber() == 2);
    BOOST_TEST(db->activeSegmentNumber() == 5);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 2));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 4
    BOOST_TEST(db->activeSegmentNumber() == 6);
    BOOST_TEST(db->startSegmentNumber() == 3);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 3));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 5));
}

BOOST_AUTO_TEST_CASE(db_read1)
{
    auto tempFolder = (ashdb::test::tempFolder("db_read1"));

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    options.filesize_max = 1024 * 5;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    for (auto i = 0u; i < 10; ++i)
    {
        std::stringstream ss;
        ss << "string" << i;
        BOOST_TEST(db->write(ss.str()) == ashdb::WriteStatus::OK);
        BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
        BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == i));
    }

    BOOST_TEST(db->size() == 10);

    for (auto i = 0u; i < 10; ++i)
    {
        auto temp = db->read(i);
        BOOST_TEST(temp == "string" + std::to_string(i));
    }
}

BOOST_AUTO_TEST_CASE(db_read2)
{
    static auto dataStr = "[ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ-";
    auto tempFolder = (ashdb::test::tempFolder("db_read2"));

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    options.filesize_max = 32;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    for (auto i = 0u; i < 100; ++i)
    {
        std::stringstream ss;
        ss << dataStr << i;
        BOOST_TEST(db->write(ss.str()) == ashdb::WriteStatus::OK);
        BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
        BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == i));
    }

    BOOST_TEST(db->size() == 100);

    for (auto i = 0u; i < 100; ++i)
    {
        std::stringstream ss;
        ss << dataStr << i;

        auto temp = db->read(i);
        BOOST_TEST(temp == ss.str());
    }
}

BOOST_AUTO_TEST_CASE(db_read3)
{
    auto tempFolder = (ashdb::test::tempFolder("db_read3"));

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    options.filesize_max = 1024 * 2;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    for (auto i = 0u; i < 100; ++i)
    {
        BOOST_CHECK_THROW(auto temp = db->read(i), std::runtime_error);
        BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
        BOOST_TEST(db->size() == i+1);
        auto temp = db->read(i);
        BOOST_TEST(temp == piStr);
    }

    BOOST_TEST(db->size() == 100);
}

BOOST_AUTO_TEST_CASE(db_read4)
{
    auto tempFolder = (ashdb::test::tempFolder("db_read4"));

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    options.filesize_max = 1024 * 1024 * 1024;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    for (auto i = 0u; i < 100; ++i)
    {
        BOOST_CHECK_THROW(auto temp = db->read(i), std::runtime_error);
        BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
        BOOST_TEST(db->size() == i+1);
        auto temp = db->read(i);
        BOOST_TEST(temp == piStr);
    }

    BOOST_TEST(db->size() == 100);
    db->close();
    BOOST_TEST(db->size() == 0);

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->size() == 0);
}

BOOST_AUTO_TEST_CASE(delete_files)
{
    auto tempFolder = (ashdb::test::tempFolder("batch_errors"));

    ashdb::Options options;
    options.filesize_max = 100;
    options.database_max = 300;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    StringDB::Batch batch;
    for (auto x = 0u; x < 3; ++x)
    {
        char c = std::to_string(x).at(0);
        std::string str(110, c);
        batch.push_back(str);
    }

    BOOST_TEST(db->write(batch) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->write("A") == ashdb::WriteStatus::OK);

    // `size()` represents the number of available records in the database
    BOOST_TEST(db->size() == 3);

    // remove the last file
    const auto datafile = db->activeDataFile();
    BOOST_TEST(boost::ends_with(datafile, "3.ash"));
    std::filesystem::remove(datafile);

    // and now try to get a record in the file we just deleted
    BOOST_CHECK_THROW(auto t = db->read(3), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(move_ctor)
{
    auto tempFolder = ashdb::test::tempFolder("move_ctor");

    ashdb::Options options;
    options.filesize_max = 1024;

    project::PersonDB db{tempFolder, options};
    BOOST_TEST(db.open() == ashdb::OpenStatus::OK);

    project::PersonDB::Batch batch;

    // create the initial 100 records
    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p = project::Person::CreatePerson(i);
        batch.push_back(p);
    }
    BOOST_TEST(db.write(batch) == ashdb::WriteStatus::OK);

    const auto startSegment = db.startSegmentNumber();
    const auto activeSegment = db.activeSegmentNumber();

    project::PersonDB db2 = std::move(db);
    BOOST_TEST(db2.size() == 100);
    BOOST_TEST(db2.startSegmentNumber() == startSegment);
    BOOST_TEST(db2.activeSegmentNumber() == activeSegment);
}

BOOST_AUTO_TEST_CASE(db_size)
{
    auto tempFolder = ashdb::test::tempFolder("db_size");

    ashdb::Options options;
    options.filesize_max = 64;

    StringDB db{tempFolder, options};
    BOOST_TEST(db.open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db.databaseSize() == 0);

    BOOST_TEST(db.write(e100Chars) == ashdb::WriteStatus::OK);
    BOOST_TEST(db.databaseSize() > 100);

    BOOST_TEST(db.write(e100Chars) == ashdb::WriteStatus::OK);
    BOOST_TEST(db.databaseSize() > 200);

    BOOST_TEST(db.write("0123456789") == ashdb::WriteStatus::OK);
    BOOST_TEST(db.databaseSize() > 210);
}

BOOST_AUTO_TEST_SUITE_END() 