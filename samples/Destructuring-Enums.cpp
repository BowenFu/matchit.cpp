#include <iostream>
#include <stack>
#include <optional>
#include "matchit.h"

struct Quit {};
using Move = std::array<int32_t, 2>;
using Write = std::string;
using ChangeColor = std::array<int32_t, 3>;
using Message = std::variant<Quit, Move, Write, ChangeColor>;

int32_t main()
{
    Message const msg = ChangeColor{0, 160, 255};

    using namespace matchit;
    Id<int32_t> x, y;
    Id<std::string> text;
    Id<int32_t> r, g, b;
    match(msg)( 
        pattern | as<Quit>(_) = [] {
            std:: cout << "The Quit variant has no data to destructure." << std::endl;
        },
        pattern | as<Move>(ds(x, y)) = [&] {
            std::cout <<
                "Move in the x direction " << *x << " and in the y direction " << *y << std::endl; 
        },
        pattern | as<Write>(text) = [&] {
            std::cout << "Text message: " << *text << std::endl;
        },
        pattern | as<ChangeColor>(ds(r, g, b)) = [&] {
            std::cout <<
                "Change the color to red " << *r << ", green " << *g << ", and blue " << *b << std::endl;
        }
     );
}
