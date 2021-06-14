#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

constexpr double clip(double value, double min, double max)
{
    return match(value)(
        pattern(and_(_ >= min, _ <= max)) = [&] { return value; },
        pattern(_ > max)                  = [&] { return max; },
        pattern(_)                        = [&] { return min; }
    );
}

static_assert(clip(5, 0, 10) == 5);
static_assert(clip(5, 6, 10) == 6);
static_assert(clip(5, 0, 4) == 4);

int main()
{
    printf("%f\n", clip(10, 5,7));
    return 0;
}
