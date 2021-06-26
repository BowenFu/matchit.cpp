#include "matchit.h"
#include <iostream>

int32_t main()
{
  using namespace matchit;
  constexpr int32_t x = 5;
  match(x)(
      // clang-format off
        pattern | 0 = [&] { std::cout << "got zero"; },
        pattern | 1 = [&] { std::cout << "got one"; },
        pattern | _ = [&] { std::cout << "don't care"; });
  // clang-format on
  return 0;
}
