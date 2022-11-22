#include "matchit.h"
#include <iostream>
#include <optional>
#include <stack>

int32_t main()
{
  std::stack<int32_t> stack;

  stack.push(1);
  stack.push(2);
  stack.push(3);

  auto const safePop = [](std::stack<int32_t> &s) -> std::optional<int32_t>
  {
    auto const result = std::optional<int32_t>{};
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
      return result;
    }
  };

  using namespace matchit;
  Id<int32_t> top;
  while (match(safePop(stack))(
      pattern | some(top) =
          [&]
      {
        std::cout << *top << std::endl;
        return true;
      },
      pattern | _ = false))
  {
  };
}