#include <gtest/gtest.h>
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

TEST(Expr, nullary)
{
  EXPECT_EQ(expr(5)(), 5);
  EXPECT_EQ((expr(5) + 5)(), 10);
}

TEST(Expr, Unary)
{
  EXPECT_EQ((_ * 2)(5), 10);
}
