#include "matchit.h"
#include <iostream>
using namespace matchit;

struct Hello
{
  int32_t id;
};
using Message = std::variant<Hello>;

int32_t main()
{
  Message const msg = Hello{5};

  using namespace matchit;
  auto const asHelloDs = asDsVia<Hello>(&Hello::id);
  Id<int32_t> id_variable;
  match(msg)(
      pattern | asHelloDs(id_variable.at(3 <= _ && _ <= 7)) =
          [&]
      {
        std::cout << "Found an id in range: " << *id_variable << std::endl;
      },
      pattern | asHelloDs(10 <= _ && _ <= 12) =
          [&]
      { std::cout << "Found an id in another range" << std::endl; },
      pattern | asHelloDs(id_variable) =
          [&]
      {
        std::cout << "Found some other id: " << *id_variable << std::endl;
      });
}