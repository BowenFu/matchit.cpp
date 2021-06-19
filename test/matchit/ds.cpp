#include <gtest/gtest.h>
#include "matchit.h"
#include <list>

using namespace matchit;

TEST(Ds, matchTuple)
{
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds("123", 123)));
  EXPECT_FALSE(matched(std::make_tuple("123", 123), ds("123", 12)));
}

TEST(Ds, matchArray)
{
  EXPECT_TRUE(matched(std::array<int32_t, 2>{123, 456}, ds(123, 456)));
  EXPECT_FALSE(matched(std::array<int32_t, 2>{123, 456}, ds(456, 123)));
}

TEST(Ds, matchVec)
{
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(123, 456)));
  EXPECT_FALSE(matched(std::vector<int32_t>{123, 456}, ds(123, 456, 123)));
}

TEST(Ds, matchList)
{
  EXPECT_TRUE(matched(std::list<int32_t>{123, 456}, ds(123, 456)));
  EXPECT_FALSE(matched(std::list<int32_t>{123, 456}, ds(123, 456, 123)));
}

TEST(Ds, tupleOoo)
{
  EXPECT_TRUE(matched(std::tuple<>{}, ds(ooo)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds(ooo)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds(ooo, 123)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds("123", ooo)));
  EXPECT_TRUE(matched(std::make_tuple("123", 123), ds("123", ooo, 123)));
}

TEST(Ds, arrayOoo)
{
  EXPECT_TRUE(matched(std::array<int32_t, 2>{123, 456}, ds(ooo)));
  EXPECT_TRUE(matched(std::array<int32_t, 0>{}, ds(ooo)));
  EXPECT_TRUE(matched(std::array<int32_t, 2>{123, 456}, ds(123, ooo)));
  EXPECT_TRUE(matched(std::array<int32_t, 2>{123, 456}, ds(ooo, 456)));
  EXPECT_TRUE(matched(std::array<int32_t, 2>{123, 456}, ds(123, ooo, 456)));
}

TEST(Ds, vecOoo)
{
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(ooo)));
  EXPECT_TRUE(matched(std::vector<int32_t>{}, ds(ooo)));
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(123, ooo)));
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(ooo, 456)));
  EXPECT_TRUE(matched(std::vector<int32_t>{123, 456}, ds(123, ooo, 456)));
}

template <typename T>
class Debug;

TEST(Ds, listOoo)
{
  EXPECT_TRUE(matched(std::list<int32_t>{123, 456}, ds(ooo)));
  EXPECT_TRUE(matched(std::list<int32_t>{}, ds(ooo)));
  EXPECT_TRUE(matched(std::list<int32_t>{123, 456}, ds(123, ooo)));
  EXPECT_TRUE(matched(std::list<int32_t>{123, 456}, ds(ooo, 456)));
  EXPECT_TRUE(matched(std::list<int32_t>{123, 456}, ds(123, ooo, 456)));
}

template <typename Range1, typename Range2>
auto expectRange(Range1 const &result, Range2 const &expected)
{
  EXPECT_EQ(result.size(), expected.size());
  auto b1 = result.begin();
  auto b2 = expected.begin();
  for (; b1 != result.end() && b2 != expected.end(); ++b1, ++b2)
  {
    EXPECT_EQ(*b1, *b2);
  }
}

TEST(Ds, vecOooBinder1)
{
  auto const vec = std::vector<int32_t>{123, 456};
  Id<SubrangeT<decltype(vec)>> subrange;
  auto matched = match(vec)(
      pattern(ooo(subrange)) = [&]
      {
        auto const expected = {123, 456};
        expectRange(*subrange, expected);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(matched);
}

TEST(Ds, vecOooBinder2)
{
  Id<SubrangeT<std::vector<int32_t>>> subrange;
  match(std::vector<int32_t>{123, 456})(
      pattern(ooo(subrange)) = [&]
      {
        auto const expected = {123, 456};
        expectRange(*subrange, expected);
      },
      pattern(_) = []
      {
        // make sure the above pattern is matched, otherwise the test case fails.
        ADD_FAILURE();
      });
}

TEST(Ds, vecOooBinder3)
{
  Id<SubrangeT<std::vector<int32_t>>> subrange;
  match(std::vector<int32_t>{123, 456})(
      pattern(123, ooo(subrange), 456) = [&]
      { EXPECT_EQ((*subrange).size(), 0); });
}

TEST(Ds, vecOooBinder4)
{
  Id<SubrangeT<std::vector<int32_t>>> subrange;
  match(std::vector<int32_t>{123, 456, 789})(
      pattern(123, ooo(subrange)) = [&]
      {
        auto const expected = {456, 789};
        expectRange(*subrange, expected);
      });
}

TEST(Ds, FailDueToTwoFewValues)
{
  EXPECT_FALSE(matched(std::vector<int32_t>{123, 456, 789}, ds(123, ooo, 456, 456, 789)));
}

TEST(Ds, listOooBinder4)
{
  Id<SubrangeT<std::list<int32_t>>> subrange;
  match(std::list<int32_t>{123, 456, 789})(
      pattern(123, ooo(subrange)) = [&]
      {
        auto const expected = {456, 789};
        expectRange(*subrange, expected);
      });
}

TEST(Ds, arrayOooBinder1)
{
  auto const array = std::array<int32_t, 2>{123, 456};
  Id<SubrangeT<decltype(array)>> subrange;
  auto matched = match(array)(
      pattern(ooo(subrange)) = [&]
      {
        auto const expected = {123, 456};
        expectRange(*subrange, expected);
        return true;
      },
      pattern(_) = expr(false));
  EXPECT_TRUE(matched);
}

TEST(Ds, arrayOooBinder2)
{
  Id<SubrangeT<std::array<int32_t, 2>>> subrange;
  match(std::array<int32_t, 2>{123, 456})(
      pattern(ooo(subrange)) = [&]
      {
        auto const expected = {123, 456};
        expectRange(*subrange, expected);
      });
}

TEST(Ds, arrayOooBinder3)
{
  Id<SubrangeT<std::array<int32_t, 2>>> subrange;
  match(std::array<int32_t, 2>{123, 456})(
      pattern(123, ooo(subrange), 456) = [&]
      { EXPECT_EQ((*subrange).size(), 0); });
}

TEST(Ds, arrayOooBinder4)
{
  Id<SubrangeT<std::array<int32_t, 2>>> subrange;
  match(std::forward_as_tuple(std::array<int32_t, 3>{123, 456, 789}))(
      pattern(ds(ds(123, ooo(subrange)))) = [&]
      {
        auto const expected = {456, 789};
        expectRange(*subrange, expected);
      });
}

// rotate
TEST(Ds, arrayOooBinder5)
{
  Id<SubrangeT<std::array<int32_t, 2>>> subrange;
  Id<int32_t> e;
  match(std::make_tuple(std::array<int32_t, 3>{123, 456, 789}, std::array<int32_t, 3>{456, 789, 123}))(
      // move head to end
      pattern(ds(e, ooo(subrange)), ds(ooo(subrange), e)) = [&]
      {
        EXPECT_EQ(*e, 123);
        auto const expected = {456, 789};
        expectRange(*subrange, expected);
      });
}

// rotate
TEST(Ds, arrayOooBinder6)
{
  Id<SubrangeT<std::array<int32_t, 2>>> subrange;
  Id<int32_t> e;
  match(std::make_tuple(std::array<int32_t, 3>{123, 456, 789}))(
      // move head to end
      pattern(ds(ds(e, ooo(subrange)))) = [&]
      {
        EXPECT_EQ(*e, 123);
        auto const expected = {456, 789};
        expectRange(*subrange, expected);
      });
}

template <typename Range>
constexpr bool recursiveSymmetric(Range const &range)
{
    Id<int32_t> i;
    Id<SubrangeT<Range const>> subrange;
    return match(range)(
        // clang-format off
        pattern(i, ooo(subrange), i) = [&] { return recursiveSymmetric(*subrange); },
        pattern(i, ooo(subrange), _) = expr(false),
        pattern(_)                   = expr(true)
        // clang-format on
    );
}

// rotate
TEST(Ds, subrangeOooBinder)
{
  EXPECT_TRUE(recursiveSymmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}));
  EXPECT_FALSE(recursiveSymmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}));

  EXPECT_TRUE(recursiveSymmetric(std::vector<int32_t>{5, 0, 3, 0, 5}));
  EXPECT_FALSE(recursiveSymmetric(std::vector<int32_t>{5, 0, 3, 7, 10}));

  EXPECT_TRUE(recursiveSymmetric(std::list<int32_t>{5, 0, 3, 0, 5}));
  EXPECT_FALSE(recursiveSymmetric(std::list<int32_t>{5, 0, 3, 7, 10}));

  EXPECT_TRUE(recursiveSymmetric(std::initializer_list<int32_t>{5, 0, 3, 0, 5}));
  EXPECT_FALSE(recursiveSymmetric(std::initializer_list<int32_t>{5, 0, 3, 7, 10}));
}
