#include <gtest/gtest.h>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

TEST(Id, matchValue)
{
  Id<int32_t> x;
  x.matchValue(1);
  EXPECT_EQ(*x, 1);
}

TEST(Id, resetId)
{
  Id<int32_t> x;
  x.matchValue(1);
  EXPECT_EQ(*x, 1);
  x.reset(1);
  EXPECT_EQ(*x, 1);
  x.reset(0);
  EXPECT_FALSE(x.hasValue());
}

TEST(Id, resetAfterFailure)
{
  Id<int32_t> x;
  match(10)(
      pattern(x) = [&] {
        EXPECT_EQ(*x, 10);
      });
  auto const matched = match(10)(
      pattern(not_(x)) = expr(true),
      pattern(_) = expr(false));
  EXPECT_FALSE(matched);
}

TEST(Id, resetAfterFailure2)
{
  Id<int32_t> x;
  match(10)(
      pattern(x) = [&] {
        EXPECT_EQ(*x, 10);
      });
  auto const matched = match(10)(
      pattern(and_(x, not_(x))) = expr(true),
      pattern(_) = expr(false));
  EXPECT_FALSE(matched);
}

TEST(Id, resetAfterFailure3)
{
  Id<int32_t> x;
  auto result = match(10)(
      pattern(and_(x, app(_ / 2, x))) = expr(true),
      pattern(_) = expr(false));
  EXPECT_FALSE(result);
  result = match(10)(
      pattern(and_(x, app(_ / 2, not_(x)))) = [&]
      {
        EXPECT_EQ(*x, 10);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(result);
}

TEST(Id, resetAfterFailure33)
{
  Id<int32_t> x;
  auto result = match(10)(
      pattern(or_(and_(not_(x), not_(x)), app(_ / 2, x))) = [&]
      {
        EXPECT_EQ(*x, 5);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(result);

  result = match(10)(
      pattern(or_(and_(x, not_(x)), app(_ / 2, x))) = [&]
      {
        EXPECT_EQ(*x, 5);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(result);

  result = match(10)(
      pattern(or_(and_(not_(x), x), app(_ / 2, x))) = [&]
      {
        EXPECT_EQ(*x, 5);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(result);
}

TEST(Id, resetAfterFailure4)
{
  Id<int32_t> x;
  auto const matched =
      match(std::make_tuple(10, 20))(
          pattern(
              or_(
                  // first / 5 == second / 2 + 1
                  ds(
                      app(_ / 5, x),
                      app(_ / 2 + 1, x)),
                  // first / 2 == second / 5 + 1
                  ds(
                      app(_ / 2, x),
                      app(_ / 5 + 1, x)))) = [&] {
            EXPECT_EQ(*x, 5);
            return true;
          },
          pattern(_) = expr(false));
  EXPECT_TRUE(matched);
}

TEST(Id, resetAfterFailure5)
{
  Id<int32_t> x;
  auto result = match(10)(
      pattern(and_(and_(or_(x)), and_(10))) = [&]
      {
        EXPECT_EQ(*x, 10);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(result);

  result = match(10)(
      pattern(and_(and_(or_(x)), and_(1))) = [&]
      {
        EXPECT_EQ(*x, 10);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_FALSE(result);
}

TEST(Id, matchMultipleTimes1)
{
  Id<int32_t> z;
  match(10)(
      pattern(and_(z, z)) =
          [&] {
            EXPECT_EQ(*z, 10);
          });
}

TEST(Id, matchMultipleTimes2)
{
  Id<std::unique_ptr<int32_t>> x;
  auto result = match(std::make_unique<int32_t>(10))(
      pattern(and_(x)) = [&] { return **x; });
  EXPECT_EQ(result, 10);
}

TEST(Id, matchMultipleTimes3)
{
  Id<std::unique_ptr<int32_t>> x1;
  Id<std::unique_ptr<int32_t>> x2;
  auto result = match(std::make_unique<int32_t>(10))(
      pattern(and_(x1, x2)) = [&] { return **x2; });
  EXPECT_EQ(result, 10);
}

TEST(Id, AppToId)
{
  Id<int32_t> ii;
  auto const result = match(11)
  (
    pattern(app(_ * _, ii)) = expr(ii)
  );
  EXPECT_EQ(result, 121);
}

TEST(Id, AppToId2)
{
  Id<std::unique_ptr<int32_t>> ii;
  auto const result = match(11)(
      pattern(app(
          [](auto &&x) { return std::make_unique<int32_t>(x); }, ii)) = [&] { return **ii; });
  EXPECT_EQ(result, 11);
}

// TODO: fix *Id
TEST(Id, AppToId3)
{
  Id<std::unique_ptr<int32_t>> ii;
  auto const result = match(11)(
      pattern(app(
          [](auto &&x) { return std::make_unique<int32_t>(x); }, ii)) = [&] { return ii.move(); });
  EXPECT_EQ(*result, 11);
}