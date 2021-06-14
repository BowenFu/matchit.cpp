#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

struct Shape
{
    virtual ~Shape() = default;
};
struct Circle : Shape {};
struct Square : Shape {};

constexpr auto getClassName(Shape const &s)
{
    return match(s)(
        pattern(as<Circle>(_)) = expr("Circle"),
        pattern(as<Square>(_)) = expr("Square")
    );
}

int main()
{
    Circle c{};
    std::cout << getClassName(c) << std::endl;
    return 0;
}
