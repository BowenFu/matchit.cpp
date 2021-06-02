#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
#include "include/utility.h"
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
