#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
#include <iostream>
using namespace matchit;

constexpr bool symmetric(std::array<int32_t, 5> const& arr)
{
    Id<int32_t> i, j; 
    return match(arr)(
        pattern(i, j, _, j, i) = expr(true),
        pattern(_)             = expr(false)
    );
}

static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) == false);
static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) == true);
static_assert(symmetric(std::array<int32_t, 5>{5, 1, 3, 0, 5}) == false);

int main()
{
    std::cout << symmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) << std::endl;
    std::cout << symmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) << std::endl;
    return 0;
}
