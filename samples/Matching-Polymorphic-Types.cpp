#include "matchit.h"
#include <iostream>
#include <tuple>

struct Shape { virtual ~Shape() = default; };
struct Circle : Shape { int radius; };
struct Rectangle : Shape { int width, height; };

template <size_t I>
constexpr auto &get(Circle const &c)
{
    if constexpr (I == 0)
    {
        return c.radius;
    }
}

template <size_t I>
constexpr auto &get(Rectangle const &r)
{
    if constexpr (I == 0)
    {
        return r.width;
    }
    else if constexpr (I == 1)
    {
        return r.height;
    }
}

namespace std
{
    template <>
    class tuple_size<Circle> : public std::integral_constant<size_t, 1>
    {
    };
    template <>
    class tuple_size<Rectangle> : public std::integral_constant<size_t, 2>
    {
    };
} // namespace std

double get_area(const Shape& shape)
{
    using namespace matchit;
    Id <int> r, w, h;
    return match(shape) ( 
        // clang-format off
        pattern | as<Circle>(ds(r))       = 3.14 * r * r,
        pattern | as<Rectangle>(ds(w, h)) = w * h
        // clang-format on
    );
}

int32_t main()
{
    auto c = Circle{};
    c.radius = 5;
    std::cout << get_area(c) << std::endl;

    return 0;
}
