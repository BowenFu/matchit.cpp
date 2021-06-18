#include <gtest/gtest.h>
#include "matchit.h"



using namespace matchit;

TEST(Ds, matchTuple)
{
    EXPECT_TRUE(matched(std::make_tuple("123", 123), ds("123", 123)));
    EXPECT_FALSE(matched(std::make_tuple("123", 123), ds("123", 12)));
}

TEST(Ds, matchVec)
{
    EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(123, 456)));
    EXPECT_FALSE(matched(std::vector<int32_t>{123, 456}, ds(123, 456, 123)));
}

TEST(Ds, tupleOoo)
{
  EXPECT_TRUE(matched(std::tuple<>{}, ds(ooo)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds(ooo)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds(ooo, 123)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds("123", ooo)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds("123", ooo, 123)));
}

TEST(Ds, vecOoo)
{
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(ooo)));
  EXPECT_TRUE(matched(std::vector<int32_t>{}, ds(ooo)));
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(123, ooo)));
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(ooo, 456)));
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(123, ooo, 456)));
}

TEST(Ds, vecOooBinder1)
{
  auto const vec = std::vector<int32_t>{123, 456};
  Id<Span<int32_t>> span;
  auto matched = match(vec)(
      pattern(ds(ooo(span))) = [&] {
          EXPECT_EQ((*span).size(), 2);
          EXPECT_EQ((*span)[0], 123);
          EXPECT_EQ((*span)[1], 456);
          return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(matched);
}

TEST(Ds, vecOooBinder2)
{
  Id<Span<int32_t>> span;
  match(std::vector<int32_t>{123, 456})(
      pattern(ds(ooo(span))) = [&] {
        EXPECT_EQ((*span).size(), 2);
        EXPECT_EQ((*span)[0], 123);
        EXPECT_EQ((*span)[1], 456);
      });
}

TEST(Ds, vecOooBinder3)
{
  Id<Span<int32_t>> span;
  match(std::vector<int32_t>{123, 456})(
      pattern(ds(123, ooo(span), 456)) = [&] {
        EXPECT_EQ((*span).size(), 0);
      });
}

TEST(Ds, vecOooBinder4)
{
  Id<Span<int32_t>> span;
  match(std::vector<int32_t>{123, 456, 789})(
      pattern(ds(123, ooo(span))) = [&] {
        EXPECT_EQ((*span).size(), 2);
        EXPECT_EQ((*span)[0], 456);
        EXPECT_EQ((*span)[1], 789);
      });
}

TEST(Ds, FailDueToTwoFewValues)
{
  EXPECT_FALSE(matched(std::vector<int32_t>{123, 456, 789}, ds(123, ooo, 456, 456, 789)));
}

TEST(Ds, arrayOooBinder1)
{
  auto const array = std::array<int32_t, 2>{123, 456};
  Id<Span<int32_t>> span;
  auto matched = match(array)(
      pattern(ds(ooo(span))) = [&] {
          EXPECT_EQ((*span).size(), 2);
          EXPECT_EQ((*span)[0], 123);
          EXPECT_EQ((*span)[1], 456);
          return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(matched);
}

TEST(Ds, arrayOooBinder2)
{
  Id<Span<int32_t>> span;
  match(std::array<int32_t, 2>{123, 456})(
      pattern(ds(ooo(span))) = [&] {
        EXPECT_EQ((*span).size(), 2);
        EXPECT_EQ((*span)[0], 123);
        EXPECT_EQ((*span)[1], 456);
      });
}

TEST(Ds, arrayOooBinder3)
{
  Id<Span<int32_t>> span;
  match(std::array<int32_t, 2>{123, 456})(
      pattern(ds(123, ooo(span), 456)) = [&] {
        EXPECT_EQ((*span).size(), 0);
      });
}

TEST(Ds, arrayOooBinder4)
{
  Id<Span<int32_t>> span;
  match(std::array<int32_t, 3>{123, 456, 789})(
      pattern(ds(123, ooo(span))) = [&] {
        EXPECT_EQ((*span).size(), 2);
        EXPECT_EQ((*span)[0], 456);
        EXPECT_EQ((*span)[1], 789);
      });
}

// rotate
TEST(Ds, arrayOooBinder5)
{
  Id<Span<int32_t>> span;
  Id<int32_t> e;
  match(std::array<int32_t, 3>{123, 456, 789}, std::array<int32_t, 3>{456, 789, 123})(
      // move head to end
      pattern(ds(e, ooo(span)), ds(ooo(span), e)) = [&]
      {
        EXPECT_EQ(*e, 123);
        EXPECT_EQ((*span).size(), 2);
        EXPECT_EQ((*span)[0], 456);
        EXPECT_EQ((*span)[1], 789);
      });
}
