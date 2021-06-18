#include "matchit.h"
using namespace matchit;

constexpr int32_t fib(int32_t n)
{
    assert(n >= 1);
    return match(n)(
        // clang-format off
        pattern(1) = expr(1),
        pattern(2) = expr(1),
        pattern(_) = [n] { return fib(n - 1) + fib(n - 2); }
        // clang-format on
    );
}

static_assert(fib(1) == 1);
static_assert(fib(2) == 1);
static_assert(fib(3) == 2);
static_assert(fib(4) == 3);
static_assert(fib(5) == 5);

template <typename Value>
constexpr auto eval(Value &&input)
{
    return match(input)(
        pattern(ds('/', 1, 1)) = expr(1),
        pattern(ds('/', 0, _)) = expr(0),
        pattern(_) = expr(-1));
}

static_assert(eval(std::make_tuple('/', 0, 5)) == 0);
