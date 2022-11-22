#include "matchit.h"
#include <iostream>

int32_t main()
{
  for (auto i = -2; i <= 5; ++i)
  {
    using namespace matchit;
    std::cout << match(i)(
                     // clang-format off
                         pattern | -1        = "It's minus one",
                         pattern | 1         = "It's a one",
                         pattern | or_(2, 4) = "It's either a two or a four",
                         pattern | _         = "Matched none of the arms"
                         // clang-format off
                         )
                  << std::endl;
    }
    return 0;
}