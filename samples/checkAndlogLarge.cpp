#include <iostream>
#include "matchit.h"

using namespace matchit;

constexpr bool checkAndlogLarge(double value)
{
    auto const square = [](auto &&v)
    { return v * v; };
    Id<double> s;
    return match(value)(
        pattern(app(square, and_(_ > 1000, s))) = [&]
        {
            std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
            return true;
        },
        pattern(_) = expr(false));
}

// comment out std::cout then uncomment this.
// static_assert(checkAndlogLarge(100));

int main()
{
    std::cout << checkAndlogLarge(100) << std::endl;
    return 0;
}
