#include <iostream>
#include <limits>
#include "matchit.h"
using namespace matchit;

namespace binary
{
    constexpr uint64_t MEGA = 1024 * 1024;
    constexpr uint64_t GIGA = 1024 * 1024 * 1024;
} // namespace binary

void sample()
{
    constexpr auto c = 'f';
    constexpr auto valid_variable = match(c)(
        // clang-format off
        pattern | ('a' <= _ && _ <= 'z')    = expr(true),
        pattern | ('A' <= _ && _ <= 'Z')    = expr(true),
        pattern | _                       = expr(false)
        // clang-format on
    );
    static_cast<void>(valid_variable);

    constexpr auto ph = 10;
    std::cout << match(ph)(
                     // clang-format off
                     pattern | (0 <= _ && _ <= 6 ) = expr("acid"),
                     pattern | (7                ) = expr("neutral"),
                     pattern | (8 <= _ && _ <= 14) = expr("base"),
                     pattern | (_                ) = [] { assert(false && "unreachable"); return ""; })
                     // clang-format on
              << std::endl;

    // using paths to constants:
    constexpr uint8_t TROPOSPHERE_MIN = 6;
    constexpr uint8_t TROPOSPHERE_MAX = 20;

    constexpr uint8_t STRATOSPHERE_MIN = TROPOSPHERE_MAX + 1;
    constexpr uint8_t STRATOSPHERE_MAX = 50;

    constexpr uint8_t MESOSPHERE_MIN = STRATOSPHERE_MAX + 1;
    constexpr uint8_t MESOSPHERE_MAX = 85;

    constexpr auto altitude = 70;

    std::cout << match(altitude)(
                     // clang-format off
                     pattern | (TROPOSPHERE_MIN  <= _ && _ <= TROPOSPHERE_MAX ) = expr("troposphere"),
                     pattern | (STRATOSPHERE_MIN <= _ && _ <= STRATOSPHERE_MAX) = expr("stratosphere"),
                     pattern | (MESOSPHERE_MIN   <= _ && _ <= MESOSPHERE_MAX  ) = expr("mesosphere"),
                     pattern | (_                                             ) = expr("outer space, maybe"))
                     // clang-format on
              << std::endl;

    constexpr auto n_items = 20'832'425U;
    constexpr auto bytes_per_item = 12U;

    Id<uint64_t> size;
    match(n_items * bytes_per_item)(
        // clang-format off
        pattern(size.at(binary::MEGA <= _ && _ <= binary::GIGA)) = [&] { std::cout << "It fits and occupies " << *size << " bytes" << std::endl; }
        // clang-format on
    );

    // using qualified paths:
    std::cout << match(static_cast<uint64_t>(0xfacade))(
                     // clang-format off
                     pattern(0U <= _ && _ <= std::numeric_limits<uint8_t>::max())  = expr("fits in a u8"),
                     pattern(0U <= _ && _ <= std::numeric_limits<uint16_t>::max()) = expr("fits in a u16"),
                     pattern(0U <= _ && _ <= std::numeric_limits<uint32_t>::max()) = expr("fits in a u32"),
                     pattern | _                                                    = expr("too big"))
              // clang-format on
              << std::endl;
}

int32_t main()
{
    sample();
    return 0;
}