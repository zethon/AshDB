#pragma once
#include <string>

namespace ashdb
{
    
enum class OpenStatus
{
    OK,
    EXISTS,
    NOT_FOUND,
    INVALID_PREFIX,
    INVALID_EXTENSION,
    ALREADY_OPEN
};

std::string ToString(OpenStatus status);

enum class WriteStatus
{
    OK,
    DATABASE_NOT_OPEN,
    INDEX_FILE_ERROR
};

std::string ToString(WriteStatus status);

} // namespace ashdb

