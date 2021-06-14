#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

constexpr bool isValid(int32_t n)
{
    return match(n)(
        pattern(or_(1, 3, 5)) = expr(true),
        pattern(_)            = expr(false)
    );
}

static_assert(isValid(5));
static_assert(!isValid(6));

int main()
{
    printf("%d\n", isValid(3));
    return 0;
}
