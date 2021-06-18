#include "matchit.h"


#include <iostream>
using namespace matchit;

constexpr double relu(double value)
{
    return match(value)(
        // pattern(meet([](auto &&v) { return v >= 0; })) = [&] { return value; },
        pattern(_ >= 0) = [&] { return value; },
        pattern(_) = [] { return 0; });
}

static_assert(relu(5) == 5);
static_assert(relu(-5) == 0);

int main()
{
    std::cout << relu(3) << std::endl;
    return 0;
}
