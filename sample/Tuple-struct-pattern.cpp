#include "matchit.h"
#include <cassert>
#include <iostream>
using namespace matchit;

void sample()
{
  constexpr auto pair = std::make_pair(10, "ten");
  Id<int32_t> a;
  Id<char const *> b;
  match(pair)(pattern | ds(a, b) = [&]
              {
                assert(*a == 10);
                assert(*b == std::string_view{"ten"});
              });
}

int32_t main()
{
  sample();
  return 0;
}