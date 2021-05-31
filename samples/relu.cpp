#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

double relu(double value)
{
    return match(value)(
        // pattern(meet([](auto &&v) { return v >= 0; })) = [&] { return value; },
        pattern(_ >= 0) = [&] { return value; },
        pattern(_) = [] { return 0; });
}

int main()
{
    printf("%f\n", relu(3));
    return 0;
}
