#include "matchit.h"
#include <iostream>

struct Shape
{
  enum class Kind
  {
    Circle,
    Rectangle
  } kind;
};
struct Circle : Shape
{
  constexpr static Kind k = Kind::Circle;
  Circle(int radius) : Shape{Shape::Kind::Circle}, radius(radius) {}
  int radius;
};

struct Rectangle : Shape
{
  constexpr static Kind k = Kind::Rectangle;
  Rectangle(int width, int height)
      : Shape{Shape::Kind::Rectangle}, width(width), height(height) {}
  int width, height;
};

bool operator==(Circle const &lhs, Circle const &rhs)
{
  return lhs.radius == rhs.radius;
}

bool operator==(Rectangle const &lhs, Rectangle const &rhs)
{
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

template <typename T>
auto get_if(Shape const *shape)
{
  return static_cast<T const *>(shape->kind == T::k ? shape : nullptr);
}

double get_area(const Shape &shape)
{
  using namespace matchit;
  Id<Circle> c;
  Id<Rectangle> r;
  return match(shape)(
      pattern |
          as<Circle>(c) = [&]
      { return 3.14 * (*c).radius * (*c).radius; },
      pattern | as<Rectangle>(r) = [&]
      { return (*r).width * (*r).height; });
}

int32_t main()
{
  Rectangle r = {5, 7};
  std::cout << get_area(r) << std::endl;
  return 0;
}
