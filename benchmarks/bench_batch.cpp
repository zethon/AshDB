#include <random>

#include <benchmark/benchmark.h>

#include <ashdb/ashdb.h>

#include "../tests/Person.h"

boost::filesystem::path tempFolder(const std::string& subfolder)
{
    auto temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("ashdb%%%%%%");
    temp /= subfolder;
    boost::filesystem::create_directories(temp);
    return temp;
}

// comparison with "DBMultipleStructWrites"
static void BatchWriteMultipleFiles(benchmark::State& state)
{
    auto tempfolder = (tempFolder("DBStructMultipleWrites")).string();

    ashdb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    ashdb::AshDB<project::Person> db{ tempfolder, options };
    db.open();

    while (state.KeepRunning())
    {
        project::PersonDB::Batch batch;
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
            batch.push_back(p);
            state.ResumeTiming();
        }

        db.write(batch);
    }
}
BENCHMARK(BatchWriteMultipleFiles);

BENCHMARK_MAIN();