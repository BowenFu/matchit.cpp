#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
#include "matchit/expression.h"
using namespace matchit;

struct A
{
    int a;
    std::string b;
};
bool operator==(A const& lhs, A const& rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b;
}

template <size_t I>
constexpr auto const& get(A const &a)
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

template <size_t I>
constexpr auto&& get(A &&a)
{
    if constexpr (I == 0)
    {
        return std::move(a.a);
    }
    else if constexpr (I == 1)
    {
        return std::move(a.b);
    }
}

namespace std
{
    template <>
    class tuple_size<A> : public std::integral_constant<size_t, 2>
    {
    };
} // namespace std

template <typename T>
auto getSecond(T&& v)
{
    Id<std::string> i;
    return match(std::forward<T>(v))(
        pattern(ds(2, i)) = expr(i),
        pattern(ds(_, i)) = expr(i));
}

int main()
{
    std::cout << getSecond(A{1, "123"}) << std::endl;
    return 0;
}
