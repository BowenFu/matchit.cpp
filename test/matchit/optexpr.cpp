#include <iostream>
#include <gtest/gtest.h>

#include "matchit.h"
using namespace matchit;

template<typename T1, typename T2>
constexpr auto eval1(std::tuple<char, T1, T2> const& exp)
{
    using namespace matchit;
    Id<T1> i;
    Id<T2> j;
    return match(exp)   // no expr()
    (
        pattern | ds('+', i, j) | when((i + j) > 0) = i + j,
        pattern | ds('-', i, j) | when(true) = i - j,
        pattern | ds('*', i, j) | when(i) = i,
        pattern | ds('/', i, j) = 12345,
        pattern | _ = [] { return -1; }
    );
}

template<typename T1, typename T2>
constexpr auto eval2(std::tuple<char, T1, T2> const& exp)
{
    using namespace matchit;
    Id<T1> i;
    Id<T2> j;
    return match(exp)   // unnecessary expr()
    (
        pattern | ds('+', i, j) | when((i + j) > 0) = i + j,
        pattern | ds('-', i, j) | when(true) = i - j,
        pattern | ds('*', i, j) | when(i) = i,
        pattern | ds('/', i, j) = 12345,
        pattern | _ = [] { return -1; }
    );
}

TEST(OptExpr, no_expr)
{
    EXPECT_EQ(eval1(std::tuple{'+', 20, 3}), 23);
    EXPECT_EQ(eval1(std::tuple{'-', 20, 3}), 17);
    EXPECT_EQ(eval1(std::tuple{'*', 20, 3}), 20);
    EXPECT_EQ(eval1(std::tuple{'/', 20, 3}), 12345);
    EXPECT_EQ(eval1(std::tuple{' ', 20, 3}), -1);
}

TEST(OptExpr, unnecessary_expr)
{
    EXPECT_EQ(eval2(std::tuple{'+', 20, 3}), 23);
    EXPECT_EQ(eval2(std::tuple{'-', 20, 3}), 17);
    EXPECT_EQ(eval2(std::tuple{'*', 20, 3}), 20);
    EXPECT_EQ(eval2(std::tuple{'/', 20, 3}), 12345);
    EXPECT_EQ(eval2(std::tuple{' ', 20, 3}), -1);
}
