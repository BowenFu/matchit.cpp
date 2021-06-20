# match(it): A lightweight header-only pattern-matching library for C++17 with macro-free APIs.

![match(it).cpp](./matchit.cpp.svg)

Here we will give equivalent code sinppets using `match(it)` for most samples given by [Rust Reference Patterns Section](https://doc.rust-lang.org/stable/reference/patterns.html). 

### Literal patterns

In Rust:

```Rust
for i in -2..5 {
    match i {
        -1 => println!("It's minus one"),
        1 => println!("It's a one"),
        2|4 => println!("It's either a two or a four"),
        _ => println!("Matched none of the arms"),
    }
}
```

In C++ with `match(it)`:

```C++
for (auto i = -2; i <= 5; ++i)
{
    std::cout <<
        match(i)(
            pattern(-1 )      = expr("It's minus one"),
            pattern(1  )      = expr("It's a one"),
            pattern(or_(2,4)) = expr("It's either a two or a four"),
            pattern(_  )      = expr("Matched none of the arms")
        )
        << std::endl;
}
```

### Identifier patterns

In Rust:

```Rust
let x = 2;

match x {
    e @ 1 ..= 5 => println!("got a range element {}", e),
    _ => println!("anything"),
}
```

In C++ with `match(it)`:

```C++
constexpr auto x = 2;
Id<int32_t> e;
match(x)(
    pattern(e.at(1 <= _ && _ <= 5)) = [&] { std::cout << "got a range element " << *e << std::endl; },
    pattern(_                     ) = [&] { std::cout << "anything" << std::endl; }
);
```

In Rust:

```Rust
struct Person {
   name: String,
   age: u8,
}
let value = Person { name: String::from("John"), age: 23 };
if let Person { name: ref person_name, age: 18..=150 } = value { }
```

In C++ with `match(it)`:

```C++
struct Person {
   std::string name;
   uint8_t age;
};

auto const value = Person{"John", 23};
auto const name_age = [](auto name_pat, auto age_pat)
{
    return and_(app(&Person::name, name_pat), app(&Person::age, age_pat));
};
Id<std::string> person_name;
match(value)(
    pattern(name_age(person_name, 18 <= _ && _ <= 150)) = []{}
);
```

In Rust:

```Rust
let x: &Option<i32> = &Some(3);
if let Some(y) = x {
    // y was converted to `ref y` and its type is &i32
}
```

In C++ with `match(it)`:

```C++
constexpr auto x = std::make_optional(3);
Id<int32_t> y;
match(x)(
    // No need to worry about y's type, by ref or by value is automatically managed by `match(it)` library.
    pattern(some(y)) = []{}
);
```

In Rust:

```Rust
// `name` is moved from person and `age` referenced
let Person { name, ref age } = person;
```

In C++ with `match(it)`:

```C++
Id<std::string> person_name;
Id<uint8_t> age;
match(std::move(value))
(
    // `name` is moved from person and `age` copied (scalar types are copied in `match(it)`)
    pattern(name_age(person_name, age)) = []{}
);
```

### Wildcard pattern

In Rust:

```Rust
let x = 20;
let (a, _) = (10, x);   // the x is always matched by _
assert_eq!(a, 10);

// ignore a function/closure param
let real_part = |a: f64, _: f64| { a };

// ignore a field from a struct
struct RGBA {
   r: f32,
   g: f32,
   b: f32,
   a: f32,
}
let color = RGBA{r: 0.4, g: 0.1, b: 0.9, a: 0.5};
let RGBA{r: red, g: green, b: blue, a: _} = color;
assert_eq!(color.r, red);
assert_eq!(color.g, green);
assert_eq!(color.b, blue);

// accept any Some, with any value
let x = Some(10);
if let Some(_) = x {}
```

In C++ with `match(it)`:

```C++
constexpr auto x = 20;
Id<int32_t> a;
match(10, x)(
    // the x is always matched by _
    pattern(a, _) = []{ assert(*a == 10;); }
);
// ignore a function/closure param
constexpr auto real_part = [](float a, float) { return a; };

// ignore a field from a struct
struct RGBA {
   float r;
   float g;
   float b;
   float a;
};

constexpr auto color = RGBA{0.4f, 0.1f, 0.9f, 0.5f};
constexpr auto dsRGBA = [](auto r, auto g, auto b, auto a)
{
    return and_(app(&RGBA::r, r),
                app(&RGBA::g, g),
                app(&RGBA::b, b),
                app(&RGBA::a, a)
                );
};

Id<float> red, green, blue;
match(color)(
    pattern(dsRGBA(red, green, blue, _)) = [&]{
        assert(color.r == *red);
        assert(color.g == *green);
        assert(color.b == *blue);
    }
);

// accept any Some, with any value
constexpr auto x = std::make_optional(10);
match(x)(
    // the x is always matched by _
    pattern(some(_)) = []{}
);
```

### Rest patterns

In Rust:

```Rust
let words = vec!["a", "b", "c"];
let slice = &words[..];
match slice {
    [] => println!("slice is empty"),
    [one] => println!("single element {}", one),
    [head, tail @ ..] => println!("head={} tail={:?}", head, tail),
}

match slice {
    // Ignore everything but the last element, which must be "!".
    [.., "!"] => println!("!!!"),

    // `start` is a slice of everything except the last element, which must be "z".
    [start @ .., "z"] => println!("starts with: {:?}", start),

    // `end` is a slice of everything but the first element, which must be "a".
    ["a", end @ ..] => println!("ends with: {:?}", end),

    rest => println!("{:?}", rest),
}

if let [.., penultimate, _] = slice {
    println!("next to last is {}", penultimate);
}

let tuple = (1, 2, 3, 4, 5);
// Rest patterns may also be used in tuple and tuple struct patterns.
match tuple {
    (1, .., y, z) => println!("y={} z={}", y, z),
    (.., 5) => println!("tail must be 5"),
    (..) => println!("matches everything else"),
}
```

In C++ with `match(it)`:

```C++
auto const words = std::vector<std::string>{"a", "b", "c"};
auto const& slice = words;
Id<std::string> head;
Id<SubrangeT<std::vector<std::string> const>> tail;
match(slice)(
    pattern(ds()                  )  = [&] { std::cout << "slice is empty" << std::endl; },
    pattern(ds(head)              )  = [&] { std::cout << "single element " << *head << std::endl; },
    // need to implement << for subrange tail
    pattern(ds(head, tail.at(ooo)))  = [&] { std::cout << "head=" << *head << " tail=" << *tail << std::endl; }
);

Id<SubrangeT<std::vector<std::string> const>> subrange;
match(slice)( 
    // Ignore everything but the last element, which must be "!".
    pattern(ds(ooo, "!")) = [&]{ std::cout << "!!!" << std::endl; },

    // `subrange` is a slice of everything except the last element, which must be "z".
    pattern(ds(subrange.at(ooo), "z")) = [&]{ std::cout << "starts with: " << *subrange << std::endl; },

    // `subrange` is a slice of everything but the first element, which must be "a".
    pattern(ds("a", subrange.at(ooo))) = [&]{ std::cout << "ends with: " << *subrange << std::endl; },

    pattern(ds(subrange.at(ooo))) = [&]{ std::cout << *subrange << std::endl; }
);

Id<std::string> penultimate;
match(slice)(
    pattern(ooo, penultimate, _) = [&] { std::cout << "next to last is " << *penultimate << std::endl; }
);

constexpr auto tuple = std::make_tuple(1, 2, 3, 4, 5);
// Rest patterns may also be used in tuple and tuple struct patterns.
Id<int32_t> y, z;
match(tuple)(  
    pattern(1, ooo, y, z) = [&] { std::cout << "y=" << *y << "z=" << *z << std::endl; },
    pattern(ooo, 5)       = [&] { std::cout << "tail must be 5" << std::endl; },
    pattern(ooo)          = [&] { std::cout << "matches everything else" << std::endl; }
);
```

### Range patterns

In Rust:

```Rust
let c = 'f';
let valid_variable = match c {
    'a'..='z' => true,
    'A'..='Z' => true,
    'α'..='ω' => true,
    _ => false,
};

let ph = 10;
println!("{}", match ph {
    0..=6 => "acid",
    7 => "neutral",
    8..=14 => "base",
    _ => unreachable!(),
});

// using paths to constants:
const TROPOSPHERE_MIN : u8 = 6;
const TROPOSPHERE_MAX : u8 = 20;

const STRATOSPHERE_MIN : u8 = TROPOSPHERE_MAX + 1;
const STRATOSPHERE_MAX : u8 = 50;

const MESOSPHERE_MIN : u8 = STRATOSPHERE_MAX + 1;
const MESOSPHERE_MAX : u8 = 85;

let altitude = 70;

println!("{}", match altitude {
    TROPOSPHERE_MIN..=TROPOSPHERE_MAX => "troposphere",
    STRATOSPHERE_MIN..=STRATOSPHERE_MAX => "stratosphere",
    MESOSPHERE_MIN..=MESOSPHERE_MAX => "mesosphere",
    _ => "outer space, maybe",
});

pub mod binary {
    pub const MEGA : u64 = 1024*1024;
    pub const GIGA : u64 = 1024*1024*1024;
}
let n_items = 20_832_425;
let bytes_per_item = 12;
if let size @ binary::MEGA..=binary::GIGA = n_items * bytes_per_item {
    println!("It fits and occupies {} bytes", size);
}

trait MaxValue {
    const MAX: u64;
}
impl MaxValue for u8 {
    const MAX: u64 = (1 << 8) - 1;
}
impl MaxValue for u16 {
    const MAX: u64 = (1 << 16) - 1;
}
impl MaxValue for u32 {
    const MAX: u64 = (1 << 32) - 1;
}
// using qualified paths:
println!("{}", match 0xfacade {
    0 ..= <u8 as MaxValue>::MAX => "fits in a u8",
    0 ..= <u16 as MaxValue>::MAX => "fits in a u16",
    0 ..= <u32 as MaxValue>::MAX => "fits in a u32",
    _ => "too big",
});
```

In C++ with `match(it)`:

```C++
constexpr auto c = 'f';
constexpr auto valid_variable = match(c)( 
    pattern('a' <= _ && _ <= 'z') = expr(true),
    pattern('A' <= _ && _ <= 'Z') = expr(true),
    pattern('α' <= _ && _ <= 'ω') = expr(true),
    pattern(_)                    = expr(false)
);

constexpr auto ph = 10;
std::cout << match(ph)( 
    pattern(0 <= _ && _ <= 6 ) = expr("acid"),
    pattern(7     )            = expr("neutral"),
    pattern(8 <= _ && _ <= 14) = expr("base"),
    pattern(_)                 = [] { assert(false && "unreachable"); }
) << std::endl;

// using paths to constants:
constexpr uint8_t TROPOSPHERE_MIN = 6;
constexpr uint8_t TROPOSPHERE_MAX = 20;

constexpr uint8_t STRATOSPHERE_MIN = TROPOSPHERE_MAX + 1;
constexpr uint8_t STRATOSPHERE_MAX = 50;

constexpr uint8_t MESOSPHERE_MIN = STRATOSPHERE_MAX + 1;
constexpr uint8_t MESOSPHERE_MAX = 85;

constexpr auto altitude = 70;

std::cout << match(altitude)( 
    pattern(TROPOSPHERE_MIN  <= _ && _ <= TROPOSPHERE_MAX)  = expr("troposphere"),
    pattern(STRATOSPHERE_MIN <= _ && _ <= STRATOSPHERE_MAX) = expr("stratosphere"),
    pattern(MESOSPHERE_MIN   <= _ && _ <= MESOSPHERE_MAX)   = expr("mesosphere"),
    pattern(_)                                              = expr("outer space, maybe")
) << std::endl;

namespace binary
{
    constexpr uint64_t MEGA = 1024*1024;
    constexpr uint64_t GIGA = 1024*1024*1024;
} // namespace binary 

constexpr auto n_items = 20'832'425U;
constexpr auto bytes_per_item = 12U;

Id<uint64_t> size;
match(n_items * bytes_per_item)
(
    pattern(size.at(binary::MEGA <= _ && _ <= binary::GIGA)) = [&] {
        std::cout << "It fits and occupies " << size << " bytes" << std::endl;
    }
);

// using qualified paths:
std::cout << match(static_cast<size_t>(0xfacade))( 
    pattern(0U <= _ && _ <= numeric_limits<uint8_t>::max())  = expr("fits in a u8"),
    pattern(0U <= _ && _ <= numeric_limits<uint16_t>::max()) = expr("fits in a u16"),
    pattern(0U <= _ && _ <= numeric_limits<uint32_t>::max()) = expr("fits in a u32"),
    pattern(_)                                               = expr("too big")
) << std::endl;
```

Tips: feel free to use variables in `match(it)`. You can write codes like

```C++
// using variables:
std::cout << match(0xfacade)( 
    pattern(min(a, b) <= _ && _ <= max(a, b))  = expr("fits in the range"),
    pattern(_)                                 = expr("out of the range")
) << std::endl;
```

### Reference patterns

In Rust:

```Rust
let int_reference = &3;

let a = match *int_reference { 0 => "zero", _ => "some" };
let b = match int_reference { &0 => "zero", _ => "some" };

assert_eq!(a, b);
```

In C++ with `match(it)`:

```C++
int32_t const value = 3;
int32_t const *int_reference = &value;

int32_t const zero = 0;

auto const a = match(*int_reference)(
    pattern(zero) = expr("zero"),
    pattern(_)    = expr("some"));

auto const b = match(int_reference)(
    pattern(&zero) = expr("zero"),
    pattern(_)     = expr("some"));

assert(a == b);
```

### Struct patterns

In Rust:

```Rust
struct Point {
    x: u32,
    y: u32,
}
let s = Point {x: 1, y: 1};

match s {
    Point {x: 10, y: 20} => (),
    Point {y: 10, x: 20} => (),    // order doesn't matter
    Point {x: 10, ..} => (),
    Point {..} => (),
}

struct PointTuple (
    u32,
    u32,
);
let t = PointTuple(1, 2);

match t {
    PointTuple {0: 10, 1: 20} => (),
    PointTuple {1: 10, 0: 20} => (),   // order doesn't matter
    PointTuple {0: 10, ..} => (),
    PointTuple {..} => (),
}
```

In C++ with `match(it)`:

```C++
#include <iostream>
#include "matchit.h"
using namespace matchit;

template <size_t I>
constexpr auto get = [](auto &&pt)
{
    return std::get<I>(pt);
};

template <size_t I>
constexpr auto appGet = [](auto &&pat)
{
    return app(get<I>, pat);
};

void sample()
{
    struct Point
    {
        uint32_t x;
        uint32_t y;
    };
    constexpr auto s = Point{1U, 1U};

    match(s)(
        pattern(and_(app(&Point::x, 10U), app(&Point::y, 20U))) = []{},
        pattern(and_(app(&Point::y, 10U), app(&Point::x, 20U))) = []{},    // order doesn't matter
        pattern(app(&Point::x, 10U))                            = []{},
        pattern(_)                                              = []{}
    );

    using PointTuple = std::tuple<uint32_t, uint32_t>;
    constexpr auto t = PointTuple{1U, 2U};

    match(t)(
        pattern(and_(appGet<0>(10U), appGet<1>(20U))) = []{},
        pattern(and_(appGet<1>(10U), appGet<0>(20U))) = []{},   // order doesn't matter
        pattern(appGet<0>(10U))                      = []{},
        pattern(_)                                  = []{}
    );
}
```

### Tuple struct patterns

In Rust:

```Rust
let pair = (10, "ten");
let (a, b) = pair;

assert_eq!(a, 10);
assert_eq!(b, "ten");
```

In C++ with `match(it)`:

```C++
constexpr auto pair = std::make_pair(10, "ten");
Id<int32_t> a;
Id<char const*> b;
match(pair)(
    pattern(a, b) = [&]{
        assert(*a == 10);
        assert(*b == "ten");
    }
);
```

### Slice patterns

In Rust:

```Rust
// Fixed size
let arr = [1, 2, 3];
match arr {
    [1, _, _] => "starts with one",
    [a, b, c] => "starts with something else",
};
```

In C++ with `match(it)`:

```C++
// Fixed size
constexpr auto arr = std::array<int32_t, 3>{1, 2, 3};
Id<int32_t> a, b, c;
match(arr)( 
    pattern(1, _, _) = expr("starts with one"),
    pattern(a, b, c) = expr("starts with something else")
);
```

In Rust:

```Rust
// Dynamic size
let v = vec![1, 2, 3];
match v[..] {
    [a, b] => { /* this arm will not apply because the length doesn't match */ }
    [a, b, c] => { /* this arm will apply */ }
    _ => { /* this wildcard is required, since the length is not known statically */ }
};
```

In C++ with `match(it)`:

```C++
// Dynamic size
auto const v = std::vector<int32_t>{1, 2, 3};
Id<int32_t> a, b, c;
match(v)( 
    pattern(a, b)    = [] { /* this arm will not apply because the length doesn't match */ },
    pattern(a, b, c) = [] { /* this arm will apply */ },
    pattern(_)       = [] { /* this wildcard is required, since the length is not known statically */ }
);
```
