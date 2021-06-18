#include <iostream>
#include "matchit.h"
using namespace matchit;

template <typename T>
constexpr auto getClassName(T const &v)
{
    return match(v)(
        // clang-format off
        pattern(as<char const *>(_)) = expr("chars"),
        pattern(as<int32_t>(_))      = expr("int")
        // clang-format on
    );
}

constexpr std::variant<int32_t, char const *> cv = 123;
static_assert(getClassName(cv) == std::string_view{"int"});

template <typename T>
constexpr auto print(T const &v)
{
    Id<char const *> c;
    Id<int32_t> i;
    return match(v)(
        // clang-format off
        pattern(as<char const *>(c)) = [&] { std::cout << "char const *: " << *c << std::endl;},
        pattern(as<int32_t>(i))      = [&] { std::cout << "int: " << *i << std::endl;}
        // clang-format on
    );
}

int main()
{
    std::variant<char const *, int32_t> v = 5;
    print(v);
    v = "123";
    print(v);
    std::any a = "arr";
    print(a);
    return 0;
}
