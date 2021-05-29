#include <iostream>
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