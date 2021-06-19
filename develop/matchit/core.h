#ifndef MATCHIT_CORE_H
#define MATCHIT_CORE_H

#include <tuple>
#include <optional>
#include <cstdint>
#include <algorithm>

namespace matchit
{
    namespace impl
    {
        template <typename Value, bool byRef>
        class ValueType
        {
        public:
            using ValueT = Value;
        };

        template <typename Value>
        class ValueType<Value, true>
        {
        public:
            using ValueT = Value &&;
        };

        template <typename Value, typename... Patterns>
        constexpr auto matchPatterns(Value &&value, Patterns const &...patterns);

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

        template <typename... Values>
        class Debug;

        template <typename First, typename... Values>
        constexpr auto match(First &&first, Values &&...values)
        {
            auto result = std::forward_as_tuple(std::forward<First>(first), std::forward<Values>(values)...);
            // Debug<decltype(result)> aaa;
            return MatchHelper<decltype(result), false>{std::forward<decltype(result)>(result)};
        }
    } // namespace impl

    // export symbols
    using impl::match;

} // namespace matchit
#endif // MATCHIT_CORE_H