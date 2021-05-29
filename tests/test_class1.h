#pragma once
#include <string>
#include <limits>
#include <iostream>

namespace project
{

struct Name
{
    std::string first;
    std::string middle;
    std::string last;

    bool operator==(const Name& other) const
    {
        return first == other.first
            && middle == other.middle
            && last == other.last;
    }
};

struct Person
{
    Name name;
    std::uint8_t age;
    double salary;
    bool married;

    bool operator==(const Person& other) const
    {
        return name == other.name
            && age == other.age
            && (fabs(this->salary - other.salary) <= std::numeric_limits<double>::epsilon())
            && married == other.married;
        ;
    }
};

void ashdb_write(std::ostream &stream, const project::Name& name)
{
    ashdb::ashdb_write(stream, name.first);
    ashdb::ashdb_write(stream, name.middle);
    ashdb::ashdb_write(stream, name.last);
}

void ashdb_write(std::ostream& stream, const project::Person& person)
{
    ashdb_write(stream, person.name);
    ashdb::ashdb_write(stream, person.age);
    ashdb::ashdb_write(stream, person.salary);
    ashdb::ashdb_write(stream, person.married);
}

void ashdb_read(std::istream& stream, project::Name& name)
{
    ashdb::ashdb_read(stream, name.first);
    ashdb::ashdb_read(stream, name.middle);
    ashdb::ashdb_read(stream, name.last);
}

void ashdb_read(std::istream& stream, project::Person& person)
{
    ashdb_read(stream, person.name);
    ashdb::ashdb_read(stream, person.age);
    ashdb::ashdb_read(stream, person.salary);
    ashdb::ashdb_read(stream, person.married);
}

} // namespace project
