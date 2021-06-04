#include "include/core.h"
#include "include/patterns.h"
#include "include/expression.h"
using namespace matchit;

bool isLarge(double value)
{
    return match(value)(
        pattern(app([](int32_t x) { return x * x; }, _ > 1000)) = [] { return true; },
        pattern(_)                                              = [&] { return false; }
    );
}

int main()
{
    printf("%d\n", isLarge(10));
    return 0;
}
