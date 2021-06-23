# match(it): A lightweight header-only pattern-matching library for C++17 with macro-free APIs.

![match(it).cpp](./matchit.cpp.svg)

## P1371R3

### Matching Integrals

In `P1371R3`:

```C++
inspect (x) {
    0 => { std::cout << "got zero"; }
    1 => { std::cout << "got one"; }
    __ => { std::cout << "don't care"; }
};
```

In `match(it)`:

```C++
match(x) ( 
    pattern(0) = [&]{ std::cout << "got zero"; },
    pattern(1) = [&]{ std::cout << "got one"; },
    pattern(_) = [&]{ std::cout << "don't care"; },
);
```

### Matching Strings

In `P1371R3`:

```C++
inspect (s) {
    "foo" => { std::cout << "got foo"; }
    "bar" => { std::cout << "got bar"; }
    __ => { std::cout << "don't care"; }
};
```

In `match(it)`:

```C++
match(s)( 
    pattern("foo") = [&]{ std::cout << "got foo"; },
    pattern("bar") = [&]{ std::cout << "got bar"; },
    pattern(_)     = [&]{ std::cout << "don't care"; }
);
```

### Matching Tuples

In `P1371R3`:

```C++
inspect(p) {
    [0, 0] => { std::cout << "on origin"; }
    [0, y] => { std::cout << "on y-axis"; }
    [x, 0] => { std::cout << "on x-axis"; }
    [x, y] => { std::cout << x << ',' << y; }
};
```

In `match(it)`:

```C++
Id<int32_t> x, y;
match(p) {
    pattern(ds(0, 0)) = [&]{ std::cout << "on origin"; },
    pattern(ds(0, y)) = [&]{ std::cout << "on y-axis"; },
    pattern(ds(x, 0)) = [&]{ std::cout << "on x-axis"; },
    pattern(ds(x, y)) = [&]{ std::cout << *x << ',' << *y; }
};
```

### Matching Variants

In `P1371R3`:

```C++
inspect (v) {
    <int> i => {
        strm << "got int: " << i;
    }
    <float> f => {
        strm << "got float: " << f;
    }
};
```

In `match(it)`:

```C++
Id<int> i;
Id<float> f;
match(v) ( 
    pattern(as<int>(i)) = [&]{  
        strm << "got int: " << *i;
    },
    pattern(as<float>(f)) = [&]{  
        strm << "got float: " << *f;
    }
);
```

### Matching Polymorphic Types

```C++
struct Shape { virtual ~Shape() = default; };
struct Circle : Shape { int radius; };
struct Rectangle : Shape { int width, height; };
```

In `P1371R3`:

```C++
int get_area(const Shape& shape)
{
    return inspect (shape) {
        <Circle> [r] => 3.14 * r * r;
        <Rectangle> [w, h] => w * h;
    };
}
```

In `match(it)`:

```C++
int get_area(const Shape& shape)
{
    return match(shape) ( 
        // need to implement get for the structs.
        pattern(as<Circle>(ds(r)))       = 3.14 * r * r,
        pattern(as<Rectangle>(ds(w, h))) = w * h
    );
}
```

### Evaluating Expression Trees

```C++
struct Expr;
struct Neg { std::shared_ptr<Expr> expr;
};
struct Add { std::shared_ptr<Expr> lhs, rhs;
};
struct Mul { std::shared_ptr<Expr> lhs, rhs;
};
struct Expr : std::variant<int, Neg, Add, Mul> {
  using variant::variant;
};
namespace std {
template <>
struct variant_size<Expr> : variant_size<Expr::variant> {};
template <std::size_t I>
struct variant_alternative<I, Expr> : variant_alternative<I, Expr::variant> {}; }
```

In `P1371R3`:

```C++
int eval(const Expr& expr) {
  return inspect (expr) {
    <int> i => i;
    <Neg> [(*?) e] => -eval(e);
    <Add> [(*?) l, (*?) r] => eval(l) + eval(r); // Optimize multiplication by 0.
    <Mul> [(*?) <int> 0, __] => 0;
    <Mul> [__, (*?) <int> 0] => 0;
    <Mul> [(*?) l, (*?) r] => eval(l) * eval(r);
}; }
```

In `match(it)`:

```C++
int eval(const Expr& expr){
  return inspect (expr) ( 
    pattern(as<int>(i))                     = i,
    pattern(as<Neg>(ds((*?) e)))            = [&]{ return -eval(*e); },
    pattern(as<Add>(ds((*?) l, (*?) r)))    = [&]{ return eval(*l) + eval(*r); },
    // Optimize multiplication by 0.
    pattern(as<Mul>(ds((*?) <int> 0, __)))  = expr(0),
    pattern(as<Mul>(ds(__, (*?) <int> 0)))  = expr(0),
    pattern(as<Mul>(ds((*?) l, (*?) r)))    = [&]{ return eval(*l) * eval(*r); }
  );
}
```

### Terminate from Inspect

In `P1371R3`:

```C++
enum class Op { Add, Sub, Mul, Div };
Op parseOp(Parser& parser) {
    return inspect (parser.consumeToken()) {
        '+' => Op::Add;
        '-' => Op::Sub;
        '*' => Op::Mul;
        '/' => Op::Div;
        token => !{
            std::cerr << "Unexpected: " << token;
            std::terminate();
        }
    };
}
```

```C++
enum class Op { Add, Sub, Mul, Div };
Op parseOp(Parser& parser) {
    Id<char> token;
    return match(parser.consumeToken()) ( 
        pattern('+') = expr(Op::Add),
        pattern('-') = expr(Op::Sub),
        pattern('*') = expr(Op::Mul),
        pattern('/') = expr(Op::Div),
        pattern(token) => [&]{
            std::cerr << "Unexpected: " << *token;
            std::terminate();
        }
    );
}
```


