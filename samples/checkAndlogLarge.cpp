#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

bool checkAndlogLarge(double value)
{
    auto const square = [](auto &&v) { return v * v; };
    Id<double> s;
    return match(value)(
        pattern(app(square, and_(_ > 1000, s))) = [&] {
                std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
                return true; },
        pattern(_) = [&] { return false; });
}

int main()
{
    printf("%d\n", checkAndlogLarge(100));
    return 0;
}
