#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

constexpr bool checkAndlogLarge(double value)
{
    auto const square = [](auto &&v) { return v * v; };
    Id<double> s;
    return match(value)(
        pattern(app(square, and_(_ > 1000, s))) = [&] {
                std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
                return true; },
        pattern(_) = [&] { return false; });
}

// comment out std::cout then uncomment this.
// static_assert(checkAndlogLarge(100));

int main()
{
    printf("%d\n", checkAndlogLarge(100));
    return 0;
}
