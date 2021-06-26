#include "matchit.h"
#include <iostream>
#include <tuple>

struct Player
{
  std::string name;
  int hitpoints;
  int coins;
};

void get_hint(const Player &p)
{
  using namespace matchit;
  using P = Player;
  Id<std::string> n;
  match(p)(
      pattern | app(&P::hitpoints, 1) =
          [&]
      { std::cout << "You're almost destroyed. Give up!\n"; },
      pattern | and_(app(&P::hitpoints, 10), app(&P::coins, 10)) =
          [&]
      { std::cout << "I need the hints from you!\n"; },
      pattern |
          app(&P::coins, 10) = [&]
      { std::cout << "Get more hitpoints!\n"; },
      pattern |
          app(&P::hitpoints, 10) = [&]
      { std::cout << "Get more ammo!\n"; },
      pattern | app(&P::name, n) =
          [&]
      {
        if (*n != "The Bruce Dickenson")
        {
          std::cout << "Get more hitpoints and ammo!\n";
        }
        else
        {
          std::cout << "More cowbell!\n";
        }
      });
}

int32_t main()
{
  const auto p = Player{"Bob", 4, 6};
  get_hint(p);

  return 0;
}
