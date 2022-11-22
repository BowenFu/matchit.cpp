#include "matchit.h"
#include <iostream>
#include <optional>

template <typename V, typename E>
using Result = std::variant<V, E>;

Result<uint8_t, std::exception> parse(std::string_view) { return uint8_t{34U}; }

int32_t main()
{
  auto const favorite_color = std::optional<std::string>{};
  auto const is_tuesday = false;

  Result<uint8_t, std::exception> const age = parse("34");

  using namespace matchit;
  Id<std::string> color;
  match(favorite_color)(
      pattern | some(color) =
          [&]
      {
        return "Using your favorite color, " + *color +
               ", as the background";
      },
      pattern | _ | when(is_tuesday) = "Tuesday is green day!",
      pattern | _ =
          [&]
      {
        Id<uint8_t> age_;
        return match(age)(pattern | as<uint8_t>(age_) | when(age_ > 30) =
                              "Using purple as the background color",
                          pattern | as<uint8_t>(age_) =
                              "Using orange as the background color",
                          pattern | _ =
                              "Using blue as the background color");
      });

  return 0;
}