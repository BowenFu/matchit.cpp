#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
#include "include/utility.h"
#include "include/expression.h"
using namespace matchit;

struct A
{
    int a;
    int b;
};
bool operator==(A const lhs, A const rhs)
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
    assert(std::get<0>(matchit::impl::drop<1>(v)) == 2);
    Id<int> i;
    return match(std::forward<T>(v))(
        pattern(ds(1, i)) = expr(i),
        pattern(ds(_, i)) = expr(i));
}

int main()
{
    std::cout << getSecond(A{1,2}) << std::endl;
    return 0;
}
