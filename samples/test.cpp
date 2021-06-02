#include <array>
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

template <typename Func>
auto optionalLift(Func func)
{
    return [func](auto &&v) {
        Id<std::decay_t<decltype(v)> > x;
        return match(v)(
            pattern(some(x)) = [func] { return std::make_optional(func(*x)); },
            pattern(none) = [] { return {}; });
    };
}

int main()
{
    printf("%d\n", optionalLift([](auto&& e){return e*e;})(std::make_optional(2)));
    return 0;
}
