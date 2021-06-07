#ifndef _CORE_H_
#define _CORE_H_
#include <tuple>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <cassert>

namespace matchit
{
namespace impl
{
template <typename... PatternPair>
class PatternPairsRetType
{
public:
    using RetType = std::common_type_t<typename PatternPair::RetType...>;
};

template <typename Value, bool byRef>
class ValueType
{
public:
    using ValueT = Value const;
};

template <typename Value>
class ValueType<Value, true>
{
public:
    using ValueT = Value &&;
};

template <typename Value, bool byRef>
class MatchHelper
{
private:
    using ValueT = typename ValueType<Value, byRef>::ValueT;
    ValueT mValue;
    using ValueRefT = ValueT&&;
public:
    template <typename V>
    explicit MatchHelper(V &&value)
        : mValue{std::forward<V>(value)}
    {
    }
    template <typename... PatternPair>
    auto operator()(PatternPair const &...patterns)
    {
        using RetType = typename PatternPairsRetType<PatternPair...>::RetType;
        if constexpr(!std::is_same_v<RetType, void>)
        {
            RetType result{};
            auto const func = [this, &result](auto const &pattern) -> bool {
                if (pattern.matchValue(std::forward<ValueRefT>(mValue)))
                {
                    result = pattern.execute();
                    return true;
                }
                return false;
            };
            bool const matched = (func(patterns) || ...);
            assert(matched);
            static_cast<void>(matched);
            return result;
        }
        else if constexpr(std::is_same_v<RetType, void>)
        {
            auto const func = [this](auto const &pattern) -> bool {
                if (pattern.matchValue(std::forward<ValueRefT>(mValue)))
                {
                    pattern.execute();
                    return true;
                }
                return false;
            };
            bool const matched = (func(patterns) || ...);
            assert(matched);
            static_cast<void>(matched);
        }
    }
};

template <typename Value>
auto match(Value &&value)
{
    return MatchHelper<Value, true>{std::forward<Value>(value)};
}

template <typename First, typename... Values>
auto match(First &&first, Values &&...values)
{
    return MatchHelper<decltype(std::forward_as_tuple(first, values...)), false>{std::forward_as_tuple(first, values...)};
}
} // namespace impl

// export symbols
using impl::match;

} // namespace matchit
#endif // _CORE_H_