#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

template<typename T1, typename T2>
auto eval(std::tuple<char, T1, T2> const& expr)
{
        Id<T1> i;
        Id<T2> j;
        return match(expr)(
            pattern(ds('+', i, j)) = [&i, &j] { return *i + *j; },
            pattern(ds('-', i, j)) = [&i, &j] { return *i - *j; },
            pattern(ds('*', i, j)) = [&i, &j] { return *i * *j; },
            pattern(ds('/', i, j)) = [&i, &j] { return *i / *j; },
            pattern(_) = [&i, &j] { assert(false); return -1; });
}

int main()
{
    printf("%d\n", eval(std::make_tuple('*', 5, 6)));
    return 0;
}
