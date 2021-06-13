#include "matchit/core.h"
#include "matchit/patterns.h"
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
    printf("%d\n", gcd(12, -15));
    return 0;
}
