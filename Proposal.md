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

becomes

```C++
inspect (v) {
    int i => {
        strm << "got int: " << i;
    }
    float f => {
        strm << "got float: " << f;
    }
};
```

```C++
int get_area(const Shape& shape)
{
    return inspect (shape) {
        <Circle> [r] => 3.14 * r * r;
        <Rectangle> [w, h] => w * h;
    };
}
```

becomes

```C++
int get_area(const Shape& shape)
{
    return inspect (shape) {
        Circle [r] => 3.14 * r * r;
        Rectangle [w, h] => w * h;
    };
}
```

```C++
int eval(const Expr& expr) {
  return inspect (expr) {
    <int> i => i;
    <Neg> [(*?) e] => -eval(e);
    <Add> [(*?) l, (*?) r] => eval(l) + eval(r);
    // Optimize multiplication by 0.
    <Mul> [(*?) <int> 0, __] => 0;
    <Mul> [__, (*?) <int> 0] => 0;
    <Mul> [(*?) l, (*?) r] => eval(l) * eval(r);
}; }
```

becomes

```C++
int eval(const Expr& expr) {
  return inspect (expr) {
    int i => i;
    Neg [some: e] => -eval(e);
    Add [some: l, some: l] => eval(l) + eval(r);
    // Optimize multiplication by 0.
    Mul [some: int 0, __] => 0;
    Mul [__, some: int 0] => 0;
    Mul [some: l, some: r] => eval(l) * eval(r);
}; }
```

```C++
static constexpr int zero = 0, one = 1;
int v = 42;
inspect (v) {
    zero => { std::cout << zero; }
//  ˆˆˆˆ identifier pattern
};
// prints: 42
```

becomes

```C++
static constexpr int zero = 0, one = 1;
int v = 42;
inspect (v) {
    +zero => { std::cout << zero; }
//  ˆˆˆˆ expression pattern
};
// prints: nothing
```

```C++
enum Color { Red, Green, Blue };
Color color = /* ... */;
inspect (color) {
    +Red => // ...
    +Green => // ...
//       ˆˆˆˆˆ id-expression
    +Blue => // ...
//  ˆˆˆˆˆˆˆˆˆ case pattern
};
```

becomes

```C++
static constexpr int zero = 0;
int v = /* ... */;
inspect (v) {
    case zero => { std::cout << "got zero"; }
//       ˆˆˆˆ id-expression
    case 1 => { std::cout << "got one"; }
//       ˆ expression pattern
    case 2 => { std::cout << "got two"; }
//  ˆˆˆˆˆˆ case pattern
};
```

becomes

```C++
static constexpr int zero = 0;
int v = /* ... */;
inspect (v) {
    +zero => { std::cout << "got zero"; }
//   ˆˆˆˆ id-expression
    1 => { std::cout << "got one"; }
//  ˆ expression pattern
    2 => { std::cout << "got two"; }
//  ˆ expression pattern
};
```

becomes

```C++
static constexpr int zero = 0, one = 1;
std::pair<int, int> p = /* ... */

inspect (p) {
    [+zero, +one] => {
//    ˆˆˆˆ   ˆˆˆ id-expression
        std::cout << zero << ' ' << one;
//      Note that    ˆˆˆˆ and       ˆˆˆ are id-expressions
//      that refer to the `static constexpr` variables.
    }
};
```

```C++
struct Node {
    int value;
    std::unique_ptr<Node> lhs, rhs;
};

void print_leftmost(const Node& node) {
    inspect (node) {
        [.value: v, .lhs: nullptr] => { std::cout << v << '\n'; }
        [.lhs: (*!) l] => { print_leftmost(l); }
//             ˆˆˆˆ dereference pattern
    };
}
```

becomes

```C++
struct Node {
    int value;
    std::unique_ptr<Node> lhs, rhs;
};

void print_leftmost(const Node& node) {
    inspect (node) {
        [.value: v, .lhs: nullptr] => { std::cout << v << '\n'; }
        [.lhs: deref: l] => { print_leftmost(l); }
//             ˆˆˆˆ dereference pattern
    };
}
```

```C++
template <typename T>
struct Is {
    template <typename Arg>
    Arg&& extract(Arg&& arg) const {
        static_assert(std::is_same_v<T, std::remove_cvref_t<Arg>>);
        return std::forward<Arg>(arg);
    }
};
template <typename T>
inline constexpr Is<T> is;
// P0480: `auto&& [std::string s, int i] = f();`
inspect (f()) {
    [(is<std::string>!) s, (is<int>!) i] => // ...
    // ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ ˆˆˆˆˆˆˆˆˆˆˆˆ extractor pattern
};
```

becomes

```C++
template <typename T>
struct Is {
    template <typename Arg>
    Arg&& extract(Arg&& arg) const {
        static_assert(std::is_same_v<T, std::remove_cvref_t<Arg>>);
        return std::forward<Arg>(arg);
    }
};
template <typename T>
inline constexpr Is<T> is;
// P0480: `auto&& [std::string s, int i] = f();`
inspect (f()) {
    [is<std::string>: s, is<int>: i] => // ...
    // ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ ˆˆˆˆˆˆˆˆˆˆˆˆ extractor pattern
};
```

```C++
struct Email {
    std::optional<std::array<std::string_view, 2>>
    try_extract(std::string_view sv) const;
};
inline constexpr Email email;
struct PhoneNumber {
    std::optional<std::array<std::string_view, 3>>
    try_extract(std::string_view sv) const;
};
inline constexpr PhoneNumber phone_number;
inspect (s) {
    (email?) [address, domain] => { std::cout << "got an email"; }
    (phone_number?) ["415", __, __] => { std::cout << "got a San Francisco phone number"; }
//  ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ extractor pattern
};
```

becomes

```C++
struct Email {
    std::optional<std::array<std::string_view, 2>>
    try_extract(std::string_view sv) const;
};
inline constexpr Email email;
struct PhoneNumber {
    std::optional<std::array<std::string_view, 3>>
    try_extract(std::string_view sv) const;
};
inline constexpr PhoneNumber phone_number;
inspect (s) {
    email: some: [address, domain] => { std::cout << "got an email"; }
    phone_number: some: ["415", __, __] => { std::cout << "got a San Francisco phone number"; }
//  ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ extractor pattern
};
```

```C++
struct Both {
  template <typename U>
  std::pair<U&&, U&&> extract(U&& u) const {
    return {std::forward<U>(u), std::forward<U>(u)};
  }
};
inline constexpr Both both;

inspect (v) {
  (both!) [[x, 0], [0, y]] => // ...
};
```

becomes

```C++
struct Both {
  template <typename U>
  std::pair<U&&, U&&> extract(U&& u) const {
    return {std::forward<U>(u), std::forward<U>(u)};
  }
};
inline constexpr Both both;

inspect (v) {
  both: [[x, 0], [0, y]] => // ...
};
```

```C++
inline constexpr at = both;

inspect (v) {
  <Point> (at!) [p, [x, y]] => // ... // ...
};
```

becomes

```C++
inline constexpr at = both;

inspect (v) {
  Point at: [p, [x, y]] => // ... // ...
};
```

```C++
template <typename T>
void Node<T>::balance() {
  *this = inspect (*this) {
    // left-left case
    [case Black, (*?) [case Red, (*?) [case Red, a, x, b], y, c], z, d]
      => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    [case Black, (*?) [case Red, a, x, (*?) [case Red, b, y, c]], z, d] // left-right case
      => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    [case Black, a, x, (*?) [case Red, (*?) [case Red, b, y, c], z, d]] // right-left case
      => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    [case Black, a, x, (*?) [case Red, b, y, (*?) [case Red, c, z, d]]] // right-right case
    => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    self => self; // do nothing
  };
}
```

becomes

```C++
template <typename T>
void Node<T>::balance() {
  *this = inspect (*this) {
    // left-left case
    [case Black, some: [case Red, some: [case Red, a, x, b], y, c], z, d]
      => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    [case Black, some: [case Red, a, x, some: [case Red, b, y, c]], z, d] // left-right case
      => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    [case Black, a, x, some: [case Red, some: [case Red, b, y, c], z, d]] // right-left case
      => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    [case Black, a, x, some: [case Red, b, y, some: [case Red, c, z, d]]] // right-right case
    => Node{Red, std::make_shared<Node>(Black, a, x, b),
                   y,
                   std::make_shared<Node>(Black, c, z, d)};
    self => self; // do nothing
  };
}
```
