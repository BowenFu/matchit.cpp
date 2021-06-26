#include "matchit.h"
#include <iostream>
#include <memory>

struct Node {
    int value;
    std::unique_ptr<Node> lhs, rhs;
};

bool operator==(Node const& lhs, Node const& rhs)
{
    return lhs.value == rhs.value && lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs;
}

void print_leftmost(const Node& node) {
    auto deref = [](auto&& e) -> decltype(auto) { return *e; };
    using namespace matchit;
    Id<int> v;
    Id<Node> l;

    using N = Node;
    match(node) ( 
        pattern | and_(app(&N::value, v), app(&N::lhs, nullptr)) = [&]{ std::cout << *v << '\n'; },
        pattern | app(&N::lhs, app(deref, l)) = [&]{ print_leftmost(*l); }
//                                ˆˆˆˆˆˆˆˆˆˆˆˆˆ dereference pattern
    );
}


int32_t main()
{
    const auto n  = Node{4, {}, {}};
    print_leftmost(n);

    return 0;
}
