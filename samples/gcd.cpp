#include "matchit.h"

#include <iostream>
using namespace matchit;

constexpr int32_t gcd(int32_t a, int32_t b)
{
    return match(a, b)(
        pattern(_, 0) = [&] { return a >= 0 ? a : -a; },
        pattern(_) = [&] { return gcd(b, a % b); });
}

static_assert(gcd(12, 6) == 6);

int main()
{
    std::cout << gcd(12, -15) << std::endl;
    return 0;
}
