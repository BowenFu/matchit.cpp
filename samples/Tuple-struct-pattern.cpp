#include <iostream>
#include "matchit.h"
using namespace matchit;

void sample()
{
    constexpr auto pair = std::make_pair(10, "ten");
    Id<int32_t> a;
    Id<char const *> b;
    match(pair)(
        pattern(a, b) = [&]
        {
            assert(*a == 10);
            assert(*b == "ten");
        });
}

int32_t main()
{
    sample();
    return 0;
}