AshDB
=======

The AshDB library provides a persistant indexed storage system. Data are stored in _segments_ that have a configurable maximum size. Data is retrived by index. 

## Database Types

The `ashdb::AshDB` class is the main database type. This is a templated class which accepts a single parameter -- the type of object that is to be stored in the database. 

The constructor itself accepts two parameters, the first is a `cosnt std::string&` to the name of the database, and the second is an optional argument of type `ashdb::Options`. These arguments are discussed later in the section _Opening a Database_. 

For example, to create a database of `std::uint32_t` integers, the database object would be declared as follows:

```cpp
#include <ashdb/ashdb.h>

ashdb::AshDB<std::uint32_t> db("integers");
```

AshDB supports C++ intergral and float types. AshDB also supports `std::string` by default, hence one could declare a database of strings like so:

```cpp
ashdb::AshDB<std::string> stringdb("strings");
```

The primitive types are implemented in `include/ashdb/primitives.h`. 

### Custom Data Types

Any data type not natively supported by AshDB can be used with AshDB by implementing read and write functions for that type. The read and and write functions can be composed from the primitive functions `ashdb_write()` and `ashdb_read()`. 

For example consider a 3D `Point` structure that consists of `x`,`y` and `z` members.

```cpp
struct Point
{
    std::uint32_t x;
    std::uint32_t y;
    std::uint32_t z;
};
```

In order to use this type with AshDB we implement `ashdb_write` and `ashdb_read`. These functions must be defined in the same namepsace as the type itself.


```cpp
void ashdb_read(std::istream& stream, Point& p)
{
    ashdb::ashdb_read(stream, p.x);
    ashdb::ashdb_read(stream, p.y);
    ashdb::ashdb_read(stream, p.z);
}

void ashdb_write(std::ostream& stream, const Point& p)
{
    ashdb::ashdb_write(stream, p.x);
    ashdb::ashdb_write(stream, p.y);
    ashdb::ashdb_write(stream, p.z);
}
```

**It is important to note that the two functions must read and write the same data members in the same order**.

Now we can declare an AshDB that supports our `Point` type.

```cpp
ashdb::AshDB<Point> pointdb("points");
```

Types can also be composed. For example if we wanted to define a `Triangle` type of three `Point` objects then  we can compose the read and write functions like so:

```cpp
struct Triangle
{
    Point a;
    Point b;
    Point c;
};

void ashdb_read(std::istream& stream, Triangle& t)
{
    ashdb_read(stream, t.a);
    ashdb_read(stream, t.b);
    ashdb_read(stream, t.c);
}

void ashdb_write(std::ostream& stream, const Triangle& t)
{
    ashdb_write(stream, t.a);
    ashdb_write(stream, t.b);
    ashdb_write(stream, t.c);
}
```

Notice that we used our previously defined `ashdb_read` and `ashdb_write` functions, which means we had to use the same namespace. Now we can define a `Triangle` database:

```cpp
ashdb::AshDB<Triangle> triangledb("triangles");
```

## Opening a Database

As discussed above, the `AshDB` constructor accepts one parameter, the name of the database, and an optional parameter of type `Options`. 

The name of the AshDB corresponds to a file system directory. All contents of the database are stored in this directory. The string can be a relative path, a full path to the database, or simply a name which means the process look for the database in the current working folder.

The `Options` parameter defines some behavior for the database. For example the default behavior will create the database folder in the `AshDB` constructor if it does not exist, thus the follow example will create the folder and open the database for reading and writing:

```cpp
#include <ashdb/ashdb.h>

...
ashdb::AshDB<std::string> stringdb("StringDB");
ashdb::OpenStatus status = stringdb.open();
assert(status == ashdb::OpenStatus::OK);
```

This will create a directory named "StringDB" in the current working directory if it doesn't already exist. If we instead want to generate an error if the folder already exists then the default behavior can be override with the `Options` parameter on the constructor:

```cpp
#include <asdhb/options.h>

ashdb::Options options;
options.error_if_exists = true;

ashdb::AshDB<std::string> stringdb("StringDB", options);
ashdb::OpenStatus status = stringdb.open();
assert(status == ashdb::OpenStatus::OK); // will fail if folder exists
```

### Options

The `Options` struct is located in `include/ashdb/options.h`. 

* `create_if_missing`: Create the database if it doesn't exist. Existence is defined by whether or not the folder itself exists and not by any particular file or files. This condition is checked in the `AshDB<T>::open()` method. The default is true.

* `error_if_exists`: Generate an error if the database exists. Existence is defined by whether or not the folder itself exists and not by any particular file or files. This condition is checked in the `AshDB<T>::open()` method. The default is false.

* `filesize_max`: An unsigned integer that defines in bytes the upper size limit of each segment. Segment files can exceed this value since individual records are not split across segments. The default is 0 which means the segments have no size limit and everything will be stored in one data file.

* `database_max`: The upper size limit of the database size. Database size is calculated by the sum of all the database files. Index files are **not** included in this total. When the total size is exceeded, the oldest data file is deleted. The default value is 0 which means the database has no size limit.

* `prefix`: The prefix used for data files and index files. For example a value of "data" would give a filename like `data-00001.ash`. The default value is "data".

* `extension`: The extension used for data files, and prefixed for the index file extensions. For example, a value of "bin" would give a data file with a name "data-00001.bin" and an index file with the name of "data-00001.binidx". The default is "ash".

## Closing a Database

Closing a database will reset the state of the database object to what it was before it was opened.

```cpp
ashdb::AshDB<std::string> stringdb("stringdb");
assert(db.opened() == false);
assert(db.open() == ashdb::OpenStatus::OK);
assert(db.opened() == true);
db.close();
assert(db.opened() == false);
```

## Writing Data

A single record can be appended to the database with the `AshDB<T>::write()` function which returns a `WriteStatus`,

### Batch Writing

Batch writing allows for faster writing by putting all the records to be written into an `std::vector` and writing them at once. 

## Reading Data

Reading data returns the data type at a given index.

### Batch Reads

Batch reads allow you to read a batch of data sequentially stored. 

### Iteration

Here is an example of iterating all the records in a string database.

Example 1 - Using Indicies
```cpp
ashdb::AshDB<std::string> stringdb("stringdb");
assert(db.opened() == false);
assert(db.open() == ashdb::OpenStatus::OK);
for (auto i = 0u; i < db.size(); ++i)
{
    std::string str = db.read(i);
    std::cout << str << '\n';
}
```

Example 2 - Using Ranges
```cpp
ashdb::AshDB<std::string> stringdb("stringdb");
assert(db.opened() == false);
assert(db.open() == ashdb::OpenStatus::OK);
for (auto str : db)
{
    std::cout << str << '\n';
}
```

Example 3 - Using Iterators
```cpp
ashdb::AshDB<std::string> stringdb("stringdb");
assert(db.opened() == false);
assert(db.open() == ashdb::OpenStatus::OK);
for (auto it = db.begin(); it != db.end(); ++it)
{
    std::cout << *it << '\n';
}
```

## Truncating Data

The database can be truncated to a specific number of records. For example, to truncate the size down to the first 50 records we can use `AshDB::truncate()`

```cpp
ashdb::AshDB<std::string> strindb("strings");
assert(db.open() == ashdb::OpenStatus::OK);
db.truncate(50);
```

Data can only be removed from the end of the database.
## Segments

All data are stored in segments. Each segment consists of a database file and an index file that are located in the database folder. The data files are named using the format:

```
<prefix>-<segment>.<extension>
```

The `<prefix>` and `<extension>` can be set using `Options::prefix` and `Options::extension` respectively. The `<segment>` corresponds to the segment number which is determined by the maxium size of each segment, which can be set using `Options::filesize_max` when constructing the database object. 

Each data file has a corresponding index file that is named:

```
<prefix>-<segment>.<extension>idx
```

The "idx" suffix is hardcoded and cannot be changed. 




