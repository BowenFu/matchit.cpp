#include <iostream>
#include "matchit.h"

int32_t main()
{
    for (auto i = -2; i <= 5; ++i)
    {
        using namespace matchit;
        std::cout << match(i)(
                         // clang-format off
                         pattern | -1        = expr("It's minus one"),
                         pattern | 1         = expr("It's a one"),
                         pattern | or_(2, 4) = expr("It's either a two or a four"),
                         pattern | _         = expr("Matched none of the arms")
                         // clang-format off
                         )
                  << std::endl;
    }
    return 0;
}