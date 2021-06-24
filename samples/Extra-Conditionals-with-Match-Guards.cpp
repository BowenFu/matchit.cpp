#include <iostream>
#include <optional>
#include "matchit.h"
using namespace matchit;

void sample1()
{
    auto const num = std::make_optional(4);

    Id<int32_t> x;
    match(num)(
        // pattern(some(x)).when(x < 5) = [&] { std::cout << "less than five: " << *x; },
        pattern(some(x.at(_ < 5))) = [&]
        { std::cout << "less than five: " << *x << std::endl;  },
        pattern(some(x)) = [&]
        { std::cout << *x << std::endl; },
        pattern | none = [&] {});
}

template <typename T>
std::ostream& operator<< (std::ostream& o, std::optional<T> const& op)
{
    if (op)
    {
        o << *op;
    }
    else
    {
        o << "none";
    }
    return o;
}

void sample2()
{
    auto const x = std::make_optional(5);
    auto const y = 10;

    Id<int32_t> n;
    match(x)( 
        // clang-format off
        pattern(some(50))             = [&]{ std::cout << "Got 50" << std::endl; },
        // pattern(some(n)).when(n == y) = [&]{ std::cout << "Matched, n = " << *n << std::endl; },
        // In `match(it)`, you can use variable inside patterns, just like literals.
        pattern(some(y))              = [&]{ std::cout << "Matched, n = " << *n << std::endl; },
        pattern | _                    = [&]{ std::cout << "Default case, x = " << x << std::endl; }
        // clang-format on
    );

    std::cout << "at the end: x = " << x << ", y = " << y << std::endl;
}

void sample3()
{
    auto const x = 4;
    auto const y = false;

    std::cout <<
        match(x)( 
            // clang-format off
            pattern(or_(4, 5, 6)).when(expr(y)) = expr("yes"),
            pattern | _                          = expr("no")
            // clang-format on
    ) << std::endl;
}

int32_t main()
{
    sample1();
    sample2();
    sample3();
    return 0;
}