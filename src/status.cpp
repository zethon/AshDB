#include <stdexcept>

#include "../include/ashdb/status.h"

namespace ashdb
{

std::string ToString(OpenStatus status)
{
    switch (status)
    {
        default:
            throw std::runtime_error("invalid OpenStatus");
        case OpenStatus::OK:
            return "OK";
        case OpenStatus::EXISTS:
            return "EXISTS";
        case OpenStatus::NOT_FOUND:
            return "NOT_FOUND";
        case OpenStatus::INVALID_PREFIX:
            return "INVALID_PREFIX";
        case OpenStatus::INVALID_EXTENSION:
            return "INVALID_EXTENSION";
        case OpenStatus::ALREADY_OPEN:
            return "ALREADY_OPEN";
    }
}

std::string ToString(WriteStatus status)
{
    switch (status)
    {
        default:
            throw std::runtime_error("invalid WriteStatus");
        case WriteStatus::OK:
            return "OK";
        case WriteStatus::DATABASE_NOT_OPEN:
            return "DATABASE_NOT_OPEN";
    }
}

} // namespace ashdb
