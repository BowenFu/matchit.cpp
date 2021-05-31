# match(it): A light-weight pattern-matching library for C++17.
![match(it).cpp](./match(it).cpp.svg)

## Basic usage.
The following sample shows to how to implement factorial using the pattern matching library.
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

int32_t factorial(int32_t n)
{
    assert(n >= 0);
    return match(n)(
        pattern(0) = [] { return 1; },
        pattern(_) = [n] { return n * factorial(n - 1); }
    );
}
```

The basic syntax is
```C++
match(VALUE)
(
    pattern(PATTERN1) = HANDLER1,
    pattern(PATTERN2) = HANDLER2,
    ...
)
```

This is an expression and will be evaluated to some value returned by handlers.
Now let's go through all kinds of patterns in the library.

We can match multiple values at the same time:
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

int32_t gcd(int32_t a, int32_t b)
{
    return match(a, b)(
        pattern(_, 0) = [&] { return std::abs(a); },
        pattern(_)    = [&] { return gcd(b, a%b); }
    );
}
```

## Expression Pattern
The value passed to `match` will be matched against the value evaluated from the expression with `pattern == value`.
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

int32_t divide(int32_t dividen, int32_t divisor)
{
    return match(dividen)(
        pattern(divisor)     = [] { return 1; },
        pattern(divisor * 2) = [] { return 2; },
        pattern(divisor * 3) = [] { return 3; },
        pattern(_)           = [] { return -1; }
    );
}
```
Note that the variable `divisor` can be used inside `pattern`.

## Wildcard Pattern
The wildcard `_` will match any patterns, as we see from the example above. It is a common practice to use it as the last pattern, playing the same role in our library as `default case` does for `switch` statements.
It can be used inside other patterns (that accpet subpatterns) as well.

## Identifier Pattern
Users can bind values with `Identifier Pattern`.

