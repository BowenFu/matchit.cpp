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
    public:
      template <typename Variant,
                typename std::enable_if<viaGetIfV<T, Variant>>::type * = nullptr>
      constexpr auto operator()(Variant const &v) const
      {
        return get_if<T>(std::addressof(v));
      }

      // template to disable implicit cast to std::any
      template <typename A, typename std::enable_if<std::is_same<A, std::any>::value>::type * = nullptr>
      constexpr auto operator()(A const &a) const
      {
        return std::any_cast<T>(std::addressof(a));
      }

      template <typename D, typename std::enable_if<!viaGetIfV<T, D> && std::is_base_of_v<T, D>>::type * = nullptr>
      constexpr auto operator()(D const &d) const
          -> decltype(static_cast<T const *>(std::addressof(d)))
      {
        return static_cast<T const *>(std::addressof(d));
      }

      template <typename B, typename std::enable_if<!viaGetIfV<T, B> && std::is_base_of_v<B, T>>::type * = nullptr>
      constexpr auto operator()(B const &b) const
          -> decltype(dynamic_cast<T const *>(std::addressof(b)))
      {
        return dynamic_cast<T const *>(std::addressof(b));
      }
    };

    template <typename T>
    constexpr AsPointer<T> asPointer;

    template <typename T>
    constexpr auto as = [](auto const pat)
    { return app(asPointer<T>, some(pat)); };

    template <typename Value, typename Pattern>
    constexpr auto matched(Value &&v, Pattern &&p)
    {
      return match(std::forward<Value>(v))(
          pattern | std::forward<Pattern>(p) = []
          { return true; },
          pattern | _ = []
          { return false; });
    }

    constexpr auto dsVia = [](auto &&...members)
    {
      return [members...](auto &&...pats)
      { return and_(app(members, pats)...); };
    };

    template <typename T>
    constexpr auto asDsVia = [](auto &&...members)
    {
      return [members...](auto &&...pats)
      {
        return as<T>(and_(app(members, pats)...));
      };
    };

  } // namespace impl
  using impl::as;
  using impl::asDsVia;
  using impl::dsVia;
  using impl::matched;
  using impl::none;
  using impl::some;
} // namespace matchit

#endif // MATCHIT_UTILITY_H
