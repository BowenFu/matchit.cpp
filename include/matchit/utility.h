#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <variant>
#include <any>
#include "./patterns.h"

namespace matchit
{
    namespace impl
    {

        template <typename T>
        auto constexpr cast = [](auto &&input) {
            return static_cast<T>(input);
        };

        auto constexpr some = [](auto const pat) {
            auto constexpr deref = [](auto &&x) { return *x; };
            return and_(app(cast<bool>, true), app(deref, pat));
        };

        auto constexpr none = app(cast<bool>, false);

        template <typename T>
        class AsPointerBase
        {
        public:
            template <typename B>
            auto operator()(B const &b) const
            {
                return dynamic_cast<T const *>(std::addressof(b));
            }
            template <typename... Types>
            auto operator()(std::variant<Types...> const &v) const
            {
                return std::get_if<T>(std::addressof(v));
            }
            auto operator()(std::any const &a) const
            {
                return std::any_cast<T>(std::addressof(a));
            }
        };

        template <typename T>
        class CustomAsPointer
        {
        };

        template <typename T, typename = std::void_t<>>
        class AsPointer : public AsPointerBase<T>
        {
        public:
            using AsPointerBase<T>::operator();
        };

        template <typename T>
        class AsPointer<T, std::void_t<decltype(&CustomAsPointer<T>::operator())>> : public AsPointerBase<T>, public CustomAsPointer<T>
        {
        public:
            using AsPointerBase<T>::operator();
            using CustomAsPointer<T>::operator();
        };

        template <typename T, typename AsPointerT = AsPointer<T> >
        auto constexpr as = [](auto const pat, AsPointerT const asPointer = {}) {
            return app(asPointer, some(pat));
        };

    } // namespace impl
    using impl::as;
    using impl::none;
    using impl::some;
} // namespace matchit

#endif // _UTILITY_H_