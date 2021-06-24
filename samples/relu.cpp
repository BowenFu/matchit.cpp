#include "matchit.h"
#include <iostream>

constexpr double relu(double value)
{
    using namespace matchit;
    return match(value)(
        // clang-format off
        pattern | (_ >= 0) = expr(value),
        pattern | _        = expr(0)
        // clang-format on
    );
}

static_assert(relu(5) == 5);
static_assert(relu(-5) == 0);

int32_t main()
{
    std::cout << relu(3) << std::endl;
    return 0;
}
