# Composable pattern objects are exactly what we need for pattern matching

We will present a brand new pattern matching library, focusing on the power of composable pattern objects: makes it easy for users to extend and can minimize the codes users need to write.

In our library [match(it)](https://github.com/BowenFu/matchit.cpp/blob/main/REFERENCE.md), we have 6 pattern primitives (`Expression Pattern`, `Wildcard Pattern`, `Predicate Pattern`, `Identifier Pattern`, `Match Guard`, `Ooo Pattern`) and 6 pattern combinators (`Or Pattern`, `And Pattern`, `Not Pattern`, `App Pattern`, `Destructure Pattern`, `At Pattern`).

Pattern primitives are all objects. And pattern combinators are callables that return new objects. Let’s call these objects `pattern objects`.

The most powerful two combinators can be `App Pattern` and `And Pattern`. Composing new patterns with them is much easier and cleaner than writing the `Matcher` and `Extractor` as described in the [current C++ Pattern Matching Proposal](https://wg21.link/P1371).

There are some predefined composed patterns: `Some / None Pattern`, and `As Pattern`.

`Some` can be simply defined as:
```
constexpr auto some = [](auto const pat) {
   return and_(app(cast<bool>, true), app(deref, pat));
};
```

The library covers all the pattern matching functionality described in the Proposal and also all functionality described in [Rust Reference Page](https://doc.rust-lang.org/stable/reference/patterns.html#literal-patterns).

We will give our suggestions on the C++ Pattern Matching Proposal based on our experience with our library.