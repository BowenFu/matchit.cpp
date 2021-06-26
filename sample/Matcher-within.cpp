#include "matchit.h"
#include <iostream>

constexpr auto within = [](auto const &first, auto const &last)
{
  // the following one has memory issue since it creates temporary unarys and
  // capture them via reference when constructing new unarys. return first <= _
  // && _ <= last;
  return matchit::meet([&](auto &&v)
                       { return first <= v && v <= last; });
};

int main()
{
  using namespace matchit;
  auto n = 5;
  match(n)(
      // clang-format off
        pattern | within(1, 10) = [&] { std::cout << n << " is in [1, 10]."; }, // 1..10
        pattern | _             = [&] { std::cout << n << " is not in [1, 10]."; }
      // clang-format on
  );
  std::cout << std::endl;
  return 0;
}
