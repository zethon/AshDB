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

static void BatchWriteSingleFile(benchmark::State& state)
{

}
BENCHMARK(BatchWriteSingleFile);

static void BatchWriteMultipleFiles(benchmark::State& state)
{

}
BENCHMARK(BatchWriteMultipleFiles);

BENCHMARK_MAIN();