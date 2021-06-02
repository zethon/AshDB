#pragma once
#include <string>
#include <limits>
#include <iostream>

namespace project
{

class Person;
using PersonDB = ashdb::AshDB<project::Person>;

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

    static Person CreatePerson(std::size_t i)
    {
        project::Person p;
        p.name.first = "Firstname" + std::to_string(i);
        p.name.middle = (i % 2) ? "Middle" + std::to_string(i) : "";
        p.name.last = "Lastname" + std::to_string(i);
        p.age = (i % 80);
        p.salary = (i % 5) * 12345.67;
        p.married = (i % 2) == 0;
        return p;
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
