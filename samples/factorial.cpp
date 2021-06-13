#include "matchit/core.h"
#include "matchit/patterns.h"
using namespace matchit;

constexpr int32_t factorial(int32_t n)
{
    assert(n >= 0);
    return match(n)(
        pattern(0) = [] { return 1; },
        pattern(_) = [n] { return n * factorial(n - 1); }
    );
}

static_assert(factorial(3) == 6);

int main()
{
    printf("%d\n", factorial(10));
    return 0;
}
