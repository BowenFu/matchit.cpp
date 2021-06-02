#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
#include "include/utility.h"
using namespace matchit;

// lift a function from T -> U to std::optional<T> -> std::optional<U>
template <typename Func>
auto optionalLift(Func func)
{
    return [func](auto &&v) {
        Id<std::decay_t<decltype(*v)> > x;
        using RetType = decltype(std::make_optional(func(*x)));
        return match(v)(
            pattern(some(x)) = [func, &x] { return std::make_optional(func(*x)); },
            pattern(none) = [] { return RetType{}; });
    };
}

int main()
{
    auto const func = [](auto &&e) { return e * e; };
    auto const result = optionalLift(func)(std::make_optional(2)); 
    std::cout << *result << std::endl;
    return 0;
}
