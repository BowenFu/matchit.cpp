#include <iostream>
#include <stack>
#include <optional>
#include "matchit.h"

struct Rgb : std::array<int32_t, 3> {};
struct Hsv : std::array<int32_t, 3> {};
using Color = std::variant<Rgb, Hsv>;

struct Quit {};
using Move = std::array<int32_t, 2>;
using Write = std::string;
using ChangeColor = Color;
using Message = std::variant<Quit, Move, Write, ChangeColor>;

int32_t main()
{
    Message const msg = ChangeColor{Hsv{{0, 160, 255}}};

    using namespace matchit;
    Id<int32_t> r, g, b;
    Id<int32_t> h, s, v;
    Id<std::string> text;
    match(msg)( 
        pattern | as<ChangeColor>(as<Rgb>(ds(r, g, b))) = [&] {
            std::cout <<
                "Change the color to red " << *r << ", green " << *g << ", and blue " << *b << std::endl;
        },
        pattern | as<ChangeColor>(as<Hsv>(ds(h, s, v))) = [&] {
            std::cout <<
                "Change the color to hue " << *h << ", saturation " << *s << ", and value " << *v << std::endl;
        }
     );
}
