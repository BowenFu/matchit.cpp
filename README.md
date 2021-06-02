# match(it): A light-weight pattern-matching library for C++17.
![match(it).cpp](./matchit.cpp.svg)

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
[![godbolt][badge.godbolt]][godbolt]

[badge.godbolt]: https://img.shields.io/badge/try%20it-on%20godbolt-222266.svg

[godbolt]: https://godbolt.org/z/d5cacaffq

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

Note that we used `and_` here to bind a value to the identifier under some conditions on the value.
This practice can achieve the functionality of `@` pattern in Rust.

TODO : Maybe we can make our `Id` work like an `immutable` data structure in Functional Programming, so to support thread safety.

## Destructure Pattern
We support Destucture Pattern for `std::tuple`, `std::pair`, and `std::array`. Each of them has a `std::get` function defined for it.
Since it is not possible to overload a function in `std` namespace, we use ADL to look up avialable `get` functions for other types.
That is to say, in order to use Destructure Pattern for structs or classes, we need to define a `get` function for them, similar to `std::get`, but inside the same namespace of the struct or the class. (`std::tuple_size` needs to be specialized as well.)
```C++
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

template<typename T1, typename T2>
auto eval(std::tuple<char, T1, T2> const& expr)
{
        Id<T1> i;
        Id<T2> j;
        return match(expr)(
            pattern(ds('+', i, j)) = [&i, &j] { return *i + *j; },
            pattern(ds('-', i, j)) = [&i, &j] { return *i - *j; },
            pattern(ds('*', i, j)) = [&i, &j] { return *i * *j; },
            pattern(ds('/', i, j)) = [&i, &j] { return *i / *j; },
            pattern(_) = [] { assert(false); return -1; });
}
```

## Match Guard
Match Guard can be used to cast extra restrictions on a pattern.
The syntax is
```C++
pattern(PATTERN).when(PREDICATE) = HANDLER
```
.

```C++
#include <array>
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

bool sumIs(std::array<int32_t, 2> const& arr, int s)
{
    Id<int32_t> i, j;
    return match(arr)(
        pattern(i, j).when([&] { return *i + *j == s; }) = [] { return true; },
        pattern(_)                                       = [] { return false; });
}
```

## Ooo Pattern
Ooo Pattern can match aribitrary number of items. It can only be used inside `ds` patterns.
```C++
#include <array>
#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

template <typename Tuple>
int32_t detectTuplePattern(Tuple const& tuple)
{
    return match(tuple)
    (
        pattern(ds(ooo(3)))             = []{return 1;}, // all 3
        pattern(ds(_, ooo(3)))          = []{return 2;}, // all 3 except the first one
        pattern(ds(ooo(3), _))          = []{return 3;}, // all 3 except the last one
        pattern(ds(_, ooo(3), _))       = []{return 4;}, // all 3 except the first and the last one
        pattern(ds(3, ooo(not_(3)), 3)) = []{return 5;}, // all non 3 except the first and the last one
        pattern(ds(3, ooo(_), 3))       = []{return 6;}, // first and last being 3, mxied by 3 and non-3 in the middle.
        pattern(_)                      = []{return 7;}  // mismatch
    );
}

int main()
{
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 3, 3, 3, 3))); // pattern 1
    printf("%d\n", detectTuplePattern(std::make_tuple(2, 3, 3, 3, 3))); // pattern 2
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 3, 3, 2)));    // pattern 3
    printf("%d\n", detectTuplePattern(std::make_tuple(2, 3, 3, 3, 2))); // pattern 4
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 4, 2, 4, 3))); // pattern 5
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 3, 2, 3, 3))); // pattern 6
    printf("%d\n", detectTuplePattern(std::make_tuple(2, 3, 2, 3, 3))); // pattern 7
    return 0;
}
```

## Compose Patterns
TODO : some / none / as
### Some / None Patterns
Some / None Patterns can be used to match raw pointers, std::optional, std::unique_ptr, std::shared_ptr and other types that can be converted to bool and dereferenced.
A typical sample can be
```C++
#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
#include "include/utility.h"
using namespace matchit;

template <typename T>
auto square(T const* t)
{
    Id<T> id;
    return match(t)(
        pattern(some(id)) = [&id] { return *id * *id; },
        pattern(none) = [] { return 0; });
}

int main()
{
    auto t = 3;
    std::cout << square(&t) << std::endl;
    return 0;
}
```

Some and none patterns are not atomic patterns, they are composed via
```C++
template <typename T>
auto constexpr cast = [](auto && input) {
    return static_cast<T>(input);
}; 

auto constexpr some = [](auto const pat) {
    auto constexpr deref = [](auto &&x) { return *x; };
    return and_(app(cast<bool>, true), app(deref, pat));
};

auto constexpr none = app(cast<bool>, false);
```
For some pattern, first we cast the value to bool, if the boolean value is true, we can further dereference it. Otherwise, the match fails.
For none pattern we simply check if the converted boolean value is false.

Some and none patterns can be used to lift functions for `std::optional`, `std::unique_ptr` and so on, refer to `samples/optionalLift.cpp`.

###