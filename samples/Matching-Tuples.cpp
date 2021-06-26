#include "matchit.h"
#include <iostream>
#include <tuple>

int32_t main()
{
  using namespace matchit;
  constexpr auto p = std::make_tuple(4, 6);
  Id<int32_t> x, y;
  match(p)(
      // clang-format off
        pattern | ds(0, 0) = [&]{ std::cout << "on origin"; },
        pattern | ds(0, y) = [&]{ std::cout << "on y-axis"; },
        pattern | ds(x, 0) = [&]{ std::cout << "on x-axis"; },
        pattern | ds(x, y) = [&]{ std::cout << *x << ',' << *y; }
      // clang-format on
  );

  return 0;
}
