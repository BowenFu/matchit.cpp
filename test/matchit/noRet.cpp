#include <gtest/gtest.h>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/expression.h"
using namespace matchit;

TEST(MatchStatement, test)
{
  testing::internal::CaptureStdout();
  match(4)(
      pattern(_) = [] { std::cout << "match all!"; });
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_STREQ(output.c_str(), "match all!");
}