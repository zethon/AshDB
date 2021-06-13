# AshDB

![Windows Workflow](https://github.com/zethon/AshDB/actions/workflows/windows.yml/badge.svg)
![macos Workflow](https://github.com/zethon/AshDB/actions/workflows/macos.yml/badge.svg)
![Ubuntu Workflow](https://github.com/zethon/AshDB/actions/workflows/ubuntu.yml/badge.svg)
[![codecov](https://codecov.io/gh/zethon/AshDB/branch/master/graph/badge.svg?token=RwtLsgXsEa)](https://codecov.io/gh/zethon/AshDB)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)


AshDB is a simple index based storage library for native and custom types that provides a customizable segmented file layout. 

This project spawned from my desire to write a cryptocoin from scratch which can be found [here](https://github.com/zethon/AshCoin). Since the database is designed to be used with the blockchain of that project there was no reason to support updates or deletes, which made implementation considerably simpler.

This is designed as a write-once-read-many database. The database is stored in segments, the max size of each segment can be configured. Likewise, the database can have a limit as a whole which when exceeded, the oldest segments of the database will be deleted.

# Documentation

[AshDB documentation](doc/README.md) is bundled with the source code.

# Example

Write the digits 0-99 to a new database, then read those numbers back out and print them.

```cpp
#include <iostream>
#include <ashdb/ashdb.h>

int main()
{
    ashdb::Options options;
    options.error_if_exists = true; 

    // creates a folder named "example1"
    ashdb::AshDB<std::uint32_t> db{"example1", options};
    
    if (db.open() != ashdb::OpenStatus::OK)
    {
        std::cerr << "The database could not be opened or already exists!\n";
        return 1;
    }
    
    for (auto idx = 100u; idx > 0; --idx)
    {
        db.write(idx - 1);
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
cmake .. -DCMAKE_BUILD_TYPE=<Debug|Release|etc...> -BUILD_UNIT_TESTS=On
```

The CMake build options include:

* `BUILD_BENCHMARKS`
* `BUILD_EXAMPLES`
* `BUILD_UNIT_TESTS`
* `CODE_COVERAGE`
`
## Performance

Though there are still some planned public API calls to be implemented, the primary _vision_ for the API is complete. Pre-release development focused on the initial implementation, with future releases planned to focus on performance. 

For more information about performance see the [benchmark README.md](benchmarks/README.md).

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
