#include "matchit.h"
#include <iostream>
#include <memory>
#include <tuple>
using namespace matchit;

struct Expr;
struct Neg
{
  std::shared_ptr<Expr> expr;
};
struct Add
{
  std::shared_ptr<Expr> lhs, rhs;
};
struct Mul
{
  std::shared_ptr<Expr> lhs, rhs;
};
struct Expr : std::variant<int, Neg, Add, Mul>
{
  using variant::variant;
};

namespace std
{
  template <>
  struct variant_size<Expr> : variant_size<Expr::variant>
  {
  };
  template <std::size_t I>
  struct variant_alternative<I, Expr> : variant_alternative<I, Expr::variant>
  {
  };
} // namespace std

bool operator==(Expr const &l, Expr const &r)
{
  return static_cast<std::variant<int, Neg, Add, Mul> const &>(l) ==
         static_cast<std::variant<int, Neg, Add, Mul> const &>(r);
}

const auto asNegDs = asDsVia<Neg>(&Neg::expr);
const auto asAddDs = asDsVia<Add>(&Add::lhs, &Add::rhs);
const auto asMulDs = asDsVia<Mul>(&Mul::lhs, &Mul::rhs);

int eval(const Expr &ex)
{
  Id<int> i;
  Id<Expr> e, l, r;
  return match(ex)(
      // clang-format off
        // FIXME: Expr{5} won't match the following line.
        pattern | as<int>(i)                   = expr(i),
        pattern | asNegDs(some(e))             = [&]{ return -eval(*e); },
        pattern | asAddDs(some(l), some(r))    = [&]{ return eval(*l) + eval(*r); },
        // Optimize multiplication by 0.
        pattern | asMulDs(some(as<int>(0)), _) = expr(0),
        pattern | asMulDs(_, some(as<int>(0))) = expr(0),
        pattern | asMulDs(some(l), some(r))    = [&]{ return eval(*l) * eval(*r); },
        pattern | _                            = expr(-1)
      // clang-format on
  );
}

int32_t main()
{
  auto e = Expr{5};
  std::cout << eval(e) << std::endl;

  return 0;
}
