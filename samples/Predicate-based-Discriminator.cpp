#include "matchit.h"
#include <iostream>

struct String {
  enum Storage { Local, Remote };

  size_t size;

  struct Rm { char *ptr; int unused_allocated_space; };
  union {
    char local[32];
    Rm remote;
  };

  // Predicate-based discriminator derived from `size`.
  Storage index() const { return size > sizeof(local) ? Remote : Local; }

  // Opt into Variant-Like protocol.
  template <Storage S>
  auto get_if();

  char *data();
};

bool operator==(String::Rm const& lhs, String::Rm const& rhs)
{
    return lhs.ptr == rhs.ptr && lhs.unused_allocated_space == rhs.unused_allocated_space;
}


namespace std {
  // Opt into Variant-Like protocol.

  template <>
  struct variant_size<String> : std::integral_constant<std::size_t, 2> {};

  template <>
  struct variant_alternative<String::Local, String> {
    using type = decltype(String::local);
  };

  template <>
  struct variant_alternative<String::Remote, String> {
    using type = decltype(String::remote);
  };
}

// Opt into Variant-Like protocol.
template <String::Storage S>
auto String::get_if()
{
  if constexpr (S == Local) return index() == Local ? &local : nullptr;
  else if constexpr (S == Remote) return index() == Remote ? &remote : nullptr;
}

template <String::Storage S>
const auto asEnum = [](auto&& pat)
{
  using namespace matchit;
  return app([](auto&& x) { return x.template get_if<S>(); }, some(pat));
};

char* String::data() {
  using namespace matchit;
  Id<char*> l;
  Id<std::decay_t<decltype(remote)>> r;
  return match(*this) ( 
    pattern | asEnum<Local>(l) = expr(l),
    pattern | asEnum<Remote>(r) = [&]{ return (*r).ptr; }
  );
}

int32_t main()
{
    std::string rm = "long string.";
    String x{};
    x.size = 100;
    x.remote = String::Rm{rm.data(), 100};
    std::cout << x.data() << std::endl;
    return 0;
}
