#include <array>
#include "matchit.h"
#include <iostream>

template <typename Tuple>
constexpr int32_t detectTuplePattern(Tuple const &tuple)
{
    using namespace matchit;
    return match(tuple)(
        // clang-format off
        pattern(2, ooo, 2) = expr(4),
        pattern(2, ooo)    = expr(3),
        pattern(ooo, 2)    = expr(2),
        pattern(ooo)       = expr(1)
        // clang-format on
    );
}

static_assert(detectTuplePattern(std::make_tuple(2, 3, 5, 7, 2)) == 4);

int32_t main()
{
    std::cout << detectTuplePattern(std::make_tuple(2, 3, 5, 7, 2)) << std::endl;
    std::cout << detectTuplePattern(std::make_tuple(2, 3, 4, 5, 6)) << std::endl;
    std::cout << detectTuplePattern(std::make_tuple(3, 3, 3, 2)) << std::endl;
    std::cout << detectTuplePattern(std::make_tuple(3, 4, 5, 6, 7)) << std::endl;
    return 0;
}
