#include <iostream>
#include <optional>
#include "matchit.h"

template <typename T>
std::ostream& operator<< (std::ostream& o, std::optional<T> const& op)
{
    if (op)
    {
        o << *op;
    }
    else
    {
        o << "none";
    }
    return o;
}

void sample1()
{
    auto setting_value = std::make_optional(5);
    auto const new_setting_value = std::make_optional(10);

    using namespace matchit;
    match (setting_value, new_setting_value) ( 
        pattern(some(_), some(_)) = [] {
            std:: cout << "Can't overwrite an existing customized value" << std::endl;
        },
        pattern(_) = [&] {
            setting_value = new_setting_value;
        }
     );

    std::cout << "setting is " << setting_value << std::endl;
}

void sample2()
{
    auto const numbers = std::make_tuple(2, 4, 8, 16, 32);

    using namespace matchit;
    Id<int32_t> first, third, fifth;
    match(numbers)(
        pattern(first, _, third, _, fifth) = [&]
        { std::cout << "Some numbers: " << *first << ", " << *third << ", " << *fifth << std::endl; });
}

int32_t main()
{
    sample1();
    sample2();
    return 0;
}