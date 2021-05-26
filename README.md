# AshDB

AshDB is a 

```cpp
// define custom class
class Person
{
    std::uint32_t age;
    std::string name;
};

// define accessors to your class using primitives
// give to you by the library
void ashdb_write(std::ostream& stream, const Person& person)
{
    ashdb::write_data(stream, person.age);
    ashdb::write_data(stream, person.name);
}

void ashdb_read(std::istream& istream, Person& person)
{
    ashdb::read_data(istream, person.age);
    ashdb::read_data(istream, person.name);
}

// set your options
ashdb::Options options;
options.filesize_max = 1024 * 1024 * 10; // 10 MB
option.dbsize_max = 1024 * 1024 * 1024 * 10; // 10 GB

// initialize the library
AshDB<Person> db{"/path/to/db", "file-{}.db", options};

// write out a bunch of data
for (auto i = 0u; i < 1000; ++i)
{
    const std::uint32_t age = i % 80;
    const std::string name = fmt::format("Person-{}", i);
    
    Person person{ age, name };
    db.write(person);
}

// iterate over the data
const auto max = db.size();
for (auto x = 0u; x < max; +x)
{
    Person person = db.read(x);
    std::cout << "name: " << person.name << " age: " << person.age << '\n';
}

// close the database
db.close();
```
