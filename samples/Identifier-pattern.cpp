#include <iostream>
#include <optional>
#include "matchit.h"
using namespace matchit;

void sample1()
{
    constexpr auto x = 2;
    Id<int32_t> e;
    match(x)(
        // clang-format off
        pattern(e.at(1 <= _ && _ <= 5)) = [&] { std::cout << "got a range element " << *e << std::endl; },
        pattern(_)                      = [&] { std::cout << "anything" << std::endl; }
        // clang-format on
    );
}

struct Person {
   std::string name;
   uint8_t age;
};

auto const name_age = [](auto name_pat, auto age_pat)
{
    return and_(app(&Person::name, name_pat), app(&Person::age, age_pat));
};

void sample2()
{
    auto const value = Person{"John", 23};
    Id<std::string> person_name;
    match(value)(
        pattern(name_age(person_name, 18 <= _ && _ <= 150)) = [] {});
}
void sample3()
{
    constexpr auto x = std::make_optional(3);
    Id<int32_t> y;
    match(x)(
        // No need to worry about y's type, by ref or by value is automatically managed by `match(it)` library.
        pattern(some(y)) = [] {});
}

void sample4()
{
    Id<std::string> person_name;
    Id<uint8_t> age;
    // auto value = Person{"John", 23};
    match(Person{"John", 23})(
    // match(std::move(value))(
        // `name` is moved from person and `age` copied (scalar types are copied in `match(it)`)
        pattern(name_age(person_name, age)) = [] {});
}

int32_t main()
{
    sample1();
    sample2();
    sample3();
    return 0;
}