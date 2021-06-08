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
