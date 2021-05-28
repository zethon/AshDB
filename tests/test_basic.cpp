#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "Test.h"

#include "../include/ashdb/ashdb.h"

namespace data = boost::unit_test::data;

using StringDB = ashdb::AshDB<std::string>;

static const char* piStr = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";

BOOST_AUTO_TEST_SUITE(basic)

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

BOOST_AUTO_TEST_CASE(db_open)
{
    auto tempFolder = ashdb::test::tempFolder("db_open");

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    ashdb::AshDB<std::string> db{tempFolder.string(), options};
    BOOST_TEST(db.open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db.startRecordNumber() == 0);
    BOOST_TEST(db.activeRecordNumber() == 0);

    options.error_if_exists = true;
    ashdb::AshDB<std::string> db2{tempFolder.string(), options};
    BOOST_TEST(db2.open() == ashdb::OpenStatus::EXISTS);
    BOOST_TEST(db2.startRecordNumber() == 0);
    BOOST_TEST(db2.activeRecordNumber() == 0);
}

BOOST_AUTO_TEST_CASE(db_open_failed)
{
    auto tempFolder = (ashdb::test::tempFolder("db_open_failed")).string();

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
    auto tempFolder = (ashdb::test::tempFolder("db_write1")).string();

    ashdb::Options options;
    options.filesize_max = 10;

    std::unique_ptr<StringDB> db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->startRecordNumber() == 0);
    BOOST_TEST(db->activeRecordNumber() == 1);
    BOOST_TEST(db->indexRecord().size() == 1);
    BOOST_TEST(db->indexRecord().at(0).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 0));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->activeRecordNumber() == 2);
    BOOST_TEST(db->indexRecord().size() == 2);
    BOOST_TEST(db->indexRecord().at(1).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 1));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK);
    BOOST_TEST(db->activeRecordNumber() == 3);
    BOOST_TEST(db->indexRecord().size() == 3);
    BOOST_TEST(db->indexRecord().at(2).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 2));

    db->close();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);

    BOOST_TEST(db->activeRecordNumber() == 3);
    BOOST_TEST(db->indexRecord().size() == 3);
    BOOST_TEST(db->indexRecord().at(0).size() == 1);
    BOOST_TEST(db->indexRecord().at(1).size() == 1);
    BOOST_TEST(db->indexRecord().at(2).size() == 1);
    BOOST_TEST(db->startRecordNumber() == 0);
    BOOST_TEST(db->startIndex().has_value());
    BOOST_TEST(*(db->startIndex()) == 0);
    BOOST_TEST(db->lastIndex().has_value());
    BOOST_TEST(*(db->lastIndex()) == 2);
}

BOOST_AUTO_TEST_CASE(db_write2)
{
    static const char* piStr = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";
    auto tempFolder = (ashdb::test::tempFolder("db_write2")).string();

    ashdb::Options options;
    options.filesize_max = 1536;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // writes in 0
    BOOST_TEST(db->startRecordNumber() == 0);
    BOOST_TEST(db->activeRecordNumber() == 0);
    BOOST_TEST(db->indexRecord().size() == 1);
    BOOST_TEST(db->indexRecord().at(0).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 0));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 0
    BOOST_TEST(db->activeRecordNumber() == 1);
    BOOST_TEST(db->indexRecord().size() == 1);
    BOOST_TEST(db->indexRecord().at(0).size() == 2);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 1));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 1
    BOOST_TEST(db->activeRecordNumber() == 1);
    BOOST_TEST(db->indexRecord().size() == 2);
    BOOST_TEST(db->indexRecord().at(1).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 2));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 1
    BOOST_TEST(db->activeRecordNumber() == 2);
    BOOST_TEST(db->indexRecord().size() == 2);
    BOOST_TEST(db->indexRecord().at(1).size() == 2);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 3));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 2
    BOOST_TEST(db->activeRecordNumber() == 2);
    BOOST_TEST(db->indexRecord().size() == 3);
    BOOST_TEST(db->indexRecord().at(2).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));

    db->close();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->activeRecordNumber() == 2);
    BOOST_TEST(db->indexRecord().size() == 3);
    BOOST_TEST(db->indexRecord().at(0).size() == 2);
    BOOST_TEST(db->indexRecord().at(1).size() == 2);
    BOOST_TEST(db->indexRecord().at(2).size() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));
}

BOOST_AUTO_TEST_CASE(db_write3)
{
    static const char *piStr = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";
    auto tempFolder = (ashdb::test::tempFolder("db_write3")).string();

    ashdb::Options options;
    options.filesize_max = 256;
    options.database_max = 3500;

    auto db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->activeRecordNumber() == 0);
    BOOST_TEST(!db->startIndex().has_value());
    BOOST_TEST(!db->lastIndex().has_value());

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 0
    BOOST_TEST(db->activeRecordNumber() == 1);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 0));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 1
    BOOST_TEST(db->activeRecordNumber() == 2);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 1));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 2
    BOOST_TEST(db->activeRecordNumber() == 3);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 0));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 2));

    // db is too big, so delete the first file
    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 3
    BOOST_TEST(db->startRecordNumber() == 1);
    BOOST_TEST(db->activeRecordNumber() == 4);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 1));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 3));

    db.reset();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->startRecordNumber() == 1);
    BOOST_TEST(db->activeRecordNumber() == 4);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 1));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 3));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 4
    BOOST_TEST(db->startRecordNumber() == 2);
    BOOST_TEST(db->activeRecordNumber() == 5);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 2));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));

    db.reset();

    db = std::make_unique<StringDB>(tempFolder, options);
    BOOST_TEST(db->open() == ashdb::OpenStatus::OK);
    BOOST_TEST(db->startRecordNumber() == 2);
    BOOST_TEST(db->activeRecordNumber() == 5);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 2));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 4));

    BOOST_TEST(db->write(piStr) == ashdb::WriteStatus::OK); // 4
    BOOST_TEST(db->activeRecordNumber() == 6);
    BOOST_TEST(db->startRecordNumber() == 3);
    BOOST_TEST((db->startIndex().has_value() && *(db->startIndex()) == 3));
    BOOST_TEST((db->lastIndex().has_value() && *(db->lastIndex()) == 5));
}

BOOST_AUTO_TEST_CASE(db_read1)
{
    auto tempFolder = (ashdb::test::tempFolder("db_read1")).string();

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
    auto tempFolder = (ashdb::test::tempFolder("db_read2")).string();

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
    auto tempFolder = (ashdb::test::tempFolder("db_read3")).string();

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
    auto tempFolder = (ashdb::test::tempFolder("db_read4")).string();

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

BOOST_AUTO_TEST_SUITE_END() 