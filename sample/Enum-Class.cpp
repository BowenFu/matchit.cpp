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

  // Dependent enum case example:
  return m::match(os_type)(
    m::pattern | OS_Type::BSD =
      [&] {
        return m::match(arch_type)(m::pattern | Arch_Type::ARM64 = 1,
                                   m::pattern | Arch_Type::X8664 = 4,
                                   m::pattern | m::_ = 1);
      },
    m::pattern | OS_Type::Linux =
      [&] {
        return m::match(arch_type)(m::pattern | Arch_Type::ARM64 = 2,
                                   m::pattern | Arch_Type::X8664 = 5,
                                   m::pattern | m::_ = 1);
      },
    m::pattern | m::_ = 1);
}

int32_t
main()
{
  // Independent enum case example:
  auto independent_sample = m::match(os_type, arch_type)(
    m::pattern | m::ds(OS_Type::Linux, Arch_Type::ARM64) = 2,
    m::pattern | m::ds(OS_Type::Linux, Arch_Type::X8664) = 5,
    m::pattern | m::ds(OS_Type::BSD, Arch_Type::X8664) = 4,
    m::pattern | m::_ = 1);

  std::cout << "independent write(3) enum case example: " << independent_sample
            << std::endl;

  std::cout << "dependent write(3) enum case example: "
            << get_sample_write_syscode_code(OS_Type::Linux, Arch_Type::ARM64)
            << std::endl;
}
