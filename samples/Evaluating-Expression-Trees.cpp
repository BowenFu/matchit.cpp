#include "matchit.h"
#include <iostream>
#include <tuple>
#include <memory>

struct Expr;
struct Neg { std::shared_ptr<Expr> expr; };
struct Add { std::shared_ptr<Expr> lhs, rhs; };
struct Mul { std::shared_ptr<Expr> lhs, rhs; };
struct Expr : std::variant<int, Neg, Add, Mul> { using variant::variant; };

bool operator==(Expr const& l, Expr const& r)
{
    return static_cast<std::variant<int, Neg, Add, Mul> const&>(l) == static_cast<std::variant<int, Neg, Add, Mul> const&>(r);
}

namespace std {
template <>
struct variant_size<Expr> : variant_size<Expr::variant> {};
template <std::size_t I>
struct variant_alternative<I, Expr> : variant_alternative<I, Expr::variant> {};
}

constexpr auto dsNeg = [](auto&& expr)
{
    return app(&Neg::expr, expr);
};

constexpr auto dsAdd = [](auto &&lhs, auto &&rhs)
{
    return and_(app(&Add::lhs, lhs),
               app(&Add::rhs, rhs));
};

constexpr auto dsMul = [](auto &&lhs, auto &&rhs)
{
    return and_(app(&Mul::lhs, lhs),
               app(&Mul::rhs, rhs));
};

int eval(const Expr &ex)
{
static_cast<void>(dsNeg);
static_cast<void>(dsAdd);
static_cast<void>(dsMul);

    using namespace matchit;
    Id<int> i;
    Id<Expr> e, l, r;
    return match(ex)(
        // clang-format off
        pattern | as<int>(i)                       = expr(i),
        pattern | as<Neg>(dsNeg(some(e)))             = [&]{ return -eval(*e); },
        pattern | as<Add>(dsAdd(some(l), some(r)))    = [&]{ return eval(*l) + eval(*r); },
        // Optimize multiplication by 0.
        pattern | as<Mul>(dsMul(some(as<int>(0)), _)) = expr(0),
        pattern | as<Mul>(dsMul(_, some(as<int>(0)))) = expr(0),
        pattern | as<Mul>(dsMul(some(l), some(r)))    = [&]{ return eval(*l) * eval(*r); }
        // clang-format on
    );
}

int32_t main()
{
    auto e = Expr{5};
    std::cout << eval(e) << std::endl;

    return 0;
}
