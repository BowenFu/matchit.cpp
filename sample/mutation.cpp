#include "matchit.h"
#include <variant>
#include <iostream>

class Circle {
public:
    void setLineWidth(float w)
    {
      std::cout << "Calling Circle::setLineWidth with w = " << w << std::endl;
    };
    // other stuff
};

class Square {
public:
    void setLineWidth(float)
    {
        // set line width
    };
    // other stuff
};

class Image {
    // other stuff
};

using Visual = std::variant<Circle, Square, Image>;

using namespace matchit;

void setLineWidth(Visual &visual, float width) {
    Id<Square*> sq;
    Id<Circle*> cir;
    match(visual)
    (
        pattern | as<Image>(_) = []{},
        pattern | asPtr<Square>(sq) = [&]
        {
            (*sq)->setLineWidth(width);
        },
        pattern | asPtr<Circle>(cir) = [&]
        {
            (*cir)->setLineWidth(width);
        }
    );
}

int main()
{
    Visual v = Circle{};
    setLineWidth(v, 1);
    return 0;
}
