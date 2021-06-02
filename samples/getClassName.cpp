#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
#include "include/utility.h"
using namespace matchit;

struct Shape
{
    virtual ~Shape() = default;
};
struct Circle : Shape {};
struct Square : Shape {};

auto getClassName(Shape const &s)
{
    return match(s)(
        pattern(as<Circle>(_)) = [] { return "Circle"; },
        pattern(as<Square>(_)) = [] { return "Square"; }
    );
}

int main()
{
    Circle c{};
    std::cout << getClassName(c) << std::endl;
    return 0;
}
