#include <iostream>
#include "matchit.h"

constexpr bool checkAndlogLarge(double value)
{
    using namespace matchit;

    auto const square = [](auto &&v)
    { return v * v; };
    Id<double> s;
    return match(value)(
        pattern | app(square, and_(_ > 1000, s)) = [&]
        {
            std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
            return true;
        },
        pattern | _ = expr(false));
}

// comment out std::cout then uncomment this.
// static_assert(checkAndlogLarge(100));

int32_t main()
{
    std::cout << checkAndlogLarge(100) << std::endl;
    return 0;
}
