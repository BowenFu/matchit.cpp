#include <iostream>
#include "matchit.h"

void sample1()
{
    using namespace matchit;
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

void sample2()
{
    using namespace matchit;
    auto const value = Person{"John", 23};
    auto const name_age = [](auto name_pat, auto age_pat)
    {
        return and_(app(&Person::name, name_pat), app(&Person::age, age_pat));
    };
    Id<std::string> person_name;
    match(value)(
        pattern(name_age(person_name, 18 <= _ && _ <= 150)) = [] {});
}

int32_t main()
{
    sample1();
    sample2();
    return 0;
}