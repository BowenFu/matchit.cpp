#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
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
    printf("%f\n", relu(3));
    return 0;
}
