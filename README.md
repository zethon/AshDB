# AshDB

![Windows Workflow](https://github.com/zethon/AshDB/actions/workflows/windows.yml/badge.svg)
![macos Workflow](https://github.com/zethon/AshDB/actions/workflows/macos.yml/badge.svg)
![Ubuntu Workflow](https://github.com/zethon/AshDB/actions/workflows/ubuntu.yml/badge.svg)
[![codecov](https://codecov.io/gh/zethon/AshDB/branch/master/graph/badge.svg?token=RwtLsgXsEa)](https://codecov.io/gh/zethon/AshDB)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)


AshDB is a simple index based storage library for native and custom types that provides a customizable segmented file layout. 

The database does not support updates. This is designed to be a write-once-read-many database. The database is stored in segments, the max size of each segment can be configured. Likewise, the database can have a limit as a whole which when exceeded, the oldest segments of the database will be deleted.

# Example

Write the digits 0-99 to a new database, then read those numbers back out and print them.

```cpp
#include <iostream>
#include <ashdb/ashdb.h>

int main()
{
    ashdb::Options options;
    options.error_if_exists = true; 

    ashdb::AshDB<std::uint32_t> db{"example1", options};
    assert(db.open() == true);
    for (auto idx = 0u; idx < 100; ++idx)
    {
        db.write(idx);
    }

    for (auto idx = 0u; idx < 100; ++idx)
    {
        std::cout << idx << " : " << db.read(idx) << '\n';
    }
    db.close();

    return 0;
}
```

## Build

After checking out the code, navigate into the folder and create a new folder called `build`. 

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=<Debug|Release|etc...> -DBUILD_ASH_TESTS=On
```

The CMake build options include:

* `BUILD_BENCHMARKS`
* `BUILD_EXAMPLES`
* `BUILD_UNIT_TESTS`
* `CODE_COVERAGE`

## Options

The database options can be configured through the `ashdb::Options` struct located in `include/ashdb/options.h`. 

* `create_if_missing`: A boolean the defines if the database should be created if it doesn't exist. Existence is defined by whether or not the folder itself exists and not by any particular file or files. The default is true.

* `error_if_exists`: A boolean that controls if an error should be generated if the database folder exists. The default is false.

* `filesize_max`: An unsigned integer that defines in bytes the upper threshold of each segment. Segment files can exceed this value since individual records are not split across segments. The default is 0 which means the segments have no size limit and everything will be stored in one data file.

* `database_max`: The upper threshold of the database size. Database size is calculated by the sum of all the database files. Index files are **not** included in this total. When the total size is exceeded, the oldest data file is deleted. The default value is 0 which means the database has no size limit.

* `prefix`: The prefix used for data files and index files. For example a value of "data" would give a filename like `data-00001.ash`. The default value is "data".

* `extension`: The extension used for data files, and prefixed for the index file extensions. For example, a value of "bin" would give a data file with a name "data-00001.bin" and an index file with the name of "data-00001.binidx". The default is "ash".
`
## Performance

Though there are still some planned public API calls to be implemented, the primary _vision_ for the API is complete. Pre-release development focused on the initial implementation, with future releases planned to focus on performance. 

```
Run on (16 X 2300 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 256 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 2.11, 2.01, 1.79
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_DBCreateOpen                25794 ns        25792 ns        26568
BM_DBOpenClose                 25181 ns        25181 ns        27629
BM_DBWriteInt                  91354 ns        91269 ns         7487
BM_DBMultipleIntWrites       9255495 ns      9255213 ns           75
BM_DBRandomIntReads          1845182 ns      1846939 ns          378
BM_DBWriteStruct               97525 ns        95837 ns         7297
BM_DBMultipleStructWrites   10170736 ns     10177726 ns           73
BM_DBRandomStructReads       1975045 ns      1977746 ns          354
```

## Repository Contents

* **benchmarks/** : Contains the benchmark tests used to measure performance (not built by default, use `-DBUILD_BENCHMARKS` in CMake step to turn on)
* **examples/** : Self contained examples of usage (not built by default, use `-DBUILD_EXAMPLES` in CMake step to turn on)
* **include/** : Root folder for header files
    * **include/ashdb/ashdb.h**: Main interface to the DB.
    * **include/ashdb/options.h**: Control over the database, including control over individual file size, max database size, and more.
    * **include/ashdb/status.h**: Status returned from public functions that represent various errors.
    * **include/ashdb/primitives.h**: Implementation of reading and writing primitive C++ types.
* **src** : The compilation units that must be built.
* **tests** : All unit-tests (not built by default, use `-DBUILD_UNIT_TESTS` in CMake step to turn on)
