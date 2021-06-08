#include <gtest/gtest.h>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

TEST(Id, matchValue)
{
  Id<int32_t> x;
  x.matchValue(1, 0);
  EXPECT_EQ(*x, 1);
}

TEST(Id, resetId)
{
  Id<int32_t> x;
  x.matchValue(1, 0);
  EXPECT_EQ(*x, 1);
  x.reset(1);
  EXPECT_EQ(*x, 1);
  x.reset(0);
  EXPECT_FALSE(x.hasValue());
}

TEST(Id, resetAfterFailure)
{
  Id<int32_t> x;
  matchPattern(10, x);
  EXPECT_EQ(*x, 10);
  matchPattern(10, not_(x));
  EXPECT_FALSE(x.hasValue());
}

TEST(Id, resetAfterFailure2)
{
  Id<int32_t> x;
  matchPattern(10, and_(x, not_(x)));
  EXPECT_FALSE(x.hasValue());
}

TEST(Id, resetAfterFailure3)
{
  Id<int32_t> x;
  matchPattern(10, and_(x, app(_ / 2, x)));
  EXPECT_FALSE(x.hasValue());
  matchPattern(10, and_(x, app(_ / 2, not_(x))));
  EXPECT_EQ(*x, 10);
  matchPattern(10, or_(and_(not_(x), not_(x)), app(_ / 2, x)));
  EXPECT_EQ(*x, 5);
  matchPattern(10, or_(and_(not_(x), x), app(_ / 2, x)));
  EXPECT_EQ(*x, 5);
  matchPattern(10, or_(and_(x, not_(x)), app(_ / 2, x)));
  EXPECT_EQ(*x, 5);
}

TEST(Id, resetAfterFailure4)
{
  Id<int32_t> x;
  auto const matched =
      matchPattern(std::make_tuple(10, 20),
                   or_(
                       // first / 5 == second / 2 + 1
                       ds(
                           app(_ / 5, x),
                           app(_ / 2 + 1, x)),
                       // first / 2 == second / 5 + 1
                       ds(
                           app(_ / 2, x),
                           app(_ / 5 + 1, x))));
  EXPECT_TRUE(matched);
  EXPECT_EQ(*x, 5);
}