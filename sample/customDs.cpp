#include "matchit.h"
#include <iostream>

struct DummyStruct
{
  int32_t size;
  char const *name;
};
constexpr bool operator==(DummyStruct const &lhs, DummyStruct const &rhs)
{
  return lhs.size == rhs.size && lhs.name == rhs.name;
}

template <size_t I>
constexpr auto const &get(DummyStruct const &d)
{
  if constexpr (I == 0)
  {
    return d.size;
  }
  else if constexpr (I == 1)
  {
    return d.name;
  }
}

namespace std
{
  template <>
  class tuple_size<DummyStruct> : public std::integral_constant<size_t, 2>
  {
  };
} // namespace std

constexpr auto getSecond(DummyStruct const &d)
{
  using namespace matchit;
  Id<char const *> i;
  return match(d)(
      // clang-format off
        pattern | ds(2, i) = i,
        pattern | _        = "not matched"
      // clang-format on
  );
}

// #if __cplusplus > 201703L
static_assert(getSecond(DummyStruct{1, "123"}) ==
              std::string_view{"not matched"});
static_assert(getSecond(DummyStruct{2, "123"}) == std::string_view{"123"});
// #endif

// Another option to destructure your struct / class.
constexpr auto dsByMember(DummyStruct const &v)
{
  using namespace matchit;
  // compose patterns for destructuring struct DummyStruct.
  constexpr auto dsA = dsVia(&DummyStruct::size, &DummyStruct::name);
  Id<char const *> i;
  return match(v)(
      // clang-format off
        pattern | dsA(2, i) = i,
        pattern | _         = "not matched"
      // clang-format on
  );
}

static_assert(dsByMember(DummyStruct{1, "123"}) ==
              std::string_view{"not matched"});
static_assert(dsByMember(DummyStruct{2, "123"}) == std::string_view{"123"});

int32_t main()
{
  std::cout << getSecond(DummyStruct{1, "123"}) << std::endl;
  std::cout << dsByMember(DummyStruct{1, "123"}) << std::endl;
  std::cout << getSecond(DummyStruct{2, "123"}) << std::endl;
  std::cout << dsByMember(DummyStruct{2, "123"}) << std::endl;
  return 0;
}
