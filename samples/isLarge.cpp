#include "matchit.h"
#include <iostream>

constexpr bool isLarge(double value)
{
    using namespace matchit;
    return match(value)(
        // clang-format off
        pattern(app(_ * _, _ > 1000)) = expr(true),
        pattern(_)                    = expr(false)
        // clang-format on
    );
}

// app with projection returning scalar types is supported by constexpr match.
static_assert(isLarge(100));

int main()
{
    std::cout << isLarge(10) << std::endl;
    return 0;
}
