#include "matchit.h"
#include <iostream>
using namespace matchit;

constexpr int32_t factorial(int32_t n)
{
    assert(n >= 0);
    return match(n)(
        // clang-format off
        pattern(0) = [] { return 1; },
        pattern(_) = [n] { return n * factorial(n - 1); }
        // clang-format on
    );
}

static_assert(factorial(3) == 6);

int main()
{
    std::cout << factorial(10) << std::endl;
    return 0;
}
