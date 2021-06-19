#include "matchit.h"
#include <iostream>
using namespace matchit;

template <typename Range>
constexpr bool recursiveSymmetric(Range const &range)
{
    Id<int32_t> i;
    Id<SubrangeT<Range const>> subrange;
    return match(range)(
        // clang-format off
        pattern(i, ooo(subrange), i) = [&] { return recursiveSymmetric(*subrange); },
        pattern(i, ooo(subrange), _) = expr(false),
        pattern(_)                   = expr(true)
        // clang-format on
    );
}

constexpr bool symmetricArray(std::array<int32_t, 5> const &arr)
{
    Id<int32_t> i, j;
    return match(arr)(
        // clang-format off
         pattern(i, j, _, j, i) = expr(true),
         pattern(_)             = expr(false)
        // clang-format on
    );
}

static_assert(symmetricArray(std::array<int32_t, 5>{5, 0, 3, 7, 10}) == false);
static_assert(symmetricArray(std::array<int32_t, 5>{5, 0, 3, 0, 5}) == true);
static_assert(symmetricArray(std::array<int32_t, 5>{5, 1, 3, 0, 5}) == false);

int main()
{
    std::cout << recursiveSymmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) << std::endl;
    std::cout << recursiveSymmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) << std::endl;
    return 0;
}
