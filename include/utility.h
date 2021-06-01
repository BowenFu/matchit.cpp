#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "./patterns.h"

namespace matchit
{
namespace impl
{

template <typename T>
auto constexpr cast = [](auto && input){
    return static_cast<T>(input);
}; 

auto constexpr some = [](auto const pat) {
    auto constexpr deref = [](auto &&x) { return *x; };
    return and_(app(cast<bool>, true), app(deref, pat));
};

auto constexpr none = app(cast<bool>, false);

template <typename T>
class AsPointerTraits
{
    class AsPointer
    {
    public:
        template <typename B>
        auto operator()(B const& b) const
        {
            return dynamic_cast<T const*>(std::addressof(b));
        }
        template <typename... Types>
        auto operator()(std::variant<Types...> const& v) const
        {
            return std::get_if<T>(std::addressof(v));
        }
        auto operator()(std::any const& a) const
        {
            return std::any_cast<T>(std::addressof(a));
        }
    };
public:
    constexpr static auto asPointer = AsPointer{};
};

// inherit from the basic Pred?

// kind -> cast
// get_if
// get

template <typename T, typename AsPointer = decltype(AsPointerTraits<T>::asPointer)>
auto constexpr as = [](auto const pat, AsPointer const asPointer = AsPointerTraits<T>::asPointer)
{
    return app(asPointer, some(pat));
    // return meet(asPointer);
};

} // namespace impl
using impl::some;
using impl::none;
using impl::as;
} // namespace matchit

#endif // _UTILITY_H_