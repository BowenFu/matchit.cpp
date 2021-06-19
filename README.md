# match(it): A light-weight header-only pattern-matching library for C++17 with macro-free APIs.
![match(it).cpp](./matchit.cpp.svg)

[![CMake](https://github.com/BowenFu/matchit.cpp/actions/workflows/cmake.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/cmake.yml)
[![CMake](https://github.com/BowenFu/matchit.cpp/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/sanitizers.yml)
[![CMake](https://github.com/BowenFu/matchit.cpp/actions/workflows/coverage.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/coverage.yml)

[![CodeQL](https://github.com/BowenFu/matchit.cpp/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/codeql-analysis.yml)
[![Codacy](https://github.com/BowenFu/matchit.cpp/actions/workflows/codacy.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/codacy.yml)

[![godbolt][badge.godbolt]][godbolt]
[![GitHub license](https://img.shields.io/github/license/BowenFu/matchit.cpp.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE)
[![Maintained](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/BowenFu/matchit.cpp)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/BowenFu/matchit.cpp) 

[badge.godbolt]: https://img.shields.io/badge/try%20it-on%20godbolt-222266.svg

[godbolt]: https://godbolt.org/z/rMaPvWcbr

## Features

- Single header library.
- Macro-free APIs.
- No heap memory allocation.
- Composable patterns.
- Extensible, users can define their own patterns, either via composing existent ones, or create brand new ones.
- Support destructing tuple-like and range-like containers.
- Partial support for constant expression.

## Related Work

`match(it)` syntax / pattern designs are heavily influenced by these related work

- [mpark/patterns](https://github.com/mpark/patterns)
- [Racket Pattern Matching](https://docs.racket-lang.org/reference/match.html)
- [Rust Patterns](https://doc.rust-lang.org/stable/reference/patterns.html)
- [jbandela/simple_match](https://github.com/jbandela/simple_match/)
- [solodon4/Mach7](https://github.com/solodon4/Mach7)
- [C++ Pattern Matching Proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1371r3.pdf)

## Syntax Design

For syntax design details please refer to [design](./DESIGN.md).

## Basic usage.

The following sample shows to how to implement factorial using the pattern matching library.

```C++
#include "matchit.h"
using namespace matchit;

constexpr int32_t factorial(int32_t n)
{
    assert(n >= 0);
    return match(n)(
        pattern(0) = expr(1),
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

This is a function call and will return some value returned by handlers. The return type is the common type for all handlers. Return type will be void if all handlers do not return values. Incompatible return types from multiple handlers is a compile error.

We can match multiple values at the same time:

```C++
#include "matchit.h"
using namespace matchit;

constexpr int32_t gcd(int32_t a, int32_t b)
{
    return match(a, b)(
        pattern(_, 0) = [&] { return a >= 0 ? a : -a; },
        pattern(_)    = [&] { return gcd(b, a%b); }
    );
}

static_assert(gcd(12, 6) == 6);
```

Note that some patterns support constexpr match, i.e. you can match them at compile time. From the above code snippets, we can see that `gcd(12, 6)` can be executed in compile time.

Now let's go through all kinds of patterns in the library.

## Expression Pattern

The value passed to `match` will be matched against the value evaluated from the expression with `pattern == value`.

```C++
#include "matchit.h"
#include <map>
using namespace matchit;

template <typename Map, typename Key>
constexpr bool contains(Map const& map, Key const& key)
{
    return match(map.find(key))(
        pattern(map.end()) = expr(false),
        pattern(_)         = expr(true)
    );
}
```
Note that the expression `map.end()` can be be used inside `pattern`.
`expr` is a helper function that can be used to generate a nullary function that returns a value. `expr(false)` is equivalent to `[]{return false;}`. It can be useful for short functions.

## Wildcard Pattern

The wildcard `_` will match any values, as we see from the example above. It is a common practice to use it as the last pattern, playing the same role in our library as `default case` does for `switch` statements.
It can be used inside other patterns (that accept subpatterns) as well.

## Predicate Pattern

Predicate Pattern can be used to put some restrictions on the value to be matched.

```C++
#include "matchit.h"
using namespace matchit;

constexpr double relu(double value)
{
    return match(value)(
        pattern(meet([](auto &&v) { return v >= 0; })) = expr(value),
        pattern(_)                                     = expr(0));
}

static_assert(relu(5) == 5);
static_assert(relu(-5) == 0);
```
We overload some operators for wildcard symbol `_` to facilitate usage of basic predicates.
The above sample can be written as

```C++
constexpr double relu(double value)
{
    return match(value)(
        pattern(_ >= 0) = expr(value),
        pattern(_)      = expr(0));
}

static_assert(relu(5) == 5);
static_assert(relu(-5) == 0);
```

## Or Pattern

Or pattern makes it possible to merge/union multiple patterns, thus can be especially useful when used with other subpatterns.

```C++
#include "matchit.h"
using namespace matchit;

constexpr bool isValid(int32_t n)
{
    return match(n)(
        pattern(or_(1, 3, 5)) = expr(true),
        pattern(_)            = expr(false)
    );
}

static_assert(isValid(5));
static_assert(!isValid(6));
```

## And Pattern

And Pattern can be used to combine multiple Predicate patterns.

```C++
#include "matchit.h"
using namespace matchit;

constexpr double clip(double value, double min, double max)
{
    return match(value)(
        pattern(and_(_ >= min, _ <= max)) = expr(value),
        pattern(_ > max)                  = expr(max),
        pattern(_)                        = expr(min)
    );
}

static_assert(clip(5, 0, 10) == 5);
static_assert(clip(5, 6, 10) == 6);
static_assert(clip(5, 0, 4) == 4);
```

The above can also be written as

```C++
#include "matchit.h"
using namespace matchit;

double clip(double value, double min, double max)
{
    return match(value)(
        pattern(min <= _ && _ <= max) = expr(value),
        pattern(_ > max)              = expr(max),
        pattern(_)                    = expr(min)
    );
}
```
. Note that `&&` can only be used between Predicate patterns. `and_` can be used for all kinds of patterns.

## App Pattern

App Pattern is like the projection for ranges introduced in C++20. 
Its syntax is

```C++
app(PROJECTION, PATTERN)
```
.
A simple sample to check whether a num is large:

```C++
#include "matchit.h"
using namespace matchit;

constexpr bool isLarge(double value)
{
    return match(value)(
        pattern(app(_ * _, _ > 1000)) = expr(true),
        pattern(_)                    = expr(false)
    );
}

// app with projection returning scalar types is supported by constexpr match.
static_assert(isLarge(100));
```
Note that `_ * _` generates a function object that computes the square of the input, can be considered the short version of `[](auto&& x){ return x*x;}`.
We suggest using this only for very short and simple functions.

## Identifier Pattern

Users can bind values with `Identifier Pattern`.
Logging the details when detecting large values can be useful for the example above. With Identifier Pattern the codes would be

```C++
#include <iostream>
#include "matchit.h"
using namespace matchit;

bool checkAndlogLarge(double value)
{
    Id<double> s;
    return match(value)(
        pattern(app(_ * _, and_(_ > 1000, s))) = [&] {
                std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
                return true; },
        pattern(_) = expr(false));
}

// comment out std::cout then uncomment this. Outputs are not support in constant expression.
// static_assert(checkAndlogLarge(100));
```
Note that we need to define/declare the identifiers (`Id<double> s`) before using it inside the pattern matching. (Do not mark it as const.)
`*` operator is used to dereference the value inside identifiers.
Identifiers are only valid inside match context.

Note that we used `and_` here to bind a value to the identifier under some conditions on the value.
This practice can achieve the functionality of `@` pattern in Rust.
We recommend always put your Identifier pattern at the end of And pattern. It is like saying that bind the value to the identifier only when all previous patterns / conditions get met.

Also note when the same identifier is bound multiple times, the bound values must equal to each other via `operator==`.
An sample to check if an array is symmetric:

```C++
#include "matchit.h"
using namespace matchit;
constexpr bool symmetric(std::array<int32_t, 5> const& arr)
{
    Id<int32_t> i, j; 
    return match(arr)(
        pattern(i, j, _, j, i) = expr(true),
        pattern(_)             = expr(false)
    );
}

static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) == false);
static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) == true);
static_assert(symmetric(std::array<int32_t, 5>{5, 1, 3, 0, 5}) == false);
```

## Destructure Pattern

We support Destructure Pattern for `std::tuple`, `std::pair`, `std::array`, and all containers (`std::vector`, `std::list`, `std::set`, and so on) with `std::begin` and `std::end` supports. 
We also support the Destructure Pattern for any types that define their own `get` function, (similar to `std::get` for `std::tuple`, `std::pair`, `std::array`).
(It is not possible to overload a function in `std` namespace, we use ADL to look up available `get` functions for other types.)
That is to say, in order to use Destructure Pattern for structs or classes, we need to define a `get` function for them inside the same namespace of the struct or the class. (`std::tuple_size` needs to be specialized as well.)

Note the outermost `ds` inside pattern can be saved. That is to say, when pattern receives multiple parameters, they are treated as subpatterns of a ds pattern.

```C++
#include "matchit.h"
using namespace matchit;

template<typename T1, typename T2>
constexpr auto eval(std::tuple<char, T1, T2> const& expr)
{
    Id<T1> i;
    Id<T2> j;
    return match(expr)(
        pattern('+', i, j) = i + j,
        pattern('-', i, j) = i - j,
        pattern('*', i, j) = i * j,
        pattern('/', i, j) = i / j,
        pattern(_) = []
        {
            assert(false);
            return -1;
        });
}

#if __cplusplus > 201703L
constexpr auto result = eval(std::make_tuple('*', 5, 6));
static_assert(result == 30);
#endif
```

Note that we overload some operators for `Id`, so `i + j` will return a expr function that return the value of `*i + *j`.
We suggest using this only for very short and simple functions.

Also note that `eval` cannot be used for constant expression until C++20, where more STL functions get marked as constexpr.

## Match Guard

Match Guard can be used to exert extra restrictions on a pattern.
The syntax is

```C++
pattern(PATTERN).when(PREDICATE) = HANDLER
```

A basic sample can be

```C++
#include <array>
#include "matchit.h"
using namespace matchit;

constexpr bool sumIs(std::array<int32_t, 2> const& arr, int s)
{
    Id<int32_t> i, j;
    return match(arr)(
        pattern(i, j).when(i + j == s) = expr(true),
        pattern(_)                     = expr(false));
}

static_assert(sumIs(std::array<int32_t, 2>{5, 6}, 11));
```

Note that `i + j == s` will return a expr function that return the result of `*i + *j == s`.

## Ooo Pattern

Ooo Pattern can match arbitrary number of items. It can only be used inside `ds` patterns and at most one Ooo pattern can appear inside a `ds` pattern.

```C++
#include <array>
#include "matchit.h"
using namespace matchit;

template <typename Tuple>
constexpr int32_t detectTuplePattern(Tuple const& tuple)
{
    return match(tuple)
    (
        pattern(2, ooo, 2)  = expr(4),
        pattern(2, ooo)     = expr(3),
        pattern(ooo, 2)     = expr(2),
        pattern(ooo)        = expr(1)
    );
}

static_assert(detectTuplePattern(std::make_tuple(2, 3, 5, 7, 2)) == 4);
```

We support binding a subrange to the ooo pattern now when destructuring a `std::array` or other containers / ranges.
Sample codes can be

```C++
template <typename Range>
constexpr bool recursiveSymmetric(Range const &range)
{
    Id<int32_t> i;
    Id<SubrangeT<Range const>> subrange;
    return match(range)(
        pattern(i, ooo(subrange), i) = [&] { return recursiveSymmetric(*subrange); },
        pattern(i, ooo(subrange), _) = expr(false),
        pattern(_)                   = expr(true)
    );
}
```

Given a range (or a container), we can check it is symmetric via recursive calls.
`SubrangeT` is a trait function and it will return the subrange type based on a given range / container.
We defined a `Subrange` class template (similar to what we will have in C++20) to make binding ooo patterns possible.
When upgrading to C++20, we will replace the class with `std::ranges::subrange`.

## Compose Patterns

### Some / None Patterns

Some / None Patterns can be used to match raw pointers, `std::optional`, `std::unique_ptr`, `std::shared_ptr` and other types that can be converted to bool and dereferenced.
A typical sample can be

```C++
#include "matchit.h"
using namespace matchit;

template <typename T>
constexpr auto square(std::optional<T> const& t)
{
    Id<T> id;
    return match(t)(
        pattern(some(id)) = id * id,
        pattern(none) = expr(0));
}
constexpr auto x = std::make_optional(5);
static_assert(square(x) == 25);
```

Some and none patterns are not atomic patterns, they are composed via

```C++
template <typename T>
constexpr auto cast = [](auto && input) {
    return static_cast<T>(input);
}; 

constexpr auto deref = [](auto &&x) { return *x; };

constexpr auto some = [](auto const pat) {
    return and_(app(cast<bool>, true), app(deref, pat));
};

constexpr auto none = app(cast<bool>, false);
```

For `some` pattern, first we cast the value to a boolean value, if the boolean value is true, we can further dereference it. Otherwise, the match fails.
For none pattern we simply check if the converted boolean value is false.

`Some` and `none` patterns can be used to lift functions for `std::optional`, `std::unique_ptr` and so on, refer to `samples/optionalLift.cpp`.

### As Pattern

As pattern can be used to handle `sum type`, including base / derived classes, `std::variant`, and `std::any`.
A simple sample can be
```C++
#include <iostream>
#include "matchit.h"
using namespace matchit;

struct Shape
{
    virtual ~Shape() = default;
};
struct Circle : Shape {};
struct Square : Shape {};

auto getClassName(Shape const &s)
{
    return match(s)(
        pattern(as<Circle>(_)) = expr("Circle"),
        pattern(as<Square>(_)) = expr("Square")
    );
}

int main()
{
    Circle c{};
    std::cout << getClassName(c) << std::endl;
    return 0;
}
```

As pattern is not an atomic pattern, either. It is composed via

```C++
template <typename T>
constexpr AsPointer<T> asPointer;

template <typename T>
constexpr auto as = [](auto const pat) {
    return app(asPointer<T>, some(pat));
};
```

#### Customization Point of `As` Pattern

The default `As` Pattern for down casting is calling `dynamic_cast`.
Users can customize their down casting via specializing `CustomAsPointer`:

```C++
#include <iostream>
#include "matchit.h"
using namespace matchit;

enum class Kind { kONE, kTWO };

class Num
{
public:
    virtual ~Num() = default;
    virtual Kind kind() const = 0;
};

class One : public Num
{
public:
    constexpr static auto k = Kind::kONE;
    Kind kind() const override
    {
        return k;
    }
};

class Two : public Num
{
public:
    constexpr static auto k = Kind::kTWO;
    Kind kind() const override
    {
        return k;
    }
};

template <Kind k>
constexpr auto kind = app(&Num::kind, k);

template <typename T>
class NumAsPointer
{
public:
    auto operator()(Num const& num) const
    {
        std::cout << "custom as pointer." << std::endl;
        return num.kind() == T::k ? static_cast<T const *>(std::addressof(num)) : nullptr;
    }
};

template <>
class matchit::impl::CustomAsPointer<One> : public NumAsPointer<One> {};

template <>
class matchit::impl::CustomAsPointer<Two> : public NumAsPointer<Two> {};

int staticCastAs(Num const& input)
{
    return match(input)(
        pattern(as<One>(_))       = expr(1),
        pattern(kind<Kind::kTWO>) = expr(2),
        pattern(_)                = expr(3));
}

int main()
{
    std::cout << staticCastAs(One{}) << std::endl;
    return 0;
}
```

`std::variant` and `std::any` can be visited as

```C++
#include "matchit.h"
using namespace matchit;

template <typename T>
constexpr auto getClassName(T const& v)
{
    return match(v)(
        pattern(as<char const*>(_)) = expr("chars"),
        pattern(as<int32_t>(_))     = expr("int")
    );
}

constexpr std::variant<int32_t, char const*> v = 123;
static_assert(getClassName(v) == std::string_view{"int"});
```

## Customziation Point

Users can specialize `PatternTraits` if they want to add a new pattern.