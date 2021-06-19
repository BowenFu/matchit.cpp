#include "matchit.h"
#include <iostream>

constexpr auto qr = [](auto divisor)
{
    using namespace matchit;
    return [divisor](auto quotient, auto remainder)
    {
        return and_(app(_ / divisor, quotient),
                    app(_ % divisor, remainder));
    };
};

constexpr std::array<int32_t, 2> quoRem(int32_t dividend, int32_t divisor)
{
    using namespace matchit;
    Id<int32_t> q;
    Id<int32_t> r;
    return match(dividend)(
        // clang-format off
        pattern(qr(divisor)(q, r)) = [&] { return std::array<int32_t, 2>{*q, *r}; },
        pattern(_)                 = [&] { return std::array<int32_t, 2>{0, 0}; }
        // clang-format on
    );
}

constexpr auto qrResult1 = quoRem(12, 6);
static_assert(qrResult1[0]== 2);
static_assert(qrResult1[1]== 0);

constexpr auto qrResult2 = quoRem(12, -5);
static_assert(qrResult2[0]== -2);
static_assert(qrResult2[1]== 2);

int main()
{
    auto const result = quoRem(12, 7);
    std::cout << result[0] << "\t" << result[1] << std::endl;
    return 0;
}
