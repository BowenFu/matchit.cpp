#ifndef MATCHIT_UTILITY_H
#define MATCHIT_UTILITY_H

#include <any>
#include <variant>

namespace matchit
{
  namespace impl
  {

    template <typename T>
    constexpr auto cast = [](auto &&input)
    { return static_cast<T>(input); };

    constexpr auto deref = [](auto &&x) -> decltype(*x) & { return *x; };
    constexpr auto some = [](auto const pat)
    {
      return and_(app(cast<bool>, true), app(deref, pat));
    };

    constexpr auto none = app(cast<bool>, false);

    template <typename Value, typename Variant, typename = std::void_t<>>
    struct ViaGetIf : std::false_type
    {
    };

    using std::get_if;

    template <typename T, typename Variant>
    struct ViaGetIf<
        T, Variant,
        std::void_t<decltype(get_if<T>(std::declval<Variant const *>()))>>
        : std::true_type
    {
    };

    template <typename T, typename Variant>
    constexpr auto viaGetIfV = ViaGetIf<T, Variant>::value;

    static_assert(viaGetIfV<int, std::variant<int, bool>>);

    template <typename T>
    class AsPointer
    {
      static_assert(!std::is_reference_v<T>);
    public:
      template <typename Variant,
                typename std::enable_if<viaGetIfV<T, std::decay_t<Variant>>>::type * = nullptr>
      constexpr auto operator()(Variant&& v) const
      {
        return get_if<T>(std::addressof(v));
      }

      // template to disable implicit cast to std::any
      template <typename A, typename std::enable_if<std::is_same<std::decay_t<A>, std::any>::value>::type * = nullptr>
      constexpr auto operator()(A&& a) const
      {
        return std::any_cast<T>(std::addressof(a));
      }

      // cast to base class
      template <typename D, typename std::enable_if<!viaGetIfV<T, D> && std::is_base_of_v<T, D>>::type * = nullptr>
      constexpr auto operator()(D const& d) const
          -> decltype(static_cast<T const *>(std::addressof(d)))
      {
        return static_cast<T const *>(std::addressof(d));
      }

      // No way to handle rvalue to save copy in this class. Need to define some in another way to handle this.
      // cast to base class
      template <typename D, typename std::enable_if<!viaGetIfV<T, D> && std::is_base_of_v<T, D>>::type * = nullptr>
      constexpr auto operator()(D& d) const
          -> decltype(static_cast<T*>(std::addressof(d)))
      {
        return static_cast<T*>(std::addressof(d));
      }

      // cast to derived class
      template <typename B, typename std::enable_if<!viaGetIfV<T, B> && std::is_base_of_v<B, T>>::type * = nullptr>
      constexpr auto operator()(B const& b) const
          -> decltype(dynamic_cast<T const *>(std::addressof(b)))
      {
        return dynamic_cast<T const *>(std::addressof(b));
      }

      // cast to derived class
      template <typename B, typename std::enable_if<!viaGetIfV<T, B> && std::is_base_of_v<B, T>>::type * = nullptr>
      constexpr auto operator()(B& b) const
          -> decltype(dynamic_cast<T*>(std::addressof(b)))
      {
        return dynamic_cast<T*>(std::addressof(b));
      }
    };

    template <typename T>
    constexpr AsPointer<T> asPointer;

    template <typename T>
    constexpr auto as = [](auto const pat)
    { return app(asPointer<T>, some(pat)); };

    template <typename T>
    constexpr auto asPtr = [](auto const pat)
    { return app(asPointer<T>, and_(pat, _ != nullptr)); };

    template <typename Value, typename Pattern>
    constexpr auto matched(Value &&v, Pattern &&p)
    {
      return match(std::forward<Value>(v))(
          pattern | std::forward<Pattern>(p) = []
          { return true; },
          pattern | _ = []
          { return false; });
    }

    constexpr auto dsVia = [](auto ...members)
    {
      return [members...](auto ...pats)
      { return and_(app(members, pats)...); };
    };

    template <typename T>
    constexpr auto asDsVia = [](auto ...members)
    {
      return [members...](auto ...pats)
      {
        // FIXME, why the following line will cause segfault in at-Bindings.cpp
        // return as<T>(dsVia(members...)(pats...));
        return as<T>(and_(app(members, pats)...));
      };
    };

  } // namespace impl
  using impl::as;
  using impl::asPtr;
  using impl::asDsVia;
  using impl::dsVia;
  using impl::matched;
  using impl::none;
  using impl::some;
} // namespace matchit

#endif // MATCHIT_UTILITY_H
