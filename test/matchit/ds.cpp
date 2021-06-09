#include <gtest/gtest.h>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

TEST(Ds, matchTuple)
{
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds("123", 123)));
  EXPECT_FALSE(matchPattern(std::make_tuple("123", 123), ds("123", 12)));
}

TEST(Ds, matchVec)
{
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, 456)));
  EXPECT_FALSE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, 456, 123)));
}

TEST(Ds, tupleOoo)
{
  EXPECT_TRUE(matchPattern(std::tuple<>{}, ds(ooo)));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds(ooo)));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds(ooo, 123)));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds("123", ooo)));
  EXPECT_TRUE(matchPattern(std::make_tuple("123", 123), ds("123", ooo, 123)));
}

TEST(Ds, vecOoo)
{
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(ooo)));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{}, ds(ooo)));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, ooo)));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(ooo, 456)));
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(123, ooo, 456)));
}

TEST(Ds, vecOooBinder)
{
  auto const vec = std::vector<int32_t>{123, 456};
  Id<Span<int32_t>> span;
  EXPECT_TRUE(matchPattern(vec, ds(ooo(span))));
  EXPECT_EQ(span.value().mSize, 2);
  EXPECT_EQ(span.value().mData[0], 123);
  EXPECT_EQ(span.value().mData[1], 456);

  #if 0
  // FIXME, need to handle span for rvalue, store an vector instead.
  Id<Span<int32_t>> span2;
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{123, 456}, ds(ooo(span2))));
  // This line overwrite the temp memory released above.
  EXPECT_TRUE(matchPattern(std::vector<int32_t>{789, 321}, ds(ooo)));
  EXPECT_EQ(span2.value().mSize, 2);
  EXPECT_EQ(span2.value().mData[0], 123);
  EXPECT_EQ(span2.value().mData[1], 456);
  #endif
}
