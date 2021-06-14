#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

template <typename T>
constexpr auto getClassName(T const& v)
{
    return match(v)(
        pattern(as<char const*>(_)) = expr("chars"),
        pattern(as<int32_t>(_))     = expr("int")
    );
}

constexpr std::variant<int32_t, char const*> v = 123;
static_assert(getClassName(v) == std::string_view{"int"});

int main()
{
    std::variant<char const*, int32_t> v = 5;
    std::cout << getClassName(v) << std::endl;
    std::any a = "arr";
    std::cout << getClassName(a) << std::endl;
    return 0;
}
