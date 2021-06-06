#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
using namespace matchit;

template <typename T>
auto getClassName(T const& v)
{
    return match(v)(
        pattern(as<std::string>(_)) = [] { return "string"; },
        pattern(as<int32_t>(_)) = [] { return "int"; }
    );
}

int main()
{
    std::variant<std::string, int32_t> v = 5;
    std::cout << getClassName(v) << std::endl;
    std::any a = std::string("arr");
    std::cout << getClassName(a) << std::endl;
    return 0;
}
