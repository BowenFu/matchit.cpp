#include <string>
#include <gtest/gtest.h>
#include <variant>
#include <array>
#include <any>
#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

bool func1()
{
  return true;
}

int64_t func2()
{
  return 12;
}

TEST(Match, test1)
{
  auto const matchFunc = [](int32_t input) {
    Id<int> ii;
    return match(input)(
        pattern(1) = func1,
        pattern(2) = func2,
        pattern(or_(56, 59)) = func2,
        pattern(_ < 0) = expr(-1),
        pattern(_ < 10) = expr(-10),
        pattern(and_(_<17, _> 15)) = expr(16),
        pattern(app(_ * _, _ > 1000)) = expr(1000),
        pattern(app(_ * _, ii)) = expr(ii),
        pattern(ii) = -ii,
        pattern(_) = expr(111));
  };
  EXPECT_EQ(matchFunc(1), true);
  EXPECT_EQ(matchFunc(2), 12);
  EXPECT_EQ(matchFunc(11), 121);   // Id matched.
  EXPECT_EQ(matchFunc(59), 12);    // or_ matched.
  EXPECT_EQ(matchFunc(-5), -1);    // meet matched.
  EXPECT_EQ(matchFunc(10), 100);   // app matched.
  EXPECT_EQ(matchFunc(100), 1000); // app > meet matched.
  EXPECT_EQ(matchFunc(5), -10);    // _ < 10 matched.
  EXPECT_EQ(matchFunc(16), 16);    // and_ matched.
}

TEST(Match, test2)
{
    auto const matchFunc = [](auto &&input) {
        Id<int> i;
        Id<int> j;
        return match(input)(
            pattern(ds('/', 1, 1)) = expr(1),
            pattern(ds('/', 0, _)) = expr(0),
            pattern(ds('*', i, j)) = i * j,
            pattern(ds('+', i, j)) = i + j,
            pattern(_)             = expr(-1));
    };
    ASSERT_EQ(matchFunc(std::make_tuple('/', 1, 1)), 1);
    ASSERT_EQ(matchFunc(std::make_tuple('+', 2, 1)), 3);
    ASSERT_EQ(matchFunc(std::make_tuple('/', 0, 1)), 0);
    ASSERT_EQ(matchFunc(std::make_tuple('*', 2, 1)), 2);
    ASSERT_EQ(matchFunc(std::make_tuple('/', 2, 1)), -1);
    ASSERT_EQ(matchFunc(std::make_tuple('/', 2, 3)), -1);
}

struct A
{
    int a;
    int b;
};
bool operator==(A const lhs, A const rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b;
}

TEST(Match, test3)
{
    auto const matchFunc = [](A const &input) {
        Id<int> i;
        Id<int> j;
        Id<A> a;
        // compose patterns for destructuring struct A.
        auto const dsA = [](Id<int> &x) {
            return and_(app(&A::a, x), app(&A::b, 1));
        };
        return match(input)(
            pattern(dsA(i)) = expr(i),
            pattern(_) = expr(-1));
    };
    EXPECT_EQ(matchFunc(A{3, 1}), 3);
    EXPECT_EQ(matchFunc(A{2, 2}), -1);
}

enum class Kind
{
    kONE,
    kTWO
};

class Num
{
public:
    virtual ~Num() = default;
    virtual Kind kind() const = 0;
};

class One : public Num
{
public:
    constexpr static auto k = Kind::kONE;
    Kind kind() const override
    {
        return k;
    }
};

class Two : public Num
{
public:
    constexpr static auto k = Kind::kTWO;
    Kind kind() const override
    {
        return k;
    }
};

bool operator==(One const &, One const &)
{
    return true;
}

bool operator==(Two const &, Two const &)
{
    return true;
}

template <Kind k>
auto constexpr kind = app(&Num::kind, k);

template <typename T>
class NumAsPointer
{
public:
    auto operator()(Num const &num) const
    {
        return num.kind() == T::k ? static_cast<T const *>(std::addressof(num)) : nullptr;
    }
};

template <>
class matchit::impl::CustomAsPointer<One> : public NumAsPointer<One>
{
};

template <>
class matchit::impl::CustomAsPointer<Two> : public NumAsPointer<Two>
{
};

TEST(Match, test4)
{
    auto const matchFunc = [](Num const &input) {
        return match(input)(
            pattern(as<One>(_)) = expr(1),
            pattern(kind<Kind::kTWO>) = expr(2),
            pattern(_) = expr(3));
    };
    matchit::impl::AsPointer<Two>()(std::variant<One, Two>{});
    EXPECT_EQ(matchFunc(One{}), 1);
    EXPECT_EQ(matchFunc(Two{}), 2);
}

TEST(Match, test5)
{
    auto const matchFunc = [](std::pair<int32_t, int32_t> ij) {
        return match(ij.first % 3, ij.second % 5)(
            pattern(0, 0) = expr(1),
            pattern(0, _ > 2) = expr(2),
            pattern(_, _ > 2) = expr(3),
            pattern(_) = expr(4));
    };
    EXPECT_EQ(matchFunc(std::make_pair(3, 5)), 1);
    EXPECT_EQ(matchFunc(std::make_pair(3, 4)), 2);
    EXPECT_EQ(matchFunc(std::make_pair(4, 4)), 3);
    EXPECT_EQ(matchFunc(std::make_pair(4, 1)), 4);
}

int32_t fib(int32_t n)
{
    EXPECT_TRUE(n > 0);
    return match(n)(
        pattern(1) = expr(1),
        pattern(2) = expr(1),
        pattern(_) = [n] { return fib(n - 1) + fib(n - 2); });
}

TEST(Match, test6)
{
    EXPECT_EQ(fib(1), 1);
    EXPECT_EQ(fib(2), 1);
    EXPECT_EQ(fib(3), 2);
    EXPECT_EQ(fib(4), 3);
    EXPECT_EQ(fib(5), 5);
}

TEST(Match, test7)
{
    auto const matchFunc = [](std::pair<int32_t, int32_t> ij) {
        Id<std::tuple<int32_t const &, int32_t const &> > id;
        // delegate at to and_
        auto const at = [](auto &&id, auto &&pattern) {
            return and_(id, pattern);
        };
        return match(ij.first % 3, ij.second % 5)(
            pattern(0, _ > 2) = expr(2),
            pattern(ds(1, _ > 2)) = expr(3),
            pattern(at(id, ds(_, 2))) = [&id] {EXPECT_TRUE(std::get<1>(*id) == 2); static_cast<void>(id); return 4; },
            pattern(_) = expr(5));
    };
    EXPECT_EQ(matchFunc(std::make_pair(4, 2)), 4);
}

TEST(Match, test8)
{
    auto const equal = [](std::pair<int32_t, std::pair<int32_t, int32_t> > ijk) {
        Id<int32_t> x;
        return match(ijk)(
            pattern(ds(x, ds(_, x))) = expr(true),
            pattern(_) = expr(false));
    };
    EXPECT_TRUE(equal(std::make_pair(2, std::make_pair(1, 2))));
    EXPECT_FALSE(equal(std::make_pair(2, std::make_pair(1, 3))));
}

// optional
TEST(Match, test9)
{
    auto const optional = [](auto const &i) {
        Id<int32_t> x;
        return match(i)(
            pattern(some(x)) = expr(true),
            pattern(none) = expr(false));
    };
    EXPECT_EQ(optional(std::make_unique<int32_t>(2)), true);
    EXPECT_EQ(optional(std::unique_ptr<int32_t>{}), false);
    EXPECT_EQ(optional(std::make_optional<int32_t>(2)), true);
    EXPECT_EQ(optional(std::optional<int32_t>{}), false);
    int32_t *p = nullptr;
    EXPECT_EQ(optional(p), false);
    int a = 3;
    EXPECT_EQ(optional(&a), true);
}

struct Shape
{
    virtual bool is() = 0;
    virtual ~Shape() = default;
};
struct Circle : Shape
{
    bool is()
    {
        return false;
    }
};
struct Square : Shape
{
    bool is()
    {
        return false;
    }
};

bool operator==(Shape const&, Shape const&)
{
    return true;
}

TEST(Match, test10)
{
    static_assert(matchit::impl::CanReset<Shape, Shape&>::value);
    static_assert(matchit::impl::CanReset<Shape const, Shape const&>::value);

    auto const dynCast = [](auto const &i) {
        return match(i)(
            pattern(some(as<Circle>(_))) = expr("Circle"),
            pattern(some(as<Square>(_))) = expr("Square"),
            pattern(none) = expr("None"));
    };

    EXPECT_EQ(dynCast(std::make_unique<Square>()), "Square");
    EXPECT_EQ(dynCast(std::make_unique<Circle>()), "Circle");
    EXPECT_EQ(dynCast(std::unique_ptr<Circle>()), "None");
}

TEST(Match, test11)
{
    auto const getIf = [](auto const &i) {
        return match(i)(
            pattern(as<Square>(_)) = expr("Square"),
            pattern(as<Circle>(_)) = expr("Circle"));
    };

    // using Value = std::variant<Square, Circle>;
    // using Pattern = matchit::impl::Meet<matchit::impl::AsPointer<Square> >;
    // static_assert(matchit::impl::MatchFuncDefinedV<Value, Pattern>);

    std::variant<Square, Circle> sc = Square{};
    EXPECT_EQ(getIf(sc), "Square");
    sc = Circle{};
    EXPECT_EQ(getIf(sc), "Circle");
}

TEST(Match, test12)
{
    EXPECT_EQ(matchPattern(std::array<int, 2>{1, 2}, ds(ooo, _)), true);
    EXPECT_EQ(matchPattern(std::array<int, 3>{1, 2, 3}, ds(ooo, _)), true);
    Id<int> x;
    EXPECT_EQ(matchPattern(std::array<int, 2>{1, 2}, ds(ooo, _)), true);
}

template <size_t I>
constexpr auto& get(A const &a)
{
    if constexpr (I == 0)
    {
        return a.a;
    }
    else if constexpr (I == 1)
    {
        return a.b;
    }
}

namespace std
{
    template <>
    class tuple_size<A> : public std::integral_constant<size_t, 2>
    {
    };
} // namespace std

TEST(Match, test13)
{
    auto const dsAgg = [](auto const &v) {
        Id<int> i;
        return match(v)(
            pattern(ds(1, i)) = expr(i),
            pattern(ds(_, i)) = expr(i));
    };

    EXPECT_EQ(dsAgg(A{1, 2}), 2);
    EXPECT_EQ(dsAgg(A{3, 2}), 2);
    EXPECT_EQ(dsAgg(A{5, 2}), 2);
    EXPECT_EQ(dsAgg(A{2, 5}), 5);
}

TEST(Match, test14)
{
    auto const anyCast = [](auto const &i) {
        return match(i)(
            pattern(as<Square>(_)) = expr("Square"),
            pattern(as<Circle>(_)) = expr("Circle"));
    };

    std::any sc;
    sc = Square{};
    EXPECT_EQ(anyCast(sc), "Square");
    sc = Circle{};
    EXPECT_EQ(anyCast(sc), "Circle");

    EXPECT_EQ(matchPattern(sc, as<Circle>(_)), true);
    EXPECT_EQ(matchPattern(sc, as<Square>(_)), false);
    // one would write if let like
    // if (matchPattern(value, pattern))
    // {
    //     ...
    // }
}

TEST(Match, test15)
{
    auto const optional = [](auto const &i) {
        Id<char> c;
        return match(i)(
            pattern(none) = expr(1),
            pattern(some(none)) = expr(2),
            pattern(some(some(c))) = expr(c));
    };
    char const **x = nullptr;
    char const *y_ = nullptr;
    char const **y = &y_;
    char const *z_ = "x";
    char const **z = &z_;

    EXPECT_EQ(optional(x), 1);
    EXPECT_EQ(optional(y), 2);
    EXPECT_EQ(optional(z), 'x');
}

TEST(Match, test16)
{
    auto const notX = [](auto const &i) {
        return match(i)(
            pattern(not_(or_(1, 2))) = expr(3),
            pattern(2) = expr(2),
            pattern(_) = expr(1));
    };
    EXPECT_EQ(notX(1), 1);
    EXPECT_EQ(notX(2), 2);
    EXPECT_EQ(notX(3), 3);
}

// when
TEST(Match, test17)
{
    auto const whenX = [](auto const &x) {
        Id<int32_t> i, j;
        return match(x)(
            pattern(i, j).when(i + j == 10) = expr(3),
            pattern(_ < 5, _) = expr(5),
            pattern(_) = expr(1));
    };
    EXPECT_EQ(whenX(std::make_pair(1, 9)), 3);
    EXPECT_EQ(whenX(std::make_pair(1, 7)), 5);
    EXPECT_EQ(whenX(std::make_pair(7, 7)), 1);
}

TEST(Match, test18)
{
    auto const idNotOwn = [](auto const &x) {
        Id<int32_t> i;
        return match(x)(
            pattern(i).when(i == 5) = expr(1),
            pattern(_) = expr(2));
    };
    EXPECT_EQ(idNotOwn(1), 2);
    EXPECT_EQ(idNotOwn(5), 1);
}

#if 0
TEST(Match, test19)
{
    auto const matchFunc = [](auto &&input) {
        Id<int> j;
        return match(input)(
            // `... / 2 3`
            pattern(ds(ooo, '/', 2, 3)) = expr(1),
            // `/ ... 3`
            // pattern(ds('/', ooo, ooo, 3)) = expr(2),
            // `... 3`
            pattern(ds(ooo, 3)) = expr(3),
            // `/ ...`
            pattern(ds('/', ooo)) = expr(4),

            pattern(ds(ooo)) = expr(222),
            // `3 3 3 3 ..` all 3
            pattern(ds(ooo)) = expr(333),

            // `... / ... 3 ...`
            // pattern(ds(ooo, '/', ooo, 3, ooo)) = expr(5),

            // This won't compile since we do compile-time check unless `Seg` is detected.
            // pattern(ds(_, "123", 5)) = expr(1),
            // This will compile
            pattern(ds(ooo, "123", 5)) = expr(6),

            // `... int 3`
            pattern(ds(ooo, j, 3)) = expr(7),
            // `... int 3`
            pattern(ds(ooo, or_(j), 3)) = expr(8),

            // `...`
            // pattern(ds(ooo, ooo, ooo, ooo)) = expr(9), // equal to ds(_)
            // pattern(ds(ooo, ooo, ooo)) = expr(10),
            // pattern(ds(ooo, ooo)) = expr(11),
            pattern(ds(ooo)) = expr(12),

            pattern(_) = expr(-1));
    };
    EXPECT_EQ(matchFunc(std::make_tuple('/', 2, 3)), 1);
    EXPECT_EQ(matchFunc(std::make_tuple('/', "123", 3)), 2);
    EXPECT_EQ(matchFunc(std::make_tuple('*', "123", 3)), 3);
    EXPECT_EQ(matchFunc(std::make_tuple('*', "123", 5)), 6);
    EXPECT_EQ(matchFunc(std::make_tuple('[', '/', ']', 2, 2, 3, 3, 5)), 5);
    EXPECT_EQ(matchFunc(std::make_tuple(3, 3, 3, 3, 3)), 3);
    EXPECT_EQ(matchPattern(std::make_tuple(3, 3, 3, 3, 3), ds(ooo(3))), true);
    EXPECT_EQ(matchPattern(std::make_tuple("123", 3, 3, 3, 2), ds("123", ooo(3), 2)), true);
    EXPECT_EQ(matchPattern(std::make_tuple("string", 3, 3, 3, 3), ds(ooo(2), 3)), false);
    EXPECT_EQ(matchPattern(std::make_tuple(3, 3, 3, 3, 3), ooo(3)), true);
    EXPECT_EQ(matchPattern(std::make_tuple(3, 2, 3, 2, 3), ooo(2)), false);
    EXPECT_EQ(matchPattern(std::make_tuple(2, 2, 2, 2, 2), ooo(2)), true);
    EXPECT_EQ(matchPattern(std::make_tuple("string", 3, 3, 3, 3), ds(ooo(2))), false);
    EXPECT_EQ(matchPattern(std::make_tuple("string"), ds(ooo(5))), false);
    // Debug<decltype(ds(ooo(2)))> x;
    EXPECT_EQ(matchPattern(std::make_tuple(3, 2, 3, 2, 3), ooo(_ > 0)), true);
    EXPECT_EQ(matchPattern(std::make_tuple(3, 2, -3, 2, 3), ooo(_ > 0)), false);
    EXPECT_EQ(matchPattern(std::make_tuple(3, 2, 3, 2, 3), ooo(not_(3))), false);
    EXPECT_EQ(matchPattern(std::make_tuple(2, 2, 2, 2, 3), ooo(not_(3))), false);
    EXPECT_EQ(matchPattern(std::make_tuple(3, 2, 2, 2, 2), ooo(not_(3))), false);
    EXPECT_EQ(matchPattern(std::make_tuple(2, 2, 2, 2, 2), ooo(not_(3))), true);
    {
        Id<int> i;
        EXPECT_EQ(matchPattern(std::make_tuple(3, 2, 2, 3, 3), ds(ooo(i), ooo(2), ooo(i))), true);
        // Id<int> n;
        // TODO, match on segment variable length with oon(pat, n)
        // EXPECT_EQ(matchPattern(std::make_tuple(3, 2, 2, 3, 3), ds(oon(3, n), ooo(2), oon(3, n))), true);
    }
}

TEST(Match, test20)
{
    Id<char> x;
    char y = 'y';
    EXPECT_EQ(matchPattern(
                std::make_tuple('+',
                                1,
                                std::make_tuple('^',
                                                std::make_tuple('s', y),
                                                2),
                                std::make_tuple('^',
                                                std::make_tuple('c', y),
                                                2)),
                ds('+',
                   ooo(1),
                   ds('^',
                      ds('s', x),
                      2),
                   ds('^',
                      ds('c', x),
                      2))),
            true);
    EXPECT_EQ(matchPattern(
                std::make_tuple('+', 1, std::make_tuple('^', std::make_tuple('s', y), 2)),
                ds('+', 1, ds('^', ds(_, x), 2))),
            true);
    EXPECT_EQ(matchPattern(
                std::make_tuple('+', 1, std::make_tuple('^', std::make_tuple('s', y), 2)),
                ds('+', 1, ds('^', ds('s', y), 2))),
            true);
    EXPECT_EQ(matchPattern(
                std::make_tuple('+', 1, std::make_tuple('^', std::make_tuple('s', y), 2)),
                ds('+', 1, ds('^', ds('s', x), 2))),
            true);
    EXPECT_TRUE(matchPattern(2, 2));
}
#endif

TEST(Match, test21)
{
    Id<std::string> strA;
    Id<const char *> strB;
    EXPECT_EQ(matchPattern(
                "abc",
                strA),
            true);
    EXPECT_EQ(matchPattern(
                "abc",
                strB),
            true);
}