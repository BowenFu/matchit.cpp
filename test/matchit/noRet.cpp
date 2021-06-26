#include <gtest/gtest.h>
#include "matchit.h"

using namespace matchit;

TEST(MatchStatement, test)
{
  testing::internal::CaptureStdout();
  match(4)(
      // clang-format off
      pattern | or_(_ < 0, 2) = [] { std::cout << "mismatch!"; },
      pattern | _             = [] { std::cout << "match all!"; }
      // clang-format on
  );
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_STREQ(output.c_str(), "match all!");
}

TEST(MatchExpreesion, Nomatch)
{
  EXPECT_THROW(
      match(4)(
          pattern | 1 = expr(true)),
      std::logic_error);
}