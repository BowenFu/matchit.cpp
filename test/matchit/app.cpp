#include <gtest/gtest.h>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

class Base
{
public:
  virtual int index() const = 0;
  virtual ~Base() = default;
};

class Derived : public Base
{
public:
  int index() const override
  {
    return 1;
  }
};

constexpr auto deref = [](auto &&x) -> decltype(*x) { return *x; };
static_assert(std::is_same_v<impl::PatternTraits<impl::App<decltype(deref), impl::Wildcard> >::template AppResultTuple<Base*>, std::tuple<> >);
static_assert(std::is_same_v<impl::PatternTraits<impl::App<decltype(deref), impl::Wildcard> >::template AppResult<Base*>, Base&>);

TEST(App, someAs)
{
  auto x = std::unique_ptr<Base>{new Derived};
  auto const result = match(x)(
      pattern(some(as<Derived>(_))) = expr(true));
  EXPECT_EQ(result, true);
}
