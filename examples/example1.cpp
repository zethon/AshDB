// Example 1 - This example writes 100 random digits to an AshDB
//             and iterartes the database printing them out.
#include <iostream>
#include <random>
#include <ashdb/ashdb.h>

int main()
{
    // set up the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1'000'000);

    ashdb::Options options;
    options.error_if_exists = true; 

    ashdb::AshDB<std::uint32_t> db{"./example1", options};
    if (auto status = db.open(); status != ashdb::OpenStatus::OK)
    {
        std::cerr << "Could not open database because " << ashdb::ToString(status) << '\n';
        return 1;
    }

    for (auto idx = 0u; idx < 100; ++idx)
    {
        std::uint32_t i = static_cast<std::uint32_t>(distrib(gen));
        db.write(i);
    }

    for (auto idx = 0u; idx < 100; ++idx)
    {
        std::cout << idx << " : " << db.read(idx) << '\n';
    }

    db.close();

    return 0;
}