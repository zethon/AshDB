# AshDB

![Windows Workflow](https://github.com/zethon/AshDB/actions/workflows/windows.yml/badge.svg)
![macos Workflow](https://github.com/zethon/AshDB/actions/workflows/macos.yml/badge.svg)
![Ubuntu Workflow](https://github.com/zethon/AshDB/actions/workflows/ubuntu.yml/badge.svg)
[![codecov](https://codecov.io/gh/zethon/AshDB/branch/master/graph/badge.svg?token=RwtLsgXsEa)](https://codecov.io/gh/zethon/AshDB)

AshDB is a simple index based storage library that provides a customizable segmented file layout. 

The database does not support updates. This is designed to be a write-once-read-many database. The database is stored in segments, the max size of each segment can be configured. Likewise, the database can have a limit as a whole which when exceeded, the oldest segments of the database will be deleted.

## Options

The database options can be configured through the `Options` struct located in `include/ashdb/options.h`. 

* `create_if_missing`: A boolean the defines if the database should be created if it doesn't exist. Existence is defined by whether or not the folder itself exists and not by any particular file or files. The default is true.

* `error_if_exists`: A boolean that controls if an error should be generated if the database folder exists. The default is false.

* `filesize_max`: An unsigned integer that defines in bytes the upper threshold of each segment. Segment files can exceed this value since individual records are not split across segments. The default is 0 which means the segments have no size limit and everything will be stored in one data file.

* `database_max`: The upper threshold of the database size. Database size is calculated by the sum of all the database files. Index files are **not** included in this total. When the total size is exceeded, the oldest data file is deleted. The default value is 0 which means the database has no size limit.

* `prefix`: The prefix used for data files and index files. For example a value of "data" would give a filename like `data-00001.ash`. The default value is "data".

* `extension`: The extension used for data files, and prefixed for the index file extensions. For example, a value of "bin" would give a data file with a name "data-00001.bin" and an index file with the name of "data-00001.binidx". The default is "ash".
`
## Performance

This project is in early development so that the primary goal is to get basic functionality complete before optimizing for performance. This does not mean that performance is being ignored but it does mean that the path of least development will be favored for the near future.

## Repository Contents

The public interface is in `include/ashdb/*.h`. 

Header files:

* **include/ashdb/ashdb.h**: Main interface to the DB.
* **include/ashdb/options.h**: Control over the database, including control over individual file size, max database size, and more.
* **include/ashdb/status.h**: Status returned from public functions that represent various errors.
* **include/ashdb/primitives.h**: Implementation of reading and writing primitive C++ types.

## Example

```cpp#include <iostream>
#include <ashdb/ashdb.h>

namespace app
{

struct Point
{
    std::uint32_t x;
    std::uint32_t y;
    std::uint32_t z;
};

void ashdb_write(std::ostream& stream, const Point& p)
{
    ashdb::ashdb_write(stream, p.x);
    ashdb::ashdb_write(stream, p.y);
    ashdb::ashdb_write(stream, p.z);
}

void ashdb_read(std::istream& stream, Point& p)
{
    ashdb::ashdb_read(stream, p.x);
    ashdb::ashdb_read(stream, p.y);
    ashdb::ashdb_read(stream, p.z);
}

}

int main()
{
    ashdb::Options options;
    options.prefix = "points";
    options.extension = "bin";
    options.filesize_max = 1024 * 1024 * 5;

    ashdb::AshDB<app::Point> db{"./testdb", options};
    if (auto status = db.open(); status != ashdb::OpenStatus::OK)
    {
        std::cerr << ashdb::ToString(status);
        return 1;
    }

    for (auto i = 0u; i < 10000; ++i)
    {
        app::Point pt { i, 10000 - i, i + i };
        db.write(pt);
    }

    db.close();

    return 0;
}
```
