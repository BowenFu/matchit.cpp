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

template <typename Value, bool byLRef>
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

template <typename Value, bool byLRef>
class MatchHelper
{
private:
    using ValueT = typename ValueType<Value, byLRef>::ValueT;
    ValueT mValue;
    using ValueRefT = ValueT&&;
public:
    explicit MatchHelper(Value &&value)
        : mValue{std::forward<Value>(value)}
    {
    }
    template <typename... PatternPair>
    auto operator()(PatternPair const &...patterns)
    {
        using RetType = typename PatternPairsRetType<PatternPair...>::RetType;
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
        return result;
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
    auto const x = std::forward_as_tuple(first, values...);
    return MatchHelper<decltype(x), false>{x};
}
} // namespace impl

// export symbols
using impl::match;

} // namespace matchit
#endif // _CORE_H_