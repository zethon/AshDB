#include <random>

#include <benchmark/benchmark.h>

#include <ashdb/ashdb.h>

#include "../tests/Person.h"

using StringDB = ashdb::AshDB<std::string>;

std::string tempFolder(const std::string& subfolder)
{
    const auto now = std::chrono::system_clock::now();
    const auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    auto temp = std::filesystem::temp_directory_path() / std::to_string(epoch);
    temp /= subfolder;
    std::filesystem::create_directories(temp);
    return temp.string();
}

static void DBCreateOpen(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBCreateOpen"));

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
    auto tempfolder = (tempFolder("DBOpenClose"));

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
    auto tempfolder = (tempFolder("DBWriteInt"));

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
    auto tempfolder = (tempFolder("DBMultipleIntWrites"));

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
    auto tempfolder = (tempFolder("DBRandomIntReads"));

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

static void DBWriteStruct(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBWriteStruct"));

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
    auto tempfolder = (tempFolder("DBStructMultipleWrites"));

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
    auto tempfolder = (tempFolder("DBRandomIntReads"));

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

static void BatchWriteSingleFile(benchmark::State& state)
{
    std:size_t run = 0;
    while (state.KeepRunning())
    {
        state.PauseTiming();
        const std::string tempName = "BatchWriteSingleFile" + std::to_string(run);
        auto tempfolder = (tempFolder(tempName));
        auto db = std::make_unique<project::PersonDB>(tempfolder, ashdb::Options{});
        db->open();
        project::PersonDB::Batch batch;
        for (auto i = 0u; i < 100; ++i)
        {
            project::Person p = project::Person::CreatePerson(i);
            batch.push_back(p);
        }
        state.ResumeTiming();

        db->write(batch);
    }
}
BENCHMARK(BatchWriteSingleFile);

// comparison with "DBMultipleStructWrites"
static void BatchWriteMultipleFiles(benchmark::State& state)
{
    std:size_t run = 0;
    while (state.KeepRunning())
    {
        state.PauseTiming();
        ashdb::Options options;
        options.filesize_max = 1024;
        const std::string tempName = "BatchWriteMultipleFiles" + std::to_string(run);
        auto tempfolder = (tempFolder(tempName));
        auto db = std::make_unique<project::PersonDB>(tempfolder, options);
        db->open();
        project::PersonDB::Batch batch;
        for (auto i = 0u; i < 100; ++i)
        {
            project::Person p = project::Person::CreatePerson(i);
            batch.push_back(p);
        }
        state.ResumeTiming();

        db->write(batch);
    }
}
BENCHMARK(BatchWriteMultipleFiles);

// compare to "DBRandomStructReads"
static void BatchReadMultipleFiles(benchmark::State& state)
{
    auto tempfolder = (tempFolder("BatchReadMultipleFiles"));

    ashdb::Options options;
    options.filesize_max = 100;

    ashdb::AshDB<project::Person> db{ tempfolder, options };
    db.open();

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
    db.write(batch);

    std::random_device rd;
    std::mt19937 gen(rd());

    while (state.KeepRunning())
    {
        state.PauseTiming();
        std::uniform_int_distribution<> distrib(0, 99);
        std::uint32_t idx = static_cast<std::uint32_t>(distrib(gen));
        state.ResumeTiming();

        // read 100 records at a random idx
        auto b = db.read(idx, 100);

        state.PauseTiming();
        b.clear(); // do something to avoid unused var warning
        state.ResumeTiming();
    }
}
BENCHMARK(BatchReadMultipleFiles);

BENCHMARK_MAIN();