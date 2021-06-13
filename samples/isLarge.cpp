#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

constexpr bool isLarge(int32_t value)
{
    return match(value)(
        pattern(app([](int32_t x) { return x * x; }, _ > 1000)) = [] { return true; },
        pattern(_)                                              = [&] { return false; }
    );
}

// static_assert(isLarge(100));

int main()
{
    printf("%d\n", isLarge(10));
    return 0;
}
