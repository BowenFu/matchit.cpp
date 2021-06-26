#include "matchit.h"
#include <iostream>
#include <optional>
#include <string_view>

template <typename T>
struct Is
{
  template <typename Arg>
  Arg &&operator()(Arg &&arg) const
  {
    static_assert(std::is_same_v<T, std::decay_t<Arg>>);
    return std::forward<Arg>(arg);
  }
};

template <typename T>
inline constexpr Is<T> is;

auto sample1()
{
  const auto x = std::make_tuple(std::string("str"), 123);
  using namespace matchit;
  Id<std::string> s;
  Id<int> i;
  match(x)(pattern | ds(app(is<std::string>, s), app(is<int>, i)) = [&]
           { std::cout << "first " << *s << " second " << *i; });
  std::cout << std::endl;
}

struct Email
{
  constexpr std::optional<std::array<std::string_view, 2>>
  operator()(std::string_view sv) const
  {
    auto const d = sv.find("@");
    if (d == std::string_view::npos)
    {
      return {};
    }
    return std::array<std::string_view, 2>{sv.substr(0, d), sv.substr(d + 1)};
  }
};

inline constexpr Email email;

struct PhoneNumber
{
  std::optional<std::array<std::string_view, 3>>
  operator()(std::string_view sv) const
  {
    return std::array<std::string_view, 3>{sv.substr(0, 3), sv.substr(3, 3),
                                           sv.substr(6)};
  }
};

inline constexpr PhoneNumber phone_number;

void sample2()
{
  using namespace matchit;

  using namespace std::literals;
  // auto const s = "match@it"sv;
  auto const s = "415123456"sv;
  Id<std::string_view> address, domain;
  match(s)(
      pattern | app(email, some(ds(address, domain))) =
          [&]
      { std::cout << "got an email"; },
      pattern | app(phone_number, some(ds("415", _, _))) =
          [&]
      { std::cout << "got a San Francisco phone number"; }
      //  ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ extractor pattern
  );
  std::cout << std::endl;
}

int32_t main()
{
  sample1();
  sample2();

  return 0;
}
