# match(it): A light-weight header-only pattern-matching library for C++17.
![match(it).cpp](./matchit.cpp.svg)

## Related Work
`match(it)` is influenced by multiple related work, programming languages or libraries

- [mpark/patterns](https://github.com/mpark/patterns)
- [Racket Pattern Matching](https://docs.racket-lang.org/reference/match.html)
- [Rust Patterns](https://doc.rust-lang.org/stable/reference/patterns.html)
- [jbandela/simple_match](https://github.com/jbandela/simple_match/)
- [solodon4/Mach7](https://github.com/solodon4/Mach7)
- [C++ Pattern Matching Proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1371r3.pdf)

## The basic syntax
```C++
match(VALUE)
(
    pattern(PATTERN1) = HANDLER1,
    pattern(PATTERN2) = HANDLER2,
    ...
)
```
is borrowed from `mpark/patterns`.
The keyword `match` is widely used for pattern matching. We cannot wrap patterns with `{}` unless we make `match` a macro. I am not a fan of macros though.
The operator `=` between the pattern and handler can be any binary operators. It's `=>` in Rust, which is not existent inside C++. Other promising candidates can be `>=`, `<=`, `>>=`. We chose between `=` and `>>=` and `=` won due to its simplicity.
`pattern` keyword makes it easy to identify a `pattern - handler` pair. It is similar to `case` keyword for switch statements.

## Handler
Handlers should always be nullary functions. This is different from `mpark/patterns` and `jbandela/simple_match`.
`expr` can be called to return simple functions returning a single value.
`expr(value)` syntax is inspired by `Boost/Lambda` library.
`pattern(xxx) = expr(zzz)` or `pattern(xxx) = [&]{zzzzzz}` syntaxes can be aligned and it is easy to find out the pattern parts and handler parts.

## Expression Pattern
Since identifiers do not have special meanings inside `matchit`, they can be used inside patterns normally.
You can even use a function call as Expression Pattern, the result of the function call will be matched against the value.
```C++
match(map.find(key))(
    pattern(map.end()) = expr(false),
    pattern(_)         = expr(true)
)
```
This is different from most other related work.

## Wildcard Pattern
This pattern is common in lots of related work. We adopt the same symbol `_` in `matchit`.
Refer to the above code snippets.

## Predicate Pattern
This inspired by Racket Pattern Matching. It has a pattern `(? expr pat ...)`, which will check if `(expr value)` is true.
Predicate Pattern corresponds to `(? expr)`.
```C++
match(value)(
    pattern(meet([](auto &&v) { return v >= 0; })) = expr(value),
    pattern(_)                                     = expr(0))
```
Predicate Pattern syntax is `meet(predicate function)`.
`_` can be used to generate simple predicates via
```C++
match(value)(
    pattern(_ >= 0) = expr(value),
    pattern(_)      = expr(0))
```

## Or Pattern
Or Pattern is borrowed from Racket Pattern Matching.
The Racket syntax is `(or pat ...)`, and the corresponding C++ syntax is `or_(pat, ...)`. Note that `or` is a C++ keyword, we use `or_` instead.
```C++
match(n)(
    pattern(or_(1, 3, 5)) = expr(true),
    pattern(_)            = expr(false))
```
Note subpatterns of `or_` pattern can be any patterns, not just expression patterns.
Say Predicate Patterns
```C++
match(n)(
    pattern(or_(_ < 3, 5)) = expr(true),
    pattern(_)            = expr(false))
```
In Rust and some other related work, there exists a similar `anyof` pattern. But only literal patterns can be used as subpatterns.

## And Pattern
And Pattern is borrowed from Racket Pattern Matching as well.
The Racket syntax is `(and pat ...)`, and the corresponding C++ syntax is `and_(pat, ...)`. Note that `and` is a C++ keyword, we use `and_` instead.
```C++
match(value)(
    pattern(and_(_ >= min, _ <= max)) = expr(value),
    pattern(_ > max)                  = expr(max),
    pattern(_)                        = expr(min))
```
Note this can also be written as
```C++
match(value)(
    pattern(min <= _ && _ <= max) = expr(value),
    pattern(_ > max)              = expr(max),
    pattern(_)                    = expr(min))
```

But `&&` can only be used between Predicate patterns, while `and_` can be used for all kinds of patterns (except Ooo Pattern).

## Not Pattern
Not Pattern is borrowed from Racket Pattern Matching as well.
The Racket syntax is `(not pat ...)`, and the corresponding C++ syntax is `not_(pat)`. Note that `not` is a C++ keyword, we use `not_` instead.
`(not pat ...)` means `none of`, in `matchit` it can be written as `not_(or_(pat, ...))`.

## App Pattern
App Pattern is borrowed from Racket Pattern Matching as well.
The Racket syntax is `(app expr pats ...)`, and the corresponding C++ syntax is `app(expr, pat)`.
The result of invoking `expr`(a unary function) on the matching value will be further matched against `pat`.
That is to say, Predicate Pattern can be expressed with App Pattern, `meet(unary)` is equivalent to `app(unary, true)`. The decision on whether we should keep Predicate Pattern as an atomic pattern is not made. It is possible that we can move it to utility later.
```C++
match(value)(
    pattern(app(_ * _, _ > 1000)) = expr(true),
    pattern(_)                    = expr(false))
```

## Identifier Pattern
Identifier Pattern exists in most related works. But how to implement it in a library is an open question.
There is no way for us to use identifiers in match context now since it is not valid C++ syntax. (This also makes it possible for us to use variables / function calls as expression patterns.)
In `mpark/patterns` and `jbandela/simple_match` the bound values to identifiers will be forward to handlers as parameters. That is to say, identifiers are more like positioned parameters.
The design is not very natural.
We choose to make identifiers as binders that can be accessed inside handlers to align with other native pattern matching designs.
This means that handlers in `matchit` are always nullary, but can be unary or binary or consisting of more arguments in the other two library.
```C++
Id<double> s;
match(value)(
    pattern(app(_ * _, and_(_ > 1000, s))) = expr(s),
    pattern(_)                             = expr(0));
```
You have to define / declare the identifiers first then bind them inside patterns and access them in handlers.
`expr(s)` is a short for `[&]{ return *s; }`, i.e., a function returning the value bound to the identifier.