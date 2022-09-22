# match(it): A lightweight header-only pattern-matching library for C++17 with macro-free APIs.

![match(it).cpp](./matchit.cpp.svg)

![Standard](https://img.shields.io/badge/c%2B%2B-17/20-blue.svg)
![Type](https://img.shields.io/badge/type-single--header-blue.svg)
![Type](https://img.shields.io/badge/type-macro--free-blue)

![Platform](https://img.shields.io/badge/platform-linux-blue)
![Platform](https://img.shields.io/badge/platform-osx-blue)
![Platform](https://img.shields.io/badge/platform-win-blue)

[![CMake](https://github.com/BowenFu/matchit.cpp/actions/workflows/cmake.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/cmake.yml)
[![CMake](https://github.com/BowenFu/matchit.cpp/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/BowenFu/matchit.cpp/actions/workflows/sanitizers.yml)
![GitHub license](https://img.shields.io/github/license/BowenFu/matchit.cpp.svg)
[![codecov](https://codecov.io/gh/BowenFu/matchit.cpp/branch/main/graph/badge.svg?token=G5B0RE6THD)](https://codecov.io/gh/BowenFu/matchit.cpp)

[badge.godbolt]: https://img.shields.io/badge/try-godbolt-blue
[godbolt]: https://godbolt.org/z/8YMr8Kz8j

## Features

- Easy to get started.
  - [![godbolt][badge.godbolt]][godbolt]

- Single header library.
- Macro-free APIs.
- **No heap memory allocation.**
- Portability: continuously tested under Ubuntu, MacOS and Windows using GCC/Clang/MSVC. 
- No external dependencies.
- Reliability : strict compiler checking option + sanitizers + valgrind.
- Composable patterns.
- Extensible, users can define their own patterns, either via composing existent ones, or create brand new ones.
- Support destructing tuple-like and range-like containers.
- Partial support for constant expression.

## Installation

### Option 1. Download `matchit.h`

Simply download the header file [`matchit.h`](https://raw.githubusercontent.com/BowenFu/matchit.cpp/main/include/matchit.h) and put it in your include directory for dependencies.

That's it.

You can download via this bash command

```bash
wget https://raw.githubusercontent.com/BowenFu/matchit.cpp/main/include/matchit.h
```

### Option 2. Manage with cmake FetchContent

Include the code snippet in your CMakeLists.txt:

```CMake
include(FetchContent)

FetchContent_Declare(
    matchit
    GIT_REPOSITORY https://github.com/BowenFu/matchit.cpp.git
    GIT_TAG main)

FetchContent_GetProperties(matchit)
if(NOT matchit_POPULATED)
    FetchContent_Populate(matchit)
    add_subdirectory(${matchit_SOURCE_DIR} ${matchit_BINARY_DIR}
                    EXCLUDE_FROM_ALL)
endif()

message(STATUS "Matchit header are present at ${matchit_SOURCE_DIR}")
```

And add `${matchit_SOURCE_DIR}/include` to your include path.

Replace `main` with latest release tag to avoid API compatibility breaking.

### Option 3. Manage with cmake find_package

Clone the repo via
```
git clone --depth 1 https://github.com/BowenFu/matchit.cpp
```

Install the library via
```
cd matchit.cpp
cmake -B ./build
cd build
make install
```

Then use find_package in your CMakeLists.txt.

### Option 4. Manage with vcpkg

(Thanks to @[daljit97](https://github.com/daljit97) for adding the support.)

```
vcpkg install matchit
```

### Option 5. Manage with conan

Now the library has been submitted to [Conan Center Index](https://github.com/conan-io/conan-center-index).

You can now install the library via conan.

(Thanks to @[sanblch](https://github.com/sanblch) for adding the support.)


## Tips for Debugging

To make your debugging easier, try to write your lambda function body in separate lines so that you can set break points in it.

```c++
pattern | xyz = [&]
{
    // Separate lines for function body <- set break points here
}
```

is much more debugging-friendly compared to

```c++
pattern | xyz = [&] { /* some codes here */ }, // <- Set break points here, you will debug into the library.
```

Do not debug into this library unless you really decide to root cause / fix some bugs in this library, just like you won't debug into STL variant or ranges.

Please try to create a minimal sample to reproduce the issues you've met. You can root cause the issue more quickly in that way.

You can also create an issue in this repo and attach the minimal sample codes and I'll try to response as soon as possible (sometimes please expect one or two days delay).

## Syntax Design

For syntax design details please refer to [REFERENCE](./REFERENCE.md).

## From Rust to match(it)

The document [From Rust to match(it)](./From-Rust-to-matchit.md) gives equivalent samples for corresponding Rust samples.

There you may have a picture of what coding with `match(it)` would be like.

## From Pattern Matching Proposal to match(it)

The document [From Pattern Matching Proposal to match(it)](./From-Pattern-Matching-Proposal-to-matchit.md) gives equivalent samples for corresponding samples in the Match Pattern Matching Proposal.

There you will see the pros and cons of the library over the proposal.

## Quick start.

Let's start a journey on the library!

(For complete samples, please refer to [samples directory](./sample).)

### Hello World!

The following sample shows to how to implement factorial using `match(it)` library.

```C++
#include "matchit.h"

constexpr int32_t factorial(int32_t n)
{
    using namespace matchit;
    assert(n >= 0);
    return match(n)(
        pattern | 0 = expr(1),
        pattern | _ = [n] { return n * factorial(n - 1); }
    );
}
```

The **basic syntax** for pattern matching is

```C++
match(VALUE)
(
    pattern | PATTERN1 = HANDLER1,
    pattern | PATTERN2 = HANDLER2,
    ...
)
```

This is a function call and will return some value returned by handlers. The return type is the common type for all handlers. Return type will be void if all handlers do not return values. Incompatible return types from multiple handlers is a compile error.
When handlers return values, the patterns must be exhaustive. A runtime error will happen if all patterns do not get matched.
It is not an error if handlers' return types are all void.

`expr` in the above sample is a helper function that can be used to generate a nullary function that returns a value. `expr(1)` is equivalent to `[]{return 1;}`. It can be useful for short functions.

The wildcard `_` will match any values. It is a common practice to always use it as the last pattern, playing the same role in our library as `default case` does for `switch` statements, to avoid case escaping.

We can match **multiple values** at the same time:

```C++
#include "matchit.h"

constexpr int32_t gcd(int32_t a, int32_t b)
{
    using namespace matchit;
    return match(a, b)(
        pattern | ds(_, 0) = [&] { return a >= 0 ? a : -a; },
        pattern | _        = [&] { return gcd(b, a%b); }
    );
}

static_assert(gcd(12, 6) == 6);
```

Note that some patterns support constexpr match, i.e. you can match them at compile time. From the above code snippets, we can see that `gcd(12, 6)` can be executed in compile time.

Different from matching patterns in other programming languages, **variables can be used normally inside patterns** in `match(it)`, this is shown in the following sample:

```C++
#include "matchit.h"
#include <map>

template <typename Map, typename Key>
constexpr bool contains(Map const& map, Key const& key)
{
    using namespace matchit;
    return match(map.find(key))(
        pattern | map.end() = expr(false),
        pattern | _         = expr(true)
    );
}
```

### Hello Moon!

We can use **Predicate Pattern** to put some restrictions on the value to be matched.

```C++
constexpr double relu(double value)
{
    return match(value)(
        pattern | (_ >= 0) = expr(value),
        pattern | _        = expr(0));
}

static_assert(relu(5) == 5);
static_assert(relu(-5) == 0);
```

We overload some operators for wildcard symbol `_` to facilitate usage of basic predicates. 

Sometimes we want to share one handler for multiple patterns, **Or Pattern** is the rescue:

```C++
#include "matchit.h"

constexpr bool isValid(int32_t n)
{
    using namespace matchit;
    return match(n)(
        pattern | or_(1, 3, 5) = expr(true),
        pattern | _            = expr(false)
    );
}

static_assert(isValid(5));
static_assert(!isValid(6));
```

**And Pattern** is for combining multiple Predicate patterns.

**App Pattern** is powerful when you want to extract some information from the subject.
Its syntax is

```C++
app(PROJECTION, PATTERN)
```

A simple sample to check whether a num is large can be:

```C++
#include "matchit.h"

constexpr bool isLarge(double value)
{
    using namespace matchit;
    return match(value)(
        pattern | app(_ * _, _ > 1000) = expr(true),
        pattern | _                    = expr(false)
    );
}

// app with projection returning scalar types is supported by constexpr match.
static_assert(isLarge(100));
```

Note that `_ * _` generates a function object that computes the square of the input, can be considered the short version of `[](auto&& x){ return x*x;}`.

Can we bind the value if we have already extract them? Sure, **Identifier Pattern** is for you.

Let's log the square result, with Identifier Pattern the codes would be

```C++
#include <iostream>
#include "matchit.h"

bool checkAndlogLarge(double value)
{
    using namespace matchit;
    Id<double> s;
    return match(value)(
        pattern | app(_ * _, s.at(_ > 1000)) = [&] {
                std::cout << value << "^2 = " << *s << " > 1000!" << std::endl;
                return true; },
        pattern | _ = expr(false));
}
```

To use Identifier Patterns,  **we need to define/declare the identifiers (`Id<double> s`) first**. (Do not mark it as const.)
This can be a little strange if you've use Identifier Patterns in other programming language. This is due to the language restriction.
But don't be upset. This added verbosity makes it possible for us to **use variables inside patterns**. You may never be able to do this in other programming language.

Here `*` operator is used to dereference the value inside identifiers.
One thing to note is that identifiers are only valid inside `match` scope. Do not try to dereference it outside.

`Id::at` is similar to the **`@` pattern** in Rust, i.e., bind the value when the subpattern gets matched.

Also note when the same identifier is bound multiple times, the bound values must equal to each other via `operator==`.
An sample to check if an array is symmetric:

```C++
#include "matchit.h"
constexpr bool symmetric(std::array<int32_t, 5> const& arr)
{
    using namespace matchit;
    Id<int32_t> i, j; 
    return match(arr)(
        pattern | ds(i, j, _, j, i) = expr(true),
        pattern | _                 = expr(false)
    );
}

static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 7, 10}) == false);
static_assert(symmetric(std::array<int32_t, 5>{5, 0, 3, 0, 5}) == true);
static_assert(symmetric(std::array<int32_t, 5>{5, 1, 3, 0, 5}) == false);
```

### Hello Mars!

Now we come to the most powerful parts: **Destructure Pattern**.
Destructure Pattern can be used for `std::tuple`, `std::pair`, `std::array` (fixed-sized containers), and dynamic containers or sized ranges(`std::vector`, `std::list`, `std::set`, and so on) with `std::begin` and `std::end` supports. 

The outermost `ds` inside pattern can be omitted. When pattern receives multiple parameters, they are treated as subpatterns of a ds pattern.

```C++
#include "matchit.h"

template<typename T1, typename T2>
constexpr auto eval(std::tuple<char, T1, T2> const& expr)
{
    using namespace matchit;
    Id<T1> i;
    Id<T2> j;
    return match(expr)(
        pattern | ds('+', i, j) = i + j,
        pattern | ds('-', i, j) = i - j,
        pattern | ds('*', i, j) = i * j,
        pattern | ds('/', i, j) = i / j,
        pattern | _ = []
        {
            assert(false);
            return -1;
        });
}
```

Some operators have been overloaded for `Id`, so `i + j` will return a nullary function that return the value of `*i + *j`.

There also ways to destructure your struct / class, make your struct / class tuple-like or adopt App Pattern.
The second option looks like

```C++
// Another option to destructure your struct / class.
constexpr auto dsByMember(DummyStruct const&v)
{
    using namespace matchit;
    // compose patterns for destructuring struct DummyStruct.
    constexpr auto dsA = dsVia(&DummyStruct::size, &DummyStruct::name);
    Id<char const*> name;
    return match(v)(
        pattern | dsA(2, name) = expr(name),
        pattern | _ = expr("not matched")
    );
};

static_assert(dsByMember(DummyStruct{1, "123"}) == std::string_view{"not matched"});
static_assert(dsByMember(DummyStruct{2, "123"}) == std::string_view{"123"});
```

Let's continue the journey. 
Sometimes you have multiple identifiers and you want exert a restriction on the relationship of them. Is that possible?
Sure! Here comes the **Match Guard**. Its syntax is

```C++
pattern | PATTERN | when(GUARD) = HANDLER
```

Say, we want to match only when the sum of two identifiers equal to some value, we can write codes as

```C++
#include <array>
#include "matchit.h"

constexpr bool sumIs(std::array<int32_t, 2> const& arr, int32_t s)
{
    using namespace matchit;
    Id<int32_t> i, j;
    return match(arr)(
        pattern | ds(i, j) | when(i + j == s) = expr(true),
        pattern | _                           = expr(false));
}

static_assert(sumIs(std::array<int32_t, 2>{5, 6}, 11));
```

That is cool, isn't it?
Note that `i + j == s` will return a nullary function that return the result of `*i + *j == s`.

Now we come to the **Ooo Pattern**. What is that? You may ask. In some programming language it's called **Rest Pattern**.
You can match arbitrary number of items with it. It can only be used inside `ds` patterns though and at most one Ooo pattern can appear inside a `ds` pattern.
You can write the code as following when you want to check pattern of a tuple.

```C++
#include <array>
#include "matchit.h"

template <typename Tuple>
constexpr int32_t detectTuplePattern(Tuple const& tuple)
{
    using namespace matchit;
    return match(tuple)
    (
        pattern | ds(2, ooo, 2)  = expr(4),
        pattern | ds(2, ooo   )  = expr(3),
        pattern | ds(ooo, 2   )  = expr(2),
        pattern | ds(ooo      )  = expr(1)
    );
}

static_assert(detectTuplePattern(std::make_tuple(2, 3, 5, 7, 2)) == 4);
```

What is more, we can **bind a subrange to the ooo pattern** when destructuring a `std::array` or other containers / ranges.
That is quite cool.
We can check if an/a `array/vector/list/set/map/subrange/...` is symmetric with:

```C++
template <typename Range>
constexpr bool recursiveSymmetric(Range const &range)
{
    Id<int32_t> i;
    Id<SubrangeT<Range const>> subrange;
    return match(range)(
        pattern | ds(i, subrange.at(ooo), i) = [&] { return recursiveSymmetric(*subrange); },
        pattern | ds(_, ooo, _)              = expr(false),
        pattern | _                          = expr(true)
    );
```

In the first pattern, we require that the head equals to the end. and if that is the case, we further check the rest parts (bound to subrange) via a recursive call.
Once some nested call fails to meet that requirement (fall through to the second pattern), the checking fails.
Otherwise when there are only one element left or the range size is zero, the last pattern gets matched, we return true.

### Hello Sun!

We've done with our core patterns. Now let's start the journey of **composing patterns**.

You should be familiar with **Some Pattern** and **None Pattern** if you have used the pattern matching feature in Rust.

Some / None Patterns can be used to match raw pointers, `std::optional`, `std::unique_ptr`, `std::shared_ptr` and other types that can be converted to bool and dereferenced.
A typical sample can be

```C++
#include "matchit.h"

template <typename T>
constexpr auto square(std::optional<T> const& t)
{
    using namespace matchit;
    Id<T> id;
    return match(t)(
        pattern | some(id) = id * id,
        pattern | none     = expr(0));
}
constexpr auto x = std::make_optional(5);
static_assert(square(x) == 25);
```

Some pattern accepts a subpattern. In the sample the subpattern is an identifier and we bind the dereferenced result to it.
None pattern is alone.

Some and none patterns are not atomic patterns in `match(it)`, they are composed via

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

For Some pattern, first we cast the value to a boolean value, if the boolean value is true, we can further dereference it. Otherwise, the match fails.
For None pattern we simply check if the converted boolean value is false.

**As pattern** is very useful for handling `sum type`, including class hierarchies, `std::variant`, and `std::any`.
`std::variant` and `std::any` can be visited as

```C++
#include "matchit.h"
template <typename T>
constexpr auto getClassName(T const& v)
{
    using namespace matchit;
    return match(v)(
        pattern | as<char const*>(_) = expr("chars"),
        pattern | as<int32_t>(_)     = expr("int32_t")
    );
}

constexpr std::variant<int32_t, char const*> v = 123;
static_assert(getClassName(v) == std::string_view{"int32_t"});
```

Class hierarchies can be matched as

```C++
struct Shape
{
    virtual ~Shape() = default;
};
struct Circle : Shape {};
struct Square : Shape {};

auto getClassName(Shape const &s)
{
    return match(s)(
        pattern | as<Circle>(_) = expr("Circle"),
        pattern | as<Square>(_) = expr("Square")
    );
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

For classes, `dynamic_cast` is used by default for As pattern, but we can change the behavior through the **Customization Point**.
Users can customize the down casting via defining a `get_if` function for their classes, similar to `std::get_if` for `std::variant`:

```C++
#include <iostream>
#include "matchit.h"

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
    Kind kind() const override { return k; }
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
auto get_if(Num const* num) {
  return static_cast<T const *>(num->kind() == T::k ? num : nullptr);
}


int32_t staticCastAs(Num const& input)
{
    using namespace matchit;
    return match(input)(
        pattern | as<One>(_)       = expr(1),
        pattern | kind<Kind::kTWO> = expr(2),
        pattern | _                = expr(3));
}

int32_t main()
{
    std::cout << staticCastAs(One{}) << std::endl;
    return 0;
}
```

### Hello Milky Way!

There is additional **Customziation Point**.

Users can specialize `PatternTraits` if they want to add a brand new pattern.

### Hello Black Hole!

One thing to note is that `Id` is not a plain type. Any copies of it are just references to it.
So do not try to return it from where it is defined.

A bad case would be

```c++
auto badId()
{
    Id<int> x;
    return x;
}
```

Returning a composed pattern including a local `Id` is also incorrect.

```c++
auto badPattern()
{
    Id<int> x;
    return composeSomePattern(x);
}
```

Good practice is to define the `Id` close to its usage in pattern matching.
```c++
auto goodPattern()
{
    Id<int> x;
    auto somePattern = composeSomePattern(x);
    return match(...)
    (
        pattern | somePattern = ...
    );
}
```

## Real world use case

[`mathiu`](https://github.com/BowenFu/mathiu.cpp) is a simple computer algebra system built upon `match(it)`.

A simple sample of `mathiu`:

```C++
auto const x = symbol("x");
auto const e = x ^ fraction(2, 3);
auto const d = diff(e, x);
// prints (* 2/3 (^ x -1/3))
std::cout << toString(d) << std::endl;
```

## Projects using this library

[opennask](https://github.com/HobbyOSs/opennask) : An 80x86 assembler like MASM/NASM for the tiny OS.


If you are aware of other projects using this library, please let me know by submitting an issue or an PR.

## Contact

If you have any questions or ideas regarding the library, please [open an issue](https://github.com/bowenfu/matchit.cpp/issues/new/choose).

Discussions / issues / PRs are all welcome.

## Related Work

`match(it)`'s syntax / pattern designs have been heavily influenced by these related work

- [mpark/patterns](https://github.com/mpark/patterns)
- [Racket Pattern Matching](https://docs.racket-lang.org/reference/match.html)
- [Rust Patterns](https://doc.rust-lang.org/stable/reference/patterns.html)
- [jbandela/simple_match](https://github.com/jbandela/simple_match/)
- [solodon4/Mach7](https://github.com/solodon4/Mach7)
- [C++ Pattern Matching Proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1371r3.pdf)

## Other Work

If you are interested in `match(it)`, you may also be interested in [hspp](https://github.com/BowenFu/hspp) which brings Haskell style programming to C++.

## Support this library

Please star the repo, share the repo, or sponsor one dollar to let me know this library matters.

## Sponsor(s)

Thanks to @[e-dant](https://github.com/e-dant) for sponsoring this project.

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=bowenfu/matchit.cpp&type=Date)](https://star-history.com/#bowenfu/matchit.cpp&Date)
