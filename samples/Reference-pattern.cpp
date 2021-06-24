#include <iostream>
#include "matchit.h"
using namespace matchit;

void sample()
{
    int32_t const value = 3;
    int32_t const *int_reference = &value;

    int32_t const zero = 0;

    auto const a = match(*int_reference)(
        pattern | zero = expr("zero"),
        pattern | _    = expr("some"));

    auto const b = match(int_reference)(
        pattern | &zero = expr("zero"),
        pattern | _     = expr("some"));

    static_cast<void>(a);
    static_cast<void>(b);

    assert(a == b);
}

int32_t main()
{
    sample();
    return 0;
}