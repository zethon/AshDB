#pragma once
#include <iostream>

namespace ashdb
{

using PointerType = char*;
using StrLenType = std::uint32_t;

template <typename T,
    typename = typename std::enable_if<(std::is_integral<T>::value)>::type>
inline void ashdb_write(std::ostream& stream, T value)
{
    stream.write(reinterpret_cast<PointerType>(&value), sizeof(value));
}

inline void ashdb_write(std::ostream& stream, double val)
{
    stream.write(reinterpret_cast<PointerType>(&val), sizeof(double));
}

inline void ashdb_write(std::ostream& stream, std::string_view data)
{
    auto size = static_cast<StrLenType>(data.size());
    ashdb_write<StrLenType>(stream, size);
    stream.write(data.data(), size);
}

template<typename T,
    typename = typename std::enable_if<(std::is_integral<T>::value)>::type>
inline void ashdb_read(std::istream& stream, T& value)
{
    stream.read(reinterpret_cast<PointerType>(&value), sizeof(value));
}

inline void ashdb_read(std::istream& stream, double& val)
{
    stream.read(reinterpret_cast<PointerType>(&val), sizeof(double));
}

inline void ashdb_read(std::istream& stream, std::string& data)
{
    StrLenType len = 0;
    ashdb_read(stream, len);

    data.resize(len);
    stream.read(reinterpret_cast<PointerType>(data.data()), len);
}

} // namespace ashdb