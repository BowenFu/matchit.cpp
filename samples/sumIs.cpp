#include <array>
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

bool sumIs(std::array<int32_t, 2> const& arr, int s)
{
    Id<int32_t> i, j;
    return match(arr)(
        pattern(i, j).when([&] { return *i + *j == s; }) = [] { return true; },
        pattern(_) = [] { return false; });
}

int main()
{
    printf("%d\n", sumIs(std::array<int32_t, 2>{5, 6}, 11));
    return 0;
}
