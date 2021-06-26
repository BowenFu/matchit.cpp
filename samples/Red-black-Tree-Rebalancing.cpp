#include "matchit.h"
#include <iostream>
#include <memory>

enum Color
{
  Red,
  Black
};
template <typename T>
struct Node
{
  Node() = default;
  Node(Color color_, std::shared_ptr<Node> const &lhs_, T value_,
       std::shared_ptr<Node> const &rhs_)
      : color{color_}, lhs{lhs_}, value{value_}, rhs{rhs_} {}
  void balance();
  Color color;
  std::shared_ptr<Node> lhs;
  T value;
  std::shared_ptr<Node> rhs;
};

template <typename T>
bool operator==(Node<T> const &lhs, Node<T> const &rhs)
{
  return lhs.color == rhs.color && lhs.lhs == rhs.lhs &&
         lhs.value == rhs.value && lhs.rhs == rhs.rhs;
}

template <typename T>
void Node<T>::balance()
{
  using namespace matchit;

  constexpr auto dsN = [](auto &&color, auto &&lhs, auto &&value, auto &&rhs)
  {
    return and_(app(&Node<T>::color, color), app(&Node<T>::lhs, lhs),
                app(&Node<T>::value, value), app(&Node<T>::rhs, rhs));
  };

  Id<std::shared_ptr<Node<T>>> a, b, c, d;
  Id<T> x, y, z;
  Id<Node> self;
  *this = match(*this)(
      pattern | dsN(Black, some(dsN(Red, some(dsN(Red, a, x, b)), y, c)), z,
                    d) // left-left case
      =
          [&]
      {
        return Node{Red, std::make_shared<Node>(Black, *a, *x, *b), *y,
                    std::make_shared<Node>(Black, *c, *z, *d)};
      },
      pattern | dsN(Black, some(dsN(Red, a, x, some(dsN(Red, b, y, c)))), z,
                    d) // left-right case
      =
          [&]
      {
        return Node{Red, std::make_shared<Node>(Black, *a, *x, *b), *y,
                    std::make_shared<Node>(Black, *c, *z, *d)};
      },
      pattern |
          dsN(Black, a, x,
              some(dsN(Red, some(dsN(Red, b, y, c)), z, d))) // right-left case
      =
          [&]
      {
        return Node{Red, std::make_shared<Node>(Black, *a, *x, *b), *y,
                    std::make_shared<Node>(Black, *c, *z, *d)};
      },
      pattern |
          dsN(Black, a, x,
              some(dsN(Red, b, y, some(dsN(Red, c, z, d))))) // right-right case
      =
          [&]
      {
        return Node{Red, std::make_shared<Node>(Black, *a, *x, *b), *y,
                    std::make_shared<Node>(Black, *c, *z, *d)};
      },
      pattern | self = expr(self) // do nothing
  );
}

int main()
{
  auto x = std::make_shared<Node<int>>(Color::Red, std::shared_ptr<Node<int>>{},
                                       1, std::shared_ptr<Node<int>>{});
  auto y = std::make_shared<Node<int>>(Color::Red, std::shared_ptr<Node<int>>{},
                                       2, x);
  auto z = std::make_shared<Node<int>>(Color::Black,
                                       std::shared_ptr<Node<int>>{}, 3, y);
  std::cout << "before balance: " << z->value << std::endl;
  z->balance();
  std::cout << "after balance: " << z->value << std::endl;
  return 0;
}