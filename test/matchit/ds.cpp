#include <gtest/gtest.h>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

TEST(Ds, matchTuple)
{
  Context context;
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds("123", 123), 0, context));
  EXPECT_FALSE(matchPattern(std::make_tuple("123", 123), ds("123", 12), 0, context));
}

TEST(Ds, matchVec)
{
  Context context;
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, 456), 0, context));
  EXPECT_FALSE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, 456, 123), 0, context));
}

TEST(Ds, tupleOoo)
{
  Context context;
  EXPECT_TRUE(matchPattern(std::tuple<>{}, ds(ooo), 0, context));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds(ooo), 0, context));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds(ooo, 123), 0, context));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds("123", ooo), 0, context));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds("123", ooo, 123), 0, context));
}

TEST(Ds, vecOoo)
{
  Context context;
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(ooo), 0, context));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{}, ds(ooo), 0, context));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, ooo), 0, context));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(ooo, 456), 0, context));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, ooo, 456), 0, context));
}

TEST(Ds, vecOooBinder1)
{
  auto const vec = std::vector<int32_t>{123, 456};
  Id<Span<int32_t>> span;
  Context context;
  EXPECT_TRUE(matchPattern(vec, ds(ooo(span)), 0, context));
  EXPECT_EQ(span.value().mSize, 2);
  EXPECT_EQ(span.value().mData[0], 123);
  EXPECT_EQ(span.value().mData[1], 456);
}

TEST(Ds, vecOooBinder2)
{
  // Note: id only available inside match scope.
  Id<Span<int32_t>> span2;
  match(std::vector<int32_t>{123, 456})(
      pattern(ds(ooo(span2))) = [&] {
        EXPECT_EQ(span2.value().mSize, 2);
        EXPECT_EQ(span2.value().mData[0], 123);
        EXPECT_EQ(span2.value().mData[1], 456);
      });
}
