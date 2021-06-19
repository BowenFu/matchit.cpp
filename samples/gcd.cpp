#include "matchit.h"
#include <iostream>

constexpr int32_t gcd(int32_t a, int32_t b)
{
    using namespace matchit;
    return match(a, b)(
        // clang-format off
        pattern(_, 0) = [&] { return a >= 0 ? a : -a; },
        pattern(_)    = [&] { return gcd(b, a % b); }
        // clang-format on
    );
}

static_assert(gcd(12, 6) == 6);

int32_t main()
{
    std::cout << gcd(12, -15) << std::endl;
    return 0;
}
