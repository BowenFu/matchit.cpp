#include "matchit.h"
#include <iostream>

auto within = [](auto first, auto last)
{
    using matchit::_;
    return first <= _ && _ <= last;
};

int main()
{
    using namespace matchit;
    auto n = 5;
    match(n)(
        // clang-format off
        pattern | within(1, 10) = [&] { std::cout << n << " is in [1, 10]."; }, // 1..10
        pattern | _             = [&] { std::cout << n << " is not in [1, 10]."; }
        // clang-format on
    );
    std::cout << std::endl;
    return 0;
}
