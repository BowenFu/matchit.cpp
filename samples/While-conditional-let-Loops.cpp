#include <iostream>
#include <stack>
#include <optional>
#include "matchit.h"

int32_t main()
{
    std::stack<int32_t> stack;

    stack.push(1);
    stack.push(2);
    stack.push(3);

    auto const safePop = [](std::stack<int32_t>& s) -> std::optional<int32_t>
    {
        try
        {
            if (s.empty())
            {
                throw "empty";
            }
            auto top = s.top();
            s.pop();
            return top;
        }
        catch (...)
        {
            return {};
        }
        
    };

    using namespace matchit;
    Id<int32_t> top;
    while (
        match(safePop(stack))(
            pattern(some(top)) = [&]
            {
                std::cout << *top << std::endl;
                return true;
            },
            pattern(_) = expr(false)))
    {
    };
}