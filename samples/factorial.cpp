#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

int32_t factorial(int32_t n)
{
    assert(n >= 0);
    return match(n)(
        pattern(0) = [] { return 1; },
        pattern(_) = [n] { return n * factorial(n - 1); }
    );
}

int main()
{
    printf("%d\n", factorial(10));
    return 0;
}
