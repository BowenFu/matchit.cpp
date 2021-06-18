#include "matchit.h"
#include <map>
#include <iostream>
using namespace matchit;

template <typename Map, typename Key>
constexpr bool contains(Map const &map, Key const &key)
{
    return match(map.find(key))(
        // clang-format off
        pattern(map.end()) = expr(false),
        pattern(_)         = expr(true)
        // clang-format on
    );
}

int main()
{
    std::cout << contains(std::map<int, int>{{1, 2}}, 1) << std::endl;
    return 0;
}
