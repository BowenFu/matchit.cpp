#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
using namespace matchit;

template <typename T>
auto square(T const* t)
{
    Id<T> id;
    return match(t)(
        pattern(some(id)) = [&id] { return *id * *id; },
        pattern(none) = [] { return 0; });
}

int main()
{
    auto t = 3;
    std::cout << square(&t) << std::endl;
    return 0;
}
