#define NO_SCALAR_REFERENCES_USED_IN_PATTERNS 1

#include "matchit.h"
#include <iostream>
#include <optional>

template <typename T>
constexpr auto square(std::optional<T> const &t)
{
  using namespace matchit;
  Id<T> id;
  return match(t)(
      // clang-format off
        pattern | some(id) = id * id,
        pattern | none     = 0
      // clang-format on
  );
}
constexpr auto x = std::make_optional(5);
static_assert(square(x) == 25);

int32_t main()
{
  auto t = std::make_optional(3);
  std::cout << square(t) << std::endl;
  return 0;
}
