#include <array>
#include "matchit.h"


#include <iostream>
using namespace matchit;

constexpr bool sumIs(std::array<int32_t, 2> const& arr, int s)
{
    Id<int32_t> i, j;
    return match(arr)(
        pattern(i, j).when(i + j == s) = expr(true),
        pattern(_)                     = expr(false));
}

static_assert(sumIs(std::array<int32_t, 2>{5, 6}, 11));

int main()
{
    std::cout << sumIs(std::array<int32_t, 2>{5, 6}, 11) << std::endl;
    return 0;
}
