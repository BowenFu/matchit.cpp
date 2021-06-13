#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

constexpr int32_t fib(int32_t n)
{
    assert(n >= 1);
    return match(n)(
        pattern(1) = expr(1),
        pattern(2) = expr(1),
        pattern(_) = [n] { return fib(n - 1) + fib(n - 2); });
}

static_assert(fib(1) == 1);
