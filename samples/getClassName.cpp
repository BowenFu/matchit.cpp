#include <iostream>
#include "matchit.h"

struct Shape
{
    virtual ~Shape() = default;
};
struct Circle : Shape
{
};
struct Square : Shape
{
};

constexpr auto getClassName(Shape const &s)
{
    using namespace matchit;
    return match(s)(
        pattern | as<Circle>(_) = expr("Circle"),
        pattern | as<Square>(_) = expr("Square"));
}

int32_t main()
{
    Circle c{};
    std::cout << getClassName(c) << std::endl;
    return 0;
}
