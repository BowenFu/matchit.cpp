#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

double clip(double value, double min, double max)
{
    return match(value)(
        pattern(and_(_ >= min, _ <= max)) = [&] { return value; },
        pattern(_ > max)                  = [&] { return max; },
        pattern(_)                        = [&] { return min; }
    );
}

int main()
{
    printf("%f\n", clip(10, 5,7));
    return 0;
}
