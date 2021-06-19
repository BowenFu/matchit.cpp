#include "matchit.h"
#include <iostream>

constexpr double clip(double value, double min, double max)
{
    using namespace matchit;
    return match(value)(
        // clang-format off
        pattern(and_(_ >= min, _ <= max)) = expr(value),
        pattern(_ > max)                  = expr(max),
        pattern(_)                        = expr(min)
        // clang-format on
    );
}

static_assert(clip(5, 0, 10) == 5);
static_assert(clip(5, 6, 10) == 6);
static_assert(clip(5, 0, 4) == 4);

int32_t main()
{
    std::cout << clip(10, 5,7) << std::endl;
    return 0;
}
