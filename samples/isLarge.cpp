#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
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
    printf("%d\n", isLarge(10));
    return 0;
}
