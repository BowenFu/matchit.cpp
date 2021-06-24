#include <iostream>
#include "matchit.h"
using namespace matchit;

template <size_t I>
constexpr auto get = [](auto &&pt)
{
    return std::get<I>(pt);
};

template <size_t I>
constexpr auto appGet = [](auto &&pat)
{
    return app(get<I>, pat);
};

void sample()
{
    struct Point
    {
        uint32_t x;
        uint32_t y;
    };
    constexpr auto s = Point{1U, 1U};

    match(s)(
        // clang-format off
        pattern(and_(app(&Point::x, 10U), app(&Point::y, 20U))) = []{},
        pattern(and_(app(&Point::y, 10U), app(&Point::x, 20U))) = []{},    // order doesn't matter
        pattern(app(&Point::x, 10U))                            = []{},
        pattern | _                                              = []{}
        // clang-format on
    );

    using PointTuple = std::tuple<uint32_t, uint32_t>;
    constexpr auto t = PointTuple{1U, 2U};

    match(t)(
        // clang-format off
        pattern(and_(appGet<0>(10U), appGet<1>(20U))) = []{},
        pattern(and_(appGet<1>(10U), appGet<0>(20U))) = []{},   // order doesn't matter
        pattern(appGet<0>(10U))                       = []{},
        pattern | _                                    = []{}
        // clang-format on
    );
}

int32_t main()
{
    sample();
    return 0;
}