#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

int32_t gcd(int32_t a, int32_t b)
{
    return match(a, b)(
        pattern(_, 0) = [&] { return std::abs(a); },
        pattern(_)    = [&] { return gcd(b, a%b); }
    );
}

int main()
{
    printf("%d\n", gcd(12, -15));
    return 0;
}
