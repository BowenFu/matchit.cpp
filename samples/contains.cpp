#include "matchit.h"

#include <map>
#include <iostream>
using namespace matchit;

template <typename Map, typename Key>
constexpr bool contains(Map const& map, Key const& key)
{
    return match(map.find(key))(
        pattern(map.end()) = [] { return false; },
        pattern(_)         = [] { return true; }
    );
}

int main()
{
    std::cout << contains(std::map<int, int>{{1,2}}, 1) << std::endl;
    return 0;
}
