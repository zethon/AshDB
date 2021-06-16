// This example shows how to use a custom type with AshDB. It is important to
// note that it is important to preserve the order in which the items and written
// to and read from the database.
#include <iostream>
#include <random>
#include <ashdb/ashdb.h>

namespace app
{

struct Point
{
    std::uint32_t x;
    std::uint32_t y;
    std::uint32_t z;
};

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

} // namespace app

namespace std
{

std::ostream& operator<<(std::ostream& out, const app::Point& pt)
{
    out << "x=" << pt.x << ", y=" << pt.y << ", z=" << pt.z;
    return out;
}

} // namespace std

int main()
{
    // we will generate some random numbers later
    std::random_device rd;
    std::mt19937 gen(rd());

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

    // load a bunch of random points and print them
    for (auto x = 0u; x < 10000; ++x)
    {
        std::uniform_int_distribution<> distrib(0, 99);
        std::uint32_t i = static_cast<std::uint32_t>(distrib(gen));

        auto pt = db.read(i);
        std::cout << pt << '\n';
    }

    db.close();

    return 0;
}