# match(it): A lightweight header-only pattern-matching library for C++17 with macro-free APIs.

![match(it).cpp](./matchit.cpp.svg)

## The basic syntax

```C++
match(VALUE)
(
    pattern | PATTERN1 = HANDLER1,
    pattern | PATTERN2 = HANDLER2,
    ...
)
```

is borrowed from `mpark/patterns`.
The keyword `match` is widely used for pattern matching. We cannot wrap patterns with `{}` unless we make `match` a macro. I am not a fan of macros though.
The operator `=` between the pattern and handler can be any binary operators. It's `=>` in Rust, which is not existent inside C++. Other promising candidates can be `>=`, `<=`, `>>=`. We chose between `=` and `>>=` and `=` won due to its simplicity.
`pattern` keyword makes it easy to identify a `pattern - handler` pair. It is similar to `case` keyword for switch statements.

### Handler

Handlers should always be nullary functions. This is different from `mpark/patterns` and `jbandela/simple_match`.
Expressions can be used as handlers as well, which are equivalent to nullary functions that return the same values.
Id instances used as handlers are equivalent to nullary functions that return the bound values.

## Pattern Primitives

### Expression Pattern

Since identifiers do not have special meanings inside `match(it)`, they can be used inside patterns normally.
You can even use a function call as Expression Pattern, the result of the function call will be matched against the value.

```C++
match(map.find(key))(
    pattern | map.end() = false,
    pattern | _         = true
)
```

This is different from most other related work.

### Wildcard Pattern

This pattern is common in lots of related work. We adopt the same symbol `_` in `match(it)`.
Refer to the above code snippets.

### Predicate Pattern

This inspired by Racket Pattern Matching. It has a pattern `(? expr pat ...)`, which will check if `(expr value)` is true.
Predicate Pattern corresponds to `(? expr)`.

```C++
match(value)(
    pattern | meet([](auto &&v { return v >= 0; })) = value,
    pattern | _                                     = 0
)
```

Predicate Pattern syntax is `meet(predicate function)`.
`_` can be used to generate simple predicates via

```C++
match(value)(
    pattern | (_ >= 0) = value,
    pattern | _        = 0
)
```

The short syntax is inspired by `jbandela/simple_match`.

### Identifier Pattern

Identifier Pattern exists in most related works. But how to implement it in a library is an open question.
There is no way for us to use identifiers in match context now since it is not valid C++ syntax. (This also makes it possible for us to use variables / function calls as expression patterns.)
In `mpark/patterns` and `jbandela/simple_match` the bound values to identifiers will be forward to handlers as parameters. That is to say, identifiers are more like positioned parameters.
The design is not very natural.
We choose to make identifiers as binders that can be accessed inside handlers to align with other native pattern matching designs.
This means that handlers in `match(it)` are always nullary, but can be unary or binary or consisting of more arguments in the other two library.

```C++
Id<double> s;
match(value)(
    pattern | app(_ * _, s) = s,
    pattern | _             = 0
);
```

You have to define / declare the identifiers first then bind them inside patterns and access them in handlers.

Identifier Pattern supports binding non-constructable (via reference), non-copyable (via reference or moving) types. This can be similar to "Ref pattern" or "Mut Ref pattern" in Rust.

Identifier Pattern requires `operator==` for the binding types.

### Match Guard

Match Guard exists in most related works. The current syntax is borrowed from `mpark/patterns`.
Match Guard can be used to exert extra restrictions on a pattern. The syntax is

```C++
pattern | PATTERN | when(PREDICATE) = HANDLER
```

A simple sample can be

```C++
bool flag = true;
return match(v)(
    pattern | 0 | when(flag) = true,
    pattern | _              = false
);
```

### Ooo Pattern

Ooo Pattern can match arbitrary number of items.
Similar patterns exist in most related works.
The current one is mostly influenced by `..` pattern in Rust. (Also inspired by Racket's `...`).
It can only be used inside Destructure Patterns and at most one Ooo pattern can appear inside one Destructure Pattern.
Refer to [Pattern Cominators / Destructure Pattern](#destructure-pattern).

```C++
match(tuple)
(
    pattern | ds(2, ooo, 2)  = 4,
    pattern | ds(2, ooo   )  = 3,
    pattern | ds(ooo, 2   )  = 2,
    pattern | ds(ooo      )  = 1
)
```

## Pattern Combinators

### Or Pattern

Or Pattern is borrowed from Racket Pattern Matching.
The Racket syntax is `(or pat ...)`, and the corresponding C++ syntax is `or_(pat, ...)`. Note that `or` is a C++ keyword, we use `or_` instead.

```C++
match(n)(
    pattern | or_(1, 3, 5) = true,
    pattern | _            = false)
```

Note subpatterns of `or_` pattern can be any patterns, not just expression patterns.
Say Predicate Patterns

```C++
match(n)(
    pattern | or_(_ < 3, 5) = true,
    pattern | _             = false
)
```

In Rust and some other related work, there exists a similar `anyof` pattern. But only literal patterns can be used as subpatterns.

### And Pattern

And Pattern is borrowed from Racket Pattern Matching as well.
The Racket syntax is `(and pat ...)`, and the corresponding C++ syntax is `and_(pat, ...)`. Note that `and` is a C++ keyword, we use `and_` instead.

```C++
match(value)(
    pattern | and_(_ >= min, _ <= max)) = value,
    pattern | (_ > max)                 = max,
    pattern | _                         = min
)
```

Note this can also be written as

```C++
match(value)(
    pattern | (min <= _ && _ <= max) = value,
    pattern | (_ > max)              = max,
    pattern | _                      = min
)
```

But `&&` can only be used between Predicate patterns, while `and_` can be used for all kinds of patterns (except Ooo Pattern).

### Not Pattern

Not Pattern is borrowed from Racket Pattern Matching as well.
The Racket syntax is `(not pat ...)`, and the corresponding C++ syntax is `not_(pat)`. Note that `not` is a C++ keyword, we use `not_` instead.
`(not pat ...)` means `none of`, in `match(it)` it can be written as `not_(or_(pat, ...))`.

### App Pattern

App Pattern is borrowed from Racket Pattern Matching as well.
The Racket syntax is `(app expr pats ...)`, and the corresponding C++ syntax is `app(expr, pat)`.
The result of invoking `expr`(a unary function) on the matching value will be further matched against `pat`.
That is to say, Predicate Pattern can be expressed with App Pattern, `meet(unary)` is equivalent to `app(unary, true)`. The decision on whether we should keep Predicate Pattern as an atomic pattern is not made. It is possible that we can move it to utility later.

```C++
match(value)(
    pattern | app(_ * _, _ > 1000) = true,
    pattern | _                    = false
)
```

### Destructure Pattern

The syntax is borrowed from `mpark/patterns`.

```C++
Id<T1> i;
Id<T2> j;
match(expr)(
    pattern | ds('+', i, j) = i + j,
    pattern | ds('-', i, j) = i - j,
    pattern | ds('*', i, j) = i * j,
    pattern | ds('/', i, j) = i / j,
    pattern | _             = -1
)
```

Note the outermost `ds` inside pattern can be saved. That is to say, when pattern receives multiple parameters, they are treated as subpatterns of a ds pattern.

```C++
Id<T1> i;
Id<T2> j;
match(expr)(
    pattern | ds('+', i, j) = i + j,
    pattern | ds('-', i, j) = i - j,
    pattern | ds('*', i, j) = i * j,
    pattern | ds('/', i, j) = i / j,
    pattern | _             = -1
)
```

We support Destructure Pattern for `std::tuple`, `std::pair`, `std::array`, and containers / ranges that can be called with `std::begin` and `std::end`.
Mismatch of element numbers is a compile error for fixed-size containers.
Mismatch of element numbers is just a mismatch for dynamic containers, neither a compile error, nor a runtime error.

There are also ways to destructure your struct / class, make your struct / class tuple-like or adopt App Pattern.
To achieve that, we need to define a `get` function for them inside the same namespace of the struct or the class. (`std::tuple_size` needs to be specialized as well.)
Refer to `samples/customDs.cpp` for more details.

The second option looks like

```C++
// Another option to destructure your struct / class.
constexpr auto dsViaMember(DummyStruct const&v)
{
    using namespace matchit;
    // compose patterns for destructuring struct DummyStruct.
    const auto dsA = dsVia(&DummyStruct::size, &DummyStruct::name);
    Id<char const*> name;
    return match(v)(
        pattern | dsA(2, name) = name,
        pattern | _ = "not matched"
    );
};

static_assert(dsViaMember(DummyStruct{1, "123"}) == std::string_view{"not matched"});
static_assert(dsViaMember(DummyStruct{2, "123"}) == std::string_view{"123"});
```

### At Pattern

At Pattern is similar to the `@` pattern in Rust. It can have one subpattern. The identifier will only be bound when the subpattern gets matched.

```C++
Id<double> s;
match(value)(
    pattern | app(_ * _, s.at(_ > 1000)) = s,
    pattern | _                          = 0
);
```

### At Pattern for Ooo Pattern

We support binding a subrange to the ooo pattern when destructuring a `std::array` or other containers / ranges.

```C++
Id<int32_t> i;
Id<SubrangeT<Range const>> subrange;
return match(range)(
    pattern | ds(i, subrange.at(ooo), i) = [&] { return recursiveSymmetric(*subrange); },
    pattern | ds(i, subrange.at(ooo), _) = false,
    pattern | _                          = true
);
```

## Predefined Composed Patterns

### Some / None Pattern

Some and None Patterns are composed patterns. The syntaxes are borrowed from `mpark/patterns`.
Their usage can be

```C++
match(t)(
    pattern | some(id) = id * id,
    pattern | none     = 0
);
```

### As Pattern

As Pattern is also a composed pattern. The syntax is borrowed from `mpark/patterns`.
It can be used to handle sum type, including class hierarchies, std::variant, and std::any. A simple sample can be

```C++
match(v)(
    pattern | as<char const*>(_) = "chars",
    pattern | as<int32_t>(_)     = "int32_t"
);
```

As Pattern can be customized for users' classes to override the dynamic cast as the default down casting via defining a `get_if` function for their structs / classes.
Refer to `samples/CustomAsPointer.cpp`.

## Customized Pattern

Users can define their Customized Pattern Primitives or Combinators via specializing `PatternTraits`.
