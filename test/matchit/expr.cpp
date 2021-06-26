#include "matchit.h"
#include <gtest/gtest.h>
using namespace matchit;

TEST(Expr, nullary)
{
  EXPECT_EQ(expr(5)(), 5);
  EXPECT_EQ((!expr(false))(), true);
  EXPECT_EQ((expr(5) + 5)(), 10);
  EXPECT_EQ((expr(5) % 5)(), 0);
  EXPECT_EQ((expr(5) < 5)(), false);
  EXPECT_EQ((expr(5) <= 5)(), true);
  EXPECT_EQ((expr(5) != 5)(), false);
  EXPECT_EQ((expr(5) >= 5)(), true);
  EXPECT_EQ((expr(false) && true)(), false);
  EXPECT_EQ((expr(false) || true)(), true);
}

TEST(Expr, Unary)
{
  EXPECT_EQ((!_)(true), false);
  EXPECT_EQ((-_)(1), -1);
  EXPECT_EQ((1 - _)(1), 0);
  EXPECT_EQ((_ % 3)(5), 2);
  EXPECT_EQ((_ * 2)(5), 10);
  EXPECT_EQ((_ == 2)(5), false);
  EXPECT_EQ((_ != 2)(5), true);
  EXPECT_EQ((_ || false)(true), true);
  EXPECT_EQ((_ && false)(true), false);
}
