#include "matchit.h"
#include <iostream>

enum class OS_Type
{
  Linux,
  BSD
};

enum class Arch_Type
{
  ARM64,
  X8664
};

int
get_sample_write_syscode_code(OS_Type os_type, Arch_Type arch_type)
{
  namespace m = matchit;
  return m::match(os_type)(
    m::pattern | OS_Type::BSD =
      [&] {
        return m::match(arch_type)(
          m::pattern | Arch_Type::ARM64 = [&] { return 1; },
          m::pattern | Arch_Type::X8664 = [&] { return 4; },
          m::pattern | m::_ = [&] { return 1; });
      },
    m::pattern | OS_Type::Linux =
      [&] {
        return m::match(arch_type)(
          m::pattern | Arch_Type::ARM64 = [&] { return 2; },
          m::pattern | Arch_Type::X8664 = [&] { return 5; },
          m::pattern | m::_ = [&] { return 1; });
      },
    m::pattern | m::_ = [&] { return 1; });
}

int32_t
main()
{
  std::cout << "sample write(3) syscode for linux arm64: "
            << get_sample_write_syscode_code(OS_Type::Linux, Arch_Type::ARM64)
            << std::endl;
}
