#include "matchit.h"
#include <iostream>

constexpr int32_t factorial(int32_t n)
{
    using namespace matchit;
    assert(n >= 0);
    return match(n)(
        // clang-format off
        pattern | 0 = [] { return 1; },
        pattern | _ = [n] { return n * factorial(n - 1); }
        // clang-format on
    );
}

static_assert(factorial(3) == 6);

int32_t main()
{
    std::cout << factorial(10) << std::endl;
    return 0;
}
