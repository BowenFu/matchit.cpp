#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

template<typename T1, typename T2>
constexpr auto eval(std::tuple<char, T1, T2> const& expr)
{
        Id<T1> i;
        Id<T2> j;
        return match(expr)(
            pattern(ds('+', i, j)) = i + j,
            pattern(ds('-', i, j)) = i - j,
            pattern(ds('*', i, j)) = i * j,
            pattern(ds('/', i, j)) = i / j,
            pattern(_)             = [] { assert(false); return -1; });
}

static_assert(std::is_literal_type_v<Id<int32_t>>);
static_assert(eval(std::make_tuple('*', 5, 6)) == 30);

int main()
{
    printf("%d\n", eval(std::make_tuple('*', 5, 6)));
    return 0;
}
