AshDB
=======

The AshDB library provides a persistant indexed storage system. Data are stored in _segments_ that have a configurable maximum size. Data is retrived by index. 

## Database Types

The `ashdb::AshDB` class is the database class. This is a templated class which accepts a single parameter -- the type of object that is to be stored in the database. For example, to create a database of `std::uint32_t` integers, the database object would be declared as follows:

```cpp
#include <ashdb/ashdb.h>

ashdb::AshDB<std::uint32_t> db;
```

AshDB supports C++ intergral and float types. AshDB also supports `std::string` by default, hence one could declare a database of strings like so:

```cpp
ashdb::AshDB<std::string> stringdb;
```

### Custom Data Types

Any data type not natively supported by AshDB can be extended by implementing read/write functions. The read and and write functions are composed from the primitive functions `ashdb_write()` and `ashdb_read()`. 

For example consider a 3D `Point` structure that consists of `x`,`y` and `z` members:

```cpp
struct Point
{
    std::uint32_t x;
    std::uint32_t y;
};
```

In order to use this type with AshDB we need two functions, `ashdb_write` and `ashdb_read`, that are in the same namespace as the type:

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

Now we can decalre an AshDB that supports `Point`:

```cpp
ashdb::AshDB<Point> pointdb;
```

Types can also be composed. For example if we wanted to define a `Triangle` type defined by three `Point` objects, then  we can compose the read and write functions. 

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
ashdb::AshDB<Triangle> triangledb;
```

## Opening a Database

An AshDB has a name which corresponds to a directory on the filesystem. All the contents for the database are stored within that directory. The following example shows how to open a database, creating it if necessary:

```cpp
#include <ashdb/ashdb.h>

...

```

### Options

## Closing a Database

## Custom Data Types


## Writing Data

### Batch Writing

## Reading Data

### Batch Reads

### Iteration

## Truncating Data

## Segments

## Index Files


