#include "matchit.h"
#include <gtest/gtest.h>
using namespace matchit;

class Base
{
public:
  virtual int32_t index() const = 0;
  virtual ~Base() = default;
};

class Derived : public Base
{
public:
  int32_t index() const override { return 1; }
};

constexpr auto deref = [](auto &&x) -> decltype(*x)
{ return *x; };
static_assert(std::is_same_v<
              impl::PatternTraits<impl::App<decltype(deref), impl::Wildcard>>::
                  template AppResultTuple<Base *>,
              std::tuple<>>);
static_assert(std::is_same_v<
              impl::PatternTraits<impl::App<decltype(deref), impl::Wildcard>>::
                  template AppResult<Base *>,
              Base &>);

TEST(App, someAs)
{
  auto const x = std::unique_ptr<Base>{new Derived};
  EXPECT_TRUE(matched(x, some(as<Derived>(_))));
}

TEST(App, scalarPtr)
{
  auto const x = std::make_unique<int>(10);
  Id<int*> xPtr;
  match(x)
  (
    pattern | some(as<int>(asPtr<int>(xPtr))) = [&] { *(*xPtr) = 20 ; }
  );
  EXPECT_EQ(*x, 20);
}
