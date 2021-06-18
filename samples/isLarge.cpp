#include "matchit.h"


#include <iostream>
using namespace matchit;

constexpr bool isLarge(double value)
{
    return match(value)(
        pattern(app(_ * _, _ > 1000)) = expr(true),
        pattern(_)                    = expr(false)
    );
}

// app with projection returning scalar types is supported by constexpr match.
static_assert(isLarge(100));

int main()
{
    std::cout << isLarge(10) << std::endl;
    return 0;
}
