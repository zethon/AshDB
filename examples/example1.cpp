#include <iostream>
#include <ashdb/ashdb.h>

int main()
{
    ashdb::Options options;
    options.filesize_max = 256; // 256 bytes
    options.database_max = 1024 * 10;; // 5k

    ashdb::AshDB<std::string> db{"./testdb", "file-{}.db}", options};

    const std::string data = "0123456789abcdef0123456789abcdef";

    for (auto i = 0u; i < 10000; ++i)
    {
        db.write(data);
    }

    db.close();

    return 0;
}