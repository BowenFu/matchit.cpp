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
