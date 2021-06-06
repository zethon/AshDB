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