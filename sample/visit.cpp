#include "matchit.h"

#include <string>
#include <iostream>

template <typename T>
auto constexpr visitPat = [](auto func)
{
    using namespace matchit;

    return as<T>(meet([func](auto&& param) {
            func(param);
            return true;
        }));
};

template <typename T>
auto constexpr visit = [](auto func)
{
    using namespace matchit;

    return pattern | visitPat<T>(func) = []{};
};

template <typename T>
void print(T&& x)
{
    using namespace matchit;

    match(x)(
        visit<std::string>(
            [](const std::string& text) {
                std::cout << "Text message: " << text << std::endl;
            }),
        visit<int32_t>(
            [](const int32_t num) {
                std::cout << "Number: " << num << std::endl;
            })
        );
}

int main()
{
    using namespace matchit;

    std::variant<int32_t, std::string> v1 = "123";
    std::variant<int32_t, std::string> v2 = 123;

    print(v1);
    print(v2);
    return 0; 
}
