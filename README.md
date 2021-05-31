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
#include <map>
using namespace matchit;

template <typename Map, typename Key>
bool contains(Map const& map, Key const& key)
{
    return match(map.find(key))(
        pattern(map.end()) = [] { return false; },
        pattern(_)         = [] { return true; }
    );
}
```
Note that the expression `map.end()` can be be used inside `pattern`.

## Wildcard Pattern
The wildcard `_` will match any patterns, as we see from the example above. It is a common practice to use it as the last pattern, playing the same role in our library as `default case` does for `switch` statements.
It can be used inside other patterns (that accpet subpatterns) as well.

## Predicate Pattern
Predicate Pattern can be used to cast some restrictions on the value to be matched.
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

double relu(double value)
{
    return match(value)(
        pattern(meet([](auto &&v) { return v >= 0; })) = [&] { return value; },
        pattern(_) = [] { return 0; });
}
```
We overload some operators for wildcard symbol `_` to faciliate usage of basic predicates.
The above sample can be written as
```C++
double relu(double value)
{
    return match(value)(
        pattern(_ >= 0) = [&] { return value; },
        pattern(_) = [] { return 0; });
}
```

## Or Pattern
Or pattern makes it possible to merge/union multiple patterns, this can be especially useful when used as subpatterns of other patterns.
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

bool isValid(int32_t n)
{
    return match(n)(
        pattern(or_(1, 3, 5)) = []{ return true; },
        pattern(_)            = []{ return false; }
    );
}
```

## And Pattern
And Pattern can be used to combine multiple Predicate patterns.
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

double clip(double value, double min, double max)
{
    return match(value)(
        pattern(and_(_ >= min, _ <= max)) = [&] { return value; },
        pattern(_ > max)                  = [&] { return max; },
        pattern(_)                        = [&] { return min; }
    );
}
```

## App Pattern
App Pattern is like the projection for ranges introduced in C++20. 
Its syntax is
```C++
app(PROJECTION, PATTERN)
```
.
An sample to check whether a num is large:
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

bool isLarge(double value)
{
    return match(value)(
        pattern(app([](int32_t x) { return x * x; }, _ > 1000)) = [] { return true; },
        pattern(_)                                              = [&] { return false; }
    );
}
```

## Identifier Pattern
Users can bind values with `Identifier Pattern`.
Logging the details when detect large values can be useful for the example above. With Identifier Pattern the codes would be
```C++
#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

bool checkAndlogLarge(double value)
{
    auto const square = [](auto &&v) { return v * v; };
    Id<double> s;
    return match(value)(
        pattern(app(square, and_(_ > 1000, s))) = [&] {
                std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
                return true; },
        pattern(_) = [&] { return false; });
}
```
Note that we need to define declare the identifiers (`Id<double> s`) before using it inside the pattern matching.
`*` operator is used to dereference the value inside identifiers.

