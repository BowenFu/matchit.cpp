#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
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
