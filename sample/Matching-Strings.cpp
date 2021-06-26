#include "matchit.h"
#include <iostream>

int32_t main()
{
  using namespace matchit;
  constexpr auto s = "bar";
  match(s)(
      // clang-format off
        pattern | "foo" = [&] { std::cout << "got foo"; },
        pattern | "bar" = [&] { std::cout << "got bar"; },
        pattern | _     = [&] { std::cout << "don't care"; }
      // clang-format on
  );
  return 0;
}
