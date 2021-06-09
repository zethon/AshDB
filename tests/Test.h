#pragma once
#include <iostream>
#include <filesystem>

#include <boost/test/data/test_case.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "../include/ashdb/ashdb.h"

constexpr auto piStr = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";
constexpr auto e100Chars = "2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274";

namespace std
{

std::ostream& operator<<(std::ostream& out, ashdb::OpenStatus status)
{
    out << ashdb::ToString(status);
    return out;
}

std::ostream& operator<<(std::ostream& out, ashdb::WriteStatus status)
{
    out << ashdb::ToString(status);
    return out;
}

}

namespace ashdb::test
{

std::string tempFolder()
{
    const auto now = std::chrono::system_clock::now();
    const auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    auto temp = std::filesystem::temp_directory_path() / std::to_string(epoch);
    temp /= std::to_string(boost::unit_test::framework::current_test_case().p_id);
    std::filesystem::create_directories(temp);
    return temp.string();
}

std::string tempFolder(const std::string& subfolder)
{
    const auto now = std::chrono::system_clock::now();
    const auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    auto temp = std::filesystem::temp_directory_path() / std::to_string(epoch);
    temp /= subfolder;
    temp /= std::to_string(boost::unit_test::framework::current_test_case().p_id);
    std::filesystem::create_directories(temp);
    return temp.string();
}

} // namespace ashdb::test