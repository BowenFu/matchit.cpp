#include <iostream>
#include <optional>
#include "matchit.h"
using namespace matchit;

void sample1()
{
    constexpr auto x = 20;
    Id<int32_t> a;
    match(10, x)(
        // the x is always matched by _
        pattern | ds(a, _) = [&] { assert(*a == 10); });
}

void sample2()
{
    // ignore a function/closure param
    constexpr auto real_part = [](float a, float) { return a; };
    static_cast<void>(real_part);

    // ignore a field from a struct
    struct RGBA
    {
        float r;
        float g;
        float b;
        float a;
    };

    constexpr auto color = RGBA{0.4f, 0.1f, 0.9f, 0.5f};
    constexpr auto dsRGBA = [](auto r, auto g, auto b, auto a)
    {
        return and_(app(&RGBA::r, r),
                    app(&RGBA::g, g),
                    app(&RGBA::b, b),
                    app(&RGBA::a, a));
    };

    Id<float> red, green, blue;
    match(color)(
        pattern | dsRGBA(red, green, blue, _) = [&]
        {
            assert(color.r == *red);
            assert(color.g == *green);
            assert(color.b == *blue);
        });

    // accept any Some, with any value
    constexpr auto x = std::make_optional(10);
    match(x)(
        // the x is always matched by _
        pattern | some(_) = [] {});
}

int32_t main()
{
    sample1();
    sample2();
    return 0;
}