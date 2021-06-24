#include <iostream>
#include <vector>
#include "matchit.h"
using namespace matchit;

void sample1()
{
    // Fixed size
    constexpr auto arr = std::array<int32_t, 3>{1, 2, 3};
    Id<int32_t> a, b, c;
    match(arr)(
        pattern | ds(1, _, _) = expr("starts with one"),
        pattern | ds(a, b, c) = expr("starts with something else"));
}

void sample2()
{
    // Dynamic size
    auto const v = std::vector<int32_t>{1, 2, 3};
    Id<int32_t> a, b, c;
    match(v)(
        // format off
        pattern | ds(a, b   ) = [] { /* this arm will not apply because the length doesn't match */ },
        pattern | ds(a, b, c) = [] { /* this arm will apply */ },
        pattern | ds(_      ) = [] { /* this wildcard is required, since the length is not known statically */ }
        // format on
    );
}

int32_t main()
{
    sample1();
    sample2();
    return 0;
}