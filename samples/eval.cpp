#include "matchit.h"
#include <iostream>

template <typename T1, typename T2>
constexpr auto eval(std::tuple<char, T1, T2> const &expr)
{
    using namespace matchit;
    Id<T1> i;
    Id<T2> j;
    return match(expr)(
        pattern('+', i, j) = i + j,
        pattern('-', i, j) = i - j,
        pattern('*', i, j) = i * j,
        pattern('/', i, j) = i / j,
        pattern(_) = []
        {
            assert(false);
            return -1;
        });
}

#if __cplusplus > 201703L
constexpr auto result = eval(std::make_tuple('*', 5, 6));
static_assert(result == 30);
#endif

int32_t main()
{
    std::cout << eval(std::make_tuple('*', 5, 6)) << std::endl;
    return 0;
}
