#include <random>

#include <benchmark/benchmark.h>

#include <ashdb/ashdb.h>

using StringDB = ashdb::AshDB<std::string>;

boost::filesystem::path tempFolder(const std::string& subfolder)
{
    auto temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("ashdb%%%%%%");
    temp /= subfolder;
    boost::filesystem::create_directories(temp);
    return temp;
}

static void DBCreateOpen(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBCreateOpen")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    while (state.KeepRunning())
    {
        StringDB db{ tempfolder, options };
        db.open();
    }
}
BENCHMARK(DBCreateOpen);

static void DBOpenClose(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBOpenClose")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    StringDB db{ tempfolder, options };

    while (state.KeepRunning())
    {
        db.open();
        db.close();
    }
}
BENCHMARK(DBOpenClose);

static void DBWriteInt(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBWriteInt")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    ashdb::AshDB<int> db{ tempfolder, options };
    db.open();

    while (state.KeepRunning())
    {
        db.write(3);
    }
}
BENCHMARK(DBWriteInt);

static void DBMultipleIntWrites(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBMultipleIntWrites")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    ashdb::AshDB<int> db{ tempfolder, options };
    db.open();

    while (state.KeepRunning())
    {
        for (auto i = 0u; i < 100u; ++i)
        {
            db.write(3);
        }
    }
}
BENCHMARK(DBMultipleIntWrites);

static void DBRandomIntReads(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBRandomIntReads")).string();

    std::random_device rd;
    std::mt19937 gen(rd());

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    ashdb::AshDB<int> db{ tempfolder, options };
    db.open();

    for (auto i = 0u; i < 100u; ++i)
    {
        db.write(3);
    }

    while (state.KeepRunning())
    {
        for (auto i = 0u; i < 100u; ++i)
        {
            state.PauseTiming();
            std::uniform_int_distribution<> distrib(0, 99);
            std::uint32_t xi = static_cast<std::uint32_t>(distrib(gen));
            state.ResumeTiming();
            db.read(xi);
        }
    }
}
BENCHMARK(DBRandomIntReads);

#include "../tests/Person.h"

static void DBWriteStruct(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBWriteStruct")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    ashdb::AshDB<project::Person> db{ tempfolder, options };
    db.open();

    project::Person p;
    p.name.first = "Firstname0";
    p.name.middle = "Middle0";
    p.name.last = "Lastname0";
    p.age = 40;
    p.salary = 12345.67;
    p.married = true;

    while (state.KeepRunning())
    {
        db.write(p);
    }
}
BENCHMARK(DBWriteStruct);

static void DBMultipleStructWrites(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBStructMultipleWrites")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    ashdb::AshDB<project::Person> db{ tempfolder, options };
    db.open();

    while (state.KeepRunning())
    {
        for (auto i = 0u; i < 100; ++i)
        {
            state.PauseTiming();
            project::Person p;
            p.name.first = "Firstname" + std::to_string(i);
            p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
            p.name.last = "Lastname" + std::to_string(i);
            p.age = (i % 80);
            p.salary = (i % 5) * 12345.67;
            p.married = (i % 2) == 0;
            state.ResumeTiming();

            db.write(p);
        }
    }
}
BENCHMARK(DBMultipleStructWrites);

static void DBRandomStructReads(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBRandomIntReads")).string();

    std::random_device rd;
    std::mt19937 gen(rd());

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;
    ashdb::AshDB<project::Person> db{ tempfolder, options };
    db.open();

    for (auto i = 0u; i < 100; ++i)
    {
        project::Person p;
        p.name.first = "Firstname" + std::to_string(i);
        p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
        p.name.last = "Lastname" + std::to_string(i);
        p.age = (i % 80);
        p.salary = (i % 5) * 12345.67;
        p.married = (i % 2) == 0;

        db.write(p);
    }

    while (state.KeepRunning())
    {
        for (auto i = 0u; i < 100u; ++i)
        {
            state.PauseTiming();
            std::uniform_int_distribution<> distrib(0, 99);
            std::uint32_t xi = static_cast<std::uint32_t>(distrib(gen));
            state.ResumeTiming();

            db.read(xi);
        }
    }
}
BENCHMARK(DBRandomStructReads);

BENCHMARK_MAIN();