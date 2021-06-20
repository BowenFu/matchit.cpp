#include <iostream>
#include <vector>
#include "matchit.h"
using namespace matchit;

template <typename T>
std::ostream& operator << (std::ostream& o, Subrange<T> const& subrange)
{
    o << " [ ";
    for (auto const& e : subrange)
    {
        o << e << ", ";
    }
    o << " ] ";
    return o;
}

void sample()
{
    auto const words = std::vector<std::string>{"a", "b", "c"};
    auto const &slice = words;
    Id<std::string> head;
    Id<SubrangeT<std::vector<std::string> const>> tail;
    match(slice)(
        // clang-format off
        pattern(ds())                   = [&] { std::cout << "slice is empty" << std::endl; },
        pattern(ds(head))               = [&] { std::cout << "single element " << *head << std::endl; },
        pattern(ds(head, tail.at(ooo))) = [&] { std::cout << "head=" << *head << " tail=" << *tail << std::endl; }
        // clang-format on
    );

    Id<SubrangeT<std::vector<std::string> const>> subrange;
    match(slice)(
        // clang-format off
        // Ignore everything but the last element, which must be "!".
        pattern(ds(ooo, "!"))              = [&] { std::cout << "!!!" << std::endl; },

        // `subrange` is a slice of everything except the last element, which must be "z".
        pattern(ds(subrange.at(ooo), "z")) = [&] { std::cout << "starts with: " << *subrange << std::endl; },

        // `subrange` is a slice of everything but the first element, which must be "a".
        pattern(ds("a", subrange.at(ooo))) = [&] { std::cout << "ends with: " << *subrange << std::endl; },

        pattern(ds(subrange.at(ooo)))      = [&] { std::cout << *subrange << std::endl; }
        // clang-format on
    );

    Id<std::string> penultimate;
    match(slice)(
        // clang-format off
        pattern(ooo, penultimate, _) = [&] { std::cout << "next to last is " << *penultimate << std::endl; }
        // clang-format on
    );

    constexpr auto tuple = std::make_tuple(1, 2, 3, 4, 5);
    // Rest patterns may also be used in tuple and tuple struct patterns.
    Id<int32_t> y, z;
    match(tuple)(
        // clang-format off
        pattern(1, ooo, y, z) = [&] { std::cout << "y=" << *y << " z=" << *z << std::endl; },
        pattern(ooo, 5)       = [&] { std::cout << "tail must be 5" << std::endl; },
        pattern(ooo)          = [&] { std::cout << "matches everything else" << std::endl; }
        // clang-format on
    );
}

int32_t main()
{
    sample();
    return 0;
}