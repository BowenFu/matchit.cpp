#ifndef MATCHIT_CORE_H
#define MATCHIT_CORE_H

#include <tuple>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <cassert>

namespace matchit
{
    namespace impl
    {
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

        template <typename Value, typename... Patterns>
        constexpr auto matchPatterns(Value&& value, Patterns const &...patterns);

        template <typename Value, bool byRef>
        class MatchHelper
        {
        private:
            using ValueT = typename ValueType<Value, byRef>::ValueT;
            ValueT mValue;
            using ValueRefT = ValueT &&;

        public:
            template <typename V>
            constexpr explicit MatchHelper(V &&value)
                : mValue{std::forward<V>(value)}
            {
            }
            template <typename... PatternPair>
            constexpr auto operator()(PatternPair const &...patterns)
            {
                return matchPatterns(std::forward<ValueRefT>(mValue), patterns...);
            }
        };

        template <typename Value>
        constexpr auto match(Value &&value)
        {
            return MatchHelper<Value, true>{std::forward<Value>(value)};
        }

        template <typename First, typename... Values>
        constexpr auto match(First &&first, Values &&...values)
        {
            return MatchHelper<decltype(std::forward_as_tuple(first, values...)), false>{std::forward_as_tuple(first, values...)};
        }
    } // namespace impl

    // export symbols
    using impl::match;

} // namespace matchit
#endif // MATCHIT_CORE_H