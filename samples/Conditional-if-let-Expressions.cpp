#include <iostream>
#include <optional>
#include "matchit.h"

template <typename V, typename E>
using Result = std::variant<V, E>;

Result<uint8_t, std::exception> parse(std::string_view)
{
    return uint8_t{34U};
}

int32_t main()
{
    auto const favorite_color = std::optional<std::string>{};
    auto const is_tuesday = false;

    Result<uint8_t, std::exception> const age = parse("34");

    using namespace matchit;
    Id<std::string> color;
    match(favorite_color)(
        pattern(some(color)) = [&]
        { return "Using your favorite color, " + *color + ", as the background"; },
        pattern(_).when(expr(is_tuesday)) = expr("Tuesday is green day!"),
        pattern | _ = [&]
        {
            Id<uint8_t> age_;
            return match(age)(
                pattern(as<uint8_t>(age_)).when(age_ > 30) = expr("Using purple as the background color"),
                pattern(as<uint8_t>(age_)) = expr("Using orange as the background color"),
                pattern | _ = expr("Using blue as the background color"));
        });

    return 0;
}