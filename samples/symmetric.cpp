#include "matchit.h"
#include <iostream>
using namespace matchit;

template <typename Range>
constexpr bool symmetric(Range const &range)
{
    Id<int32_t> i;
    Id<SubrangeT<Range const>> subrange;
    return match(range)(
        // clang-format off
        pattern(i, ooo(subrange), i) = [&] { return symmetric(*subrange); },
        pattern(i, ooo(subrange), _) = expr(false),
        pattern(_)                   = expr(true)
        // clang-format on
    );
}

// static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) == false);
// static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) == true);
// static_assert(symmetric(std::array<int32_t, 5>{5, 1, 3, 0, 5}) == false);

int main()
{
    std::cout << symmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) << std::endl;
    std::cout << symmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) << std::endl;
    return 0;
}
