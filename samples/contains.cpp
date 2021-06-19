#include "matchit.h"
#include <map>
#include <iostream>

template <typename Map, typename Key>
constexpr bool contains(Map const &map, Key const &key)
{
    using namespace matchit;
    return match(map.find(key))(
        // clang-format off
        pattern(map.end()) = expr(false),
        pattern(_)         = expr(true)
        // clang-format on
    );
}

int32_t main()
{
    std::cout << contains(std::map<int32_t, int32_t>{{1, 2}}, 1) << std::endl;
    return 0;
}
