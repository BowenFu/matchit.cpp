#include <iostream>
#include "matchit.h"
using namespace matchit;

struct Hello { int32_t id; };
using Message = std::variant<Hello>;

int32_t main()
{
    Message const msg = Hello{5};

    using namespace matchit;
    Id<int32_t> id_variable;
    match(msg)( 
        pattern | as<Hello>(app(&Hello::id, id_variable.at(3 <= _ && _ <= 7))) = [&] {
            std::cout << "Found an id in range: " << *id_variable << std::endl;
        },
        pattern | as<Hello>(app(&Hello::id, 10 <= _ && _ <= 12)) = [&] {
            std::cout << "Found an id in another range" << std::endl;
        },
        pattern | as<Hello>(app(&Hello::id, id_variable)) = [&] {
            std::cout << "Found some other id: " << *id_variable << std::endl;
        }
    );
}