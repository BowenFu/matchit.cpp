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
#ifndef MATCHIT_EXPRESSION_H
#define MATCHIT_EXPRESSION_H

#include <type_traits>

namespace matchit
{
    namespace impl
    {
        template <typename T>
        class Nullary : public T
        {
        public:
            using T::operator();
        };

        template <typename T>
        constexpr auto nullary(T const &t)
        {
            return Nullary<T>{t};
        }

        template <typename T>
        class Id;
        template <typename T>
        constexpr auto expr(Id<T> const &id)
        {
            return nullary([&] { return *id; });
        }

        template <typename T>
        constexpr auto expr(T const&v)
        {
            return nullary([&]
                           { return v; });
        }

        // for constant
        template <typename T>
        class EvalTraits
        {
        public:
            template <typename... Args>
            constexpr static decltype(auto) evalImpl(T const &v, Args const&...)
            {
                return v;
            }
        };

        template <typename T>
        class EvalTraits<Nullary<T>>
        {
        public:
            constexpr static decltype(auto) evalImpl(Nullary<T> const &e)
            {
                return e();
            }
        };

        // Only allowed in nullary
        template <typename T>
        class EvalTraits<Id<T>>
        {
        public:
            constexpr static decltype(auto) evalImpl(Id<T> const &id)
            {
                return *id;
            }
        };

        template <typename Pred>
        class Meet;

        // Unary is an alias of Meet.
        template <typename T>
        using Unary = Meet<T>;

        template <typename T>
        class EvalTraits<Unary<T>>
        {
        public:
            template <typename Arg>
            constexpr static decltype(auto) evalImpl(Unary<T> const &e, Arg const&arg)
            {
                return e(arg);
            }
        };

        class Wildcard;
        template <>
        class EvalTraits<Wildcard>
        {
        public:
            template <typename Arg>
            constexpr static decltype(auto) evalImpl(Wildcard const &, Arg const&arg)
            {
                return arg;
            }
        };

        template <typename T, typename... Args>
        constexpr decltype(auto) eval(T const&t, Args const&...args)
        {
            return EvalTraits<T>::evalImpl(t, args...);
        }

        template <typename T>
        class IsNullaryOrId : public std::false_type
        {
        };

        template <typename T>
        class IsNullaryOrId<Id<T>> : public std::true_type
        {
        };

        template <typename T>
        class IsNullaryOrId<Nullary<T>> : public std::true_type
        {
        };

        template <typename T>
        constexpr auto isNullaryOrIdV = IsNullaryOrId<std::decay_t<T>>::value;

#define UN_OP_FOR_NULLARY(op)                                               \
    template <typename T, std::enable_if_t<isNullaryOrIdV<T>, bool> = true> \
    constexpr auto operator op(T const&t)                                       \
    {                                                                       \
        return nullary([&] { return op eval(t); });                         \
    }

#define BIN_OP_FOR_NULLARY(op)                                                                               \
    template <typename T, typename U, std::enable_if_t<isNullaryOrIdV<T> || isNullaryOrIdV<U>, bool> = true> \
    constexpr auto operator op(T const&t, U const&u)                                                                 \
    {                                                                                                        \
        return nullary([&] { return eval(t) op eval(u); });                                                  \
    }

        // ADL will find these operators.
        UN_OP_FOR_NULLARY(!)
        UN_OP_FOR_NULLARY(-)

#undef UN_OP_FOR_NULLARY

        BIN_OP_FOR_NULLARY(+)
        BIN_OP_FOR_NULLARY(-)
        BIN_OP_FOR_NULLARY(*)
        BIN_OP_FOR_NULLARY(/)
        BIN_OP_FOR_NULLARY(%)
        BIN_OP_FOR_NULLARY(<)
        BIN_OP_FOR_NULLARY(<=)
        BIN_OP_FOR_NULLARY(==)
        BIN_OP_FOR_NULLARY(!=)
        BIN_OP_FOR_NULLARY(>=)
        BIN_OP_FOR_NULLARY(>)
        BIN_OP_FOR_NULLARY(||)
        BIN_OP_FOR_NULLARY(&&)
        BIN_OP_FOR_NULLARY(<<)

#undef BIN_OP_FOR_NULLARY

        // Unary
        template <typename T>
        class IsUnaryOrWildcard : public std::false_type
        {
        };

        template <>
        class IsUnaryOrWildcard<Wildcard> : public std::true_type
        {
        };

        template <typename T>
        class IsUnaryOrWildcard<Unary<T>> : public std::true_type
        {
        };

        template <typename T>
        constexpr auto isUnaryOrWildcardV = IsUnaryOrWildcard<std::decay_t<T>>::value;

        // unary is an alias of meet.
        template <typename T>
        constexpr auto unary(T &&t)
        {
            return meet(std::forward<T>(t));
        }

#define UN_OP_FOR_UNARY(op)                                                     \
    template <typename T, std::enable_if_t<isUnaryOrWildcardV<T>, bool> = true> \
    constexpr auto operator op(T const&t)                                           \
    {                                                                           \
        return unary([&](auto &&arg) constexpr { return op eval(t, arg); });    \
    }

#define BIN_OP_FOR_UNARY(op)                                                                                         \
    template <typename T, typename U, std::enable_if_t<isUnaryOrWildcardV<T> || isUnaryOrWildcardV<U>, bool> = true> \
    constexpr auto operator op(T const&t, U const&u)                                                                         \
    {                                                                                                                \
        return unary([&](auto &&arg) constexpr { return eval(t, arg) op eval(u, arg); });                            \
    }

        UN_OP_FOR_UNARY(!)
        UN_OP_FOR_UNARY(-)

#undef UN_OP_FOR_UNARY

        BIN_OP_FOR_UNARY(+)
        BIN_OP_FOR_UNARY(-)
        BIN_OP_FOR_UNARY(*)
        BIN_OP_FOR_UNARY(/)
        BIN_OP_FOR_UNARY(%)
        BIN_OP_FOR_UNARY(<)
        BIN_OP_FOR_UNARY(<=)
        BIN_OP_FOR_UNARY(==)
        BIN_OP_FOR_UNARY(!=)
        BIN_OP_FOR_UNARY(>=)
        BIN_OP_FOR_UNARY(>)
        BIN_OP_FOR_UNARY(||)
        BIN_OP_FOR_UNARY(&&)

#undef BIN_OP_FOR_UNARY

    } // namespace impl
    using impl::expr;
} // namespace matchit

#endif // MATCHIT_EXPRESSION_H
#ifndef MATCHIT_PATTERNS_H
#define MATCHIT_PATTERNS_H

#include <memory>
#include <tuple>
#include <functional>
#include <vector>
#include <variant>
#include <array>
#include <type_traits>
#include <cassert>

namespace matchit
{
    namespace impl
    {
        template <typename T, typename... Ts>
        class WithinTypes
        {
        public:
            constexpr static auto value = (std::is_same_v<T, Ts> || ...);
        };

        template <typename T, typename Tuple>
        class PrependUnique;

        template <typename T, typename... Ts>
        class PrependUnique<T, std::tuple<Ts...>>
        {
            constexpr static auto unique = !WithinTypes<T, Ts...>::value;

        public:
            using type = std::conditional_t<unique, std::tuple<T, Ts...>, std::tuple<Ts...>>;
        };

        template <typename T, typename Tuple>
        using PrependUniqueT = typename PrependUnique<T, Tuple>::type;

        template <typename Tuple>
        class Unique;

        template <typename Tuple>
        using UniqueT = typename Unique<Tuple>::type;

        template <>
        class Unique<std::tuple<>>
        {
        public:
            using type = std::tuple<>;
        };

        template <typename T, typename... Ts>
        class Unique<std::tuple<T, Ts...>>
        {
        public:
            using type = PrependUniqueT<T, UniqueT<std::tuple<Ts...>>>;
        };

        static_assert(std::is_same_v<std::tuple<int32_t>, UniqueT<std::tuple<int32_t, int32_t>>>);
        static_assert(std::is_same_v<std::tuple<std::tuple<>, int32_t>, UniqueT<std::tuple<int32_t, std::tuple<>, int32_t>>>);

        using std::get;

        namespace detail
        {
            template <std::size_t start, class Tuple, std::size_t... I>
            constexpr decltype(auto) subtupleImpl(Tuple &&t, std::index_sequence<I...>)
            {
                return std::forward_as_tuple(get<start + I>(std::forward<Tuple>(t))...);
            }
        } // namespace detail

        // [start, end)
        template <std::size_t start, std::size_t end, class Tuple>
        constexpr decltype(auto) subtuple(Tuple &&t)
        {
            constexpr auto tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple>>;
            static_assert(start <= end);
            static_assert(end <= tupleSize);
            return detail::subtupleImpl<start>(
                std::forward<Tuple>(t),
                std::make_index_sequence<end - start>{});
        }

        template <std::size_t start, class Tuple>
        constexpr decltype(auto) drop(Tuple &&t)
        {
            constexpr auto tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple>>;
            static_assert(start <= tupleSize);
            return subtuple<start, tupleSize>(std::forward<Tuple>(t));
        }

        template <std::size_t len, class Tuple>
        constexpr decltype(auto) take(Tuple &&t)
        {
            constexpr auto tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple>>;
            static_assert(len <= tupleSize);
            return subtuple<0, len>(std::forward<Tuple>(t));
        }

        template <class F, class Tuple>
        constexpr decltype(auto) apply_(F &&f, Tuple &&t)
        {
            return std::apply(
                std::forward<F>(f), drop<0>(std::forward<Tuple>(t)));
        }

        // as constexpr
        template <class F, class... Args>
        constexpr std::invoke_result_t<F, Args...>
        invoke_(F &&f, Args &&...args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        {
            return std::apply(
                std::forward<F>(f), std::forward_as_tuple(std::forward<Args>(args)...));
        }

        template <typename Pattern>
        class PatternTraits;

        template <typename... PatternPairs>
        class PatternPairsRetType
        {
        public:
            using RetType = std::common_type_t<typename PatternPairs::RetType...>;
        };

        enum class IdProcess : int32_t
        {
            kCANCEL,
            kCONFIRM
        };

        template <typename Pattern>
        constexpr void processId(Pattern const &pattern, int32_t depth, IdProcess idProcess)
        {
            PatternTraits<Pattern>::processIdImpl(pattern, depth, idProcess);
        }

        template <typename Tuple>
        class Variant;

        template <typename T, typename... Ts>
        class Variant<std::tuple<T, Ts...>>
        {
        public:
            using type = std::variant<std::monostate, T, Ts...>;
        };

        template <typename... Ts>
        class Context
        {
            using ElementT = typename Variant<UniqueT<std::tuple<Ts...>>>::type;
            using ContainerT = std::array<ElementT, sizeof...(Ts)>;
            ContainerT mMemHolder;
            size_t mSize = 0;

        public:
            template <typename T>
            constexpr void emplace_back(T &&t)
            {
                mMemHolder[mSize] = std::forward<T>(t);
                ++mSize;
            }
            constexpr auto back() -> ElementT &
            {
                return mMemHolder[mSize - 1];
            }
        };

        template <>
        class Context<>
        {
        };

        template <typename T>
        class ContextTrait;

        template <typename... Ts>
        class ContextTrait<std::tuple<Ts...>>
        {
        public:
            using ContextT = Context<Ts...>;
        };

        template <typename Value, typename Pattern, typename ConctextT>
        constexpr auto matchPattern(Value &&value, Pattern const &pattern, int32_t depth, ConctextT &context)
        {
            auto const result = PatternTraits<Pattern>::matchPatternImpl(std::forward<Value>(value), pattern, depth, context);
            auto const process = result ? IdProcess::kCONFIRM : IdProcess::kCANCEL;
            processId(pattern, depth, process);
            return result;
        }

        template <typename... Ts>
        class Debug;

        template <typename Pattern, typename Func>
        class PatternPair
        {
        public:
            using RetType = std::invoke_result_t<Func>;
            using PatternT = Pattern;

            constexpr PatternPair(Pattern const &pattern, Func const &func)
                : mPattern{pattern}, mHandler{func}
            {
            }
            template <typename Value, typename ContextT>
            constexpr bool matchValue(Value &&value, ContextT &context) const
            {
                return matchPattern(std::forward<Value>(value), mPattern, /*depth*/ 0, context);
            }
            constexpr auto execute() const
            {
                return mHandler();
            }

        private:
            Pattern const &mPattern;
            Func const &mHandler;
        };

        template <typename Pattern, typename Pred>
        class PostCheck;

        template <typename Pattern>
        class PatternHelper
        {
        public:
            constexpr explicit PatternHelper(Pattern const &pattern)
                : mPattern{pattern}
            {
            }
            template <typename Func>
            constexpr auto operator=(Func const &func)
            {
                return PatternPair<Pattern, Func>{mPattern, func};
            }
            template <typename Pred>
            constexpr auto when(Pred const &pred)
            {
                return PatternHelper<PostCheck<Pattern, Pred>>(PostCheck(mPattern, pred));
            }

        private:
            Pattern const mPattern;
        };

        template <typename Pattern>
        constexpr auto pattern(Pattern const &p)
        {
            return PatternHelper<std::decay_t<Pattern>>{p};
        }

        template <typename... Patterns>
        class Ds;

        template <typename... Patterns>
        constexpr auto ds(Patterns const &...patterns) -> Ds<Patterns...>;

        template <typename First, typename... Patterns>
        constexpr auto pattern(First const &f, Patterns const &...ps)
        {
            return PatternHelper<Ds<First, Patterns...>>{ds(f, ps...)};
        }

        template <typename Pattern>
        class PatternTraits
        {
        public:
            template <typename Value>
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, Pattern const &pattern, int32_t /* depth */, ContextT & /*context*/)
            {
                return pattern == std::forward<Value>(value);
            }
            constexpr static void processIdImpl(Pattern const &, int32_t /*depth*/, IdProcess)
            {
            }
        };

        class Wildcard
        {
        };

        constexpr Wildcard _;

        template <>
        class PatternTraits<Wildcard>
        {
            using Pattern = Wildcard;

        public:
            template <typename Value>
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            constexpr static bool matchPatternImpl(Value &&, Pattern const &, int32_t, ContextT &)
            {
                return true;
            }
            constexpr static void processIdImpl(Pattern const &, int32_t /*depth*/, IdProcess)
            {
            }
        };

        template <typename... Patterns>
        class Or
        {
        public:
            constexpr explicit Or(Patterns const &...patterns)
                : mPatterns{patterns...}
            {
            }
            constexpr auto const &patterns() const
            {
                return mPatterns;
            }

        private:
            std::tuple<Patterns...> mPatterns;
        };

        template <typename... Patterns>
        constexpr auto or_(Patterns const &...patterns)
        {
            return Or<Patterns...>{patterns...};
        }

        template <typename... Patterns>
        class PatternTraits<Or<Patterns...>>
        {
        public:
            template <typename Value>
            using AppResultTuple = decltype(std::tuple_cat(typename PatternTraits<Patterns>::template AppResultTuple<Value>{}...));

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, Or<Patterns...> const &orPat, int32_t depth, ContextT &context)
            {
                constexpr auto patSize = sizeof...(Patterns);
                return std::apply(
                           [&value, depth, &context](auto const &...patterns) {
                               return (matchPattern(value, patterns, depth + 1, context) || ...);
                           },
                           take<patSize - 1>(orPat.patterns())) ||
                       matchPattern(std::forward<Value>(value), get<patSize - 1>(orPat.patterns()), depth + 1, context);
            }
            constexpr static void processIdImpl(Or<Patterns...> const &orPat, int32_t depth, IdProcess idProcess)
            {
                return std::apply(
                    [depth, idProcess](Patterns const &...patterns) {
                        return (processId(patterns, depth, idProcess), ...);
                    },
                    orPat.patterns());
            }
        };

        template <typename Pred>
        class Meet : public Pred
        {
        public:
            using Pred::operator();
        };

        template <typename Pred>
        constexpr auto meet(Pred const &pred)
        {
            return Meet<Pred>{pred};
        }

        template <typename Pred>
        class PatternTraits<Meet<Pred>>
        {
        public:
            template <typename Value>
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, Meet<Pred> const &meetPat, int32_t /* depth */, ContextT &)
            {
                return meetPat(std::forward<Value>(value));
            }
            constexpr static void processIdImpl(Meet<Pred> const &, int32_t /*depth*/, IdProcess)
            {
            }
        };

        template <typename Unary, typename Pattern>
        class App
        {
        public:
            constexpr App(Unary &&unary, Pattern const &pattern)
                : mUnary{std::forward<Unary>(unary)}, mPattern{pattern}
            {
            }
            constexpr auto const &unary() const
            {
                return mUnary;
            }
            constexpr auto const &pattern() const
            {
                return mPattern;
            }

        private:
            Unary const mUnary;
            Pattern const mPattern;
        };

        template <typename Unary, typename Pattern>
        constexpr auto app(Unary &&unary, Pattern const &pattern)
        {
            return App<Unary, Pattern>{std::forward<Unary>(unary), pattern};
        }

        constexpr auto y = 1;
        static_assert(std::holds_alternative<int32_t const *>(std::variant<std::monostate, const int32_t *>{&y}));

        template <typename Unary, typename Pattern>
        class PatternTraits<App<Unary, Pattern>>
        {
        public:
            template <typename Value>
            using AppResult = std::invoke_result_t<Unary, Value>;
            // We store value for scalar types in Id and they can not be moved. So to support constexpr.
            template <typename Value>
            using AppResultCurTuple = std::conditional_t<std::is_lvalue_reference_v<AppResult<Value>> || std::is_scalar_v<AppResult<Value>>, std::tuple<>, std::tuple<AppResult<Value>>>;

            template <typename Value>
            using AppResultTuple = decltype(std::tuple_cat(std::declval<AppResultCurTuple<Value>>(), std::declval<typename PatternTraits<Pattern>::template AppResultTuple<AppResult<Value>>>()));

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, App<Unary, Pattern> const &appPat, int32_t depth, ContextT &context)
            {
                if constexpr (std::is_same_v<AppResultCurTuple<Value>, std::tuple<>>)
                {
                    return matchPattern(std::forward<AppResult<Value>>(invoke_(appPat.unary(), value)), appPat.pattern(), depth + 1, context);
                }
                else
                {
                    context.emplace_back(invoke_(appPat.unary(), value));
                    decltype(auto) result = get<std::decay_t<AppResult<Value>>>(context.back());
                    return matchPattern(std::forward<AppResult<Value>>(result), appPat.pattern(), depth + 1, context);
                }
            }
            constexpr static void processIdImpl(App<Unary, Pattern> const &appPat, int32_t depth, IdProcess idProcess)
            {
                return processId(appPat.pattern(), depth, idProcess);
            }
        };

        template <typename... Patterns>
        class And
        {
        public:
            constexpr explicit And(Patterns const &...patterns)
                : mPatterns{patterns...}
            {
            }
            constexpr auto const &patterns() const
            {
                return mPatterns;
            }

        private:
            std::tuple<Patterns...> mPatterns;
        };

        template <typename... Patterns>
        constexpr auto and_(Patterns const &...patterns)
        {
            return And<Patterns...>{patterns...};
        }

        template <typename... Patterns>
        class PatternTraits<And<Patterns...>>
        {
        public:
            template <typename Value>
            using AppResultTuple = decltype(std::tuple_cat(std::declval<typename PatternTraits<Patterns>::template AppResultTuple<Value>>()...));

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, And<Patterns...> const &andPat, int32_t depth, ContextT &context)
            {
                constexpr auto patSize = sizeof...(Patterns);
                return std::apply(
                           [&value, depth, &context](auto const &...patterns) {
                               return (matchPattern(value, patterns, depth + 1, context) && ...);
                           },
                           take<patSize - 1>(andPat.patterns())) &&
                       matchPattern(std::forward<Value>(value), get<patSize - 1>(andPat.patterns()), depth + 1, context);
            }
            constexpr static void processIdImpl(And<Patterns...> const &andPat, int32_t depth, IdProcess idProcess)
            {
                return std::apply(
                    [depth, idProcess](Patterns const &...patterns) {
                        return (processId(patterns, depth, idProcess), ...);
                    },
                    andPat.patterns());
            }
        };

        template <typename Pattern>
        class Not
        {
        public:
            explicit Not(Pattern const &pattern)
                : mPattern{pattern}
            {
            }
            auto const &pattern() const
            {
                return mPattern;
            }

        private:
            Pattern mPattern;
        };

        template <typename Pattern>
        constexpr auto not_(Pattern const &pattern)
        {
            return Not<Pattern>{pattern};
        }

        template <typename Pattern>
        class PatternTraits<Not<Pattern>>
        {
        public:
            template <typename Value>
            using AppResultTuple = typename PatternTraits<Pattern>::template AppResultTuple<Value>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, Not<Pattern> const &notPat, int32_t depth, ContextT &context)
            {
                return !matchPattern(std::forward<Value>(value), notPat.pattern(), depth + 1, context);
            }
            constexpr static void processIdImpl(Not<Pattern> const &notPat, int32_t depth, IdProcess idProcess)
            {
                processId(notPat.pattern(), depth, idProcess);
            }
        };

        class Deleter
        {
        public:
            template <typename Type>
            void operator()(Type *ptr)
            {
                if (mOwn)
                {
                    std::default_delete<Type>{}(ptr);
                }
            }
            bool mOwn{false};
        };

        template <typename Ptr, typename Value, typename = std::void_t<>>
        struct StorePointer : std::false_type
        {
        };

        template <typename Type>
        using ValueVariant = std::conditional_t<std::is_abstract_v<Type>, std::variant<std::monostate, Type const *>, std::variant<std::monostate, Type, Type const *>>;

        template <typename Type, typename Value>
        struct StorePointer<Type, Value, std::void_t<decltype(std::declval<ValueVariant<Type> &>() = &std::declval<Value>())>>
            : std::conjunction<std::is_lvalue_reference<Value>, std::negation<std::is_scalar<Value>>>
        {
        };

        static_assert(StorePointer<char, char &>::value);
        static_assert(StorePointer<const char, char &>::value);
        static_assert(StorePointer<const char, const char &>::value);
        static_assert(StorePointer<std::unique_ptr<int32_t> const, std::unique_ptr<int32_t> const &>::value);
        static_assert(StorePointer<std::tuple<int &, int &> const, std::tuple<int &, int &> const &>::value);
        static_assert(!StorePointer<std::unique_ptr<int32_t>, std::unique_ptr<int32_t> &&>::value);

        template <typename... Ts>
        class Overload : public Ts...
        {
        public:
            using Ts::operator()...;
        };

        template <typename... Ts>
        constexpr auto overload(Ts &&...ts)
        {
            return Overload<Ts...>{ts...};
        }

        template <typename Type>
        class Id
        {
        private:
            class Block
            {
            public:
                ValueVariant<Type> mVariant;
                int32_t mDepth;

                constexpr auto &variant()
                {
                    return mVariant;
                }
                constexpr auto hasValue() const
                {
                    return std::visit(
                        overload(
                            [](Type const &) {
                                return true;
                            },
                            [](Type const *) {
                                return true;
                            },
                            [](std::monostate const &) {
                                return false;
                            }),
                        mVariant);
                }
                constexpr decltype(auto) value() const
                {
                    return std::visit(
                        overload(
                            [](Type const &v) -> Type const & {
                                return v;
                            },
                            [](Type const *p) -> Type const & {
                                return *p;
                            },
                            [](std::monostate const &) -> Type const & {
                                assert(false && "invalid state!");
                                return *reinterpret_cast<Type const *>(1);
                            }),
                        mVariant);
                }

                constexpr decltype(auto) mutableValue()
                {
                    return std::visit(
                        overload(
                            [](Type &v) -> Type & {
                                return v;
                            },
                            [](Type const *) -> Type & {
                                assert(false && "Cannot get mutableValue for pointer type!");
                                return *reinterpret_cast<Type *>(1);
                            },
                            [](std::monostate &) -> Type & {
                                assert(false && "invalid state!");
                                return *reinterpret_cast<Type *>(1);
                            }),
                        mVariant);
                }
                constexpr void reset(int32_t depth)
                {
                    if (mDepth - depth >= 0)
                    {
                        mVariant = {};
                        mDepth = depth;
                    }
                }
                constexpr void confirm(int32_t depth)
                {
                    if (mDepth > depth || mDepth == 0)
                    {
                        assert(depth == mDepth - 1 || depth == mDepth || mDepth == 0);
                        mDepth = depth;
                    }
                }
            };
            class IdTrait
            {
            public:
                template <typename Value>
                constexpr static auto matchValueImpl(ValueVariant<Type> &v, Value &&value, std::false_type /* StorePointer */)
                {
                    // for constexpr
                    v = ValueVariant<Type>{std::forward<Value>(value)};
                }
                template <typename Value>
                constexpr static auto matchValueImpl(ValueVariant<Type> &v, Value &&value, std::true_type /* StorePointer */)
                {
                    v = ValueVariant<Type>{&value};
                }
            };

            using BlockVT = std::variant<Block, Block *>;
            BlockVT mBlock = Block{};

        public:
            constexpr Id() = default;

            constexpr Id(Id const &id)
            {
                mBlock = BlockVT{&id.block()};
            }

            constexpr Block &block() const
            {
                return std::visit(
                    overload(
                        [](Block &v) -> Block & {
                            return v;
                        },
                        [](Block *p) -> Block & {
                            return *p;
                        }),
                    // constexpr does not allow mutable, we use const_cast instead.
                    // Never declare Id as const.
                    const_cast<BlockVT &>(mBlock));
            }

            template <typename Value>
            constexpr auto
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
                __attribute__((no_sanitize_address))
#endif
#endif
                matchValue(Value &&v) const
            {
                if (hasValue())
                {
                    return value() == v;
                }
                IdTrait::matchValueImpl(block().variant(), std::forward<Value>(v), StorePointer<Type, Value>{});
                return true;
            }
            constexpr void reset(int32_t depth) const
            {
                return block().reset(depth);
            }
            constexpr void confirm(int32_t depth) const
            {
                return block().confirm(depth);
            }
            constexpr bool hasValue() const
            {
                return block().hasValue();
            }
            constexpr Type const &value() const
            {
                return block().value();
            }
            constexpr Type const &operator*() const
            {
                return value();
            }
            constexpr Type &&move()
            {
                return std::move(block().mutableValue());
            }
        };

        template <typename Type>
        class PatternTraits<Id<Type>>
        {
        public:
            template <typename Value>
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, Id<Type> const &idPat, int32_t /* depth */, ContextT &)
            {
                return idPat.matchValue(std::forward<Value>(value));
            }
            constexpr static void processIdImpl(Id<Type> const &idPat, int32_t depth, IdProcess idProcess)
            {
                switch (idProcess)
                {
                case IdProcess::kCANCEL:
                    idPat.reset(depth);
                    break;

                case IdProcess::kCONFIRM:
                    idPat.confirm(depth);
                    break;
                }
            }
        };

        template <typename... Patterns>
        class Ds
        {
        public:
            constexpr explicit Ds(Patterns const &...patterns)
                : mPatterns{patterns...}
            {
            }
            constexpr auto const &patterns() const
            {
                return mPatterns;
            }

        private:
            template <typename T>
            struct AddConstToPointer
            {
                using type = std::conditional_t<
                    !std::is_pointer_v<T>,
                    T,
                    std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>>>;
            };
            template <typename T>
            using AddConstToPointerT = typename AddConstToPointer<T>::type;

            static_assert(std::is_same_v<AddConstToPointerT<void *>, void const *>);
            static_assert(std::is_same_v<AddConstToPointerT<int32_t>, int32_t>);

        public:
            using Type = std::tuple<AddConstToPointerT<std::decay_t<Patterns>>...>;

        private:
            Type mPatterns;
        };

        template <typename... Patterns>
        constexpr auto ds(Patterns const &...patterns) -> Ds<Patterns...>
        {
            return Ds<Patterns...>{patterns...};
        }

        template <typename T>
        class Span
        {
            T const *mData;
            size_t mSize;
        public:
            Span(T const * data, size_t size)
            : mData{data}
            , mSize{size}
            {}
            
            T const * data() const
            {
                return mData;
            }
            size_t size() const
            {
                return mSize;
            }
            constexpr T const& operator[] (size_t idx) const
            {
                assert(idx < mSize);
                return mData[idx];
            }
        };

        template <typename T>
        constexpr auto makeSpan(T const *data, size_t size)
        {
            return Span<T>{data, size};
        }

        template <typename T>
        bool operator==(Span<T> const &lhs, Span<T> const &rhs)
        {
            return lhs.size() == rhs.size() && std::equal(lhs.data(), lhs.data() + lhs.size(), rhs.data());
        }

        template <typename T>
        class OooBinder
        {
            Id<T> mId;

        public:
            OooBinder(Id<T> const &id)
                : mId{id}
            {
            }
            decltype(auto) binder() const
            {
                return mId;
            }
        };

        class Ooo
        {
        public:
            template <typename T>
            constexpr auto operator()(Id<T> id) const
            {
                return OooBinder<T>{id};
            }
        };

        constexpr Ooo ooo;

        template <>
        class PatternTraits<Ooo>
        {
        public:
            template <typename Value>
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&, Ooo, int32_t /*depth*/, ContextT &)
            {
                return true;
            }
            constexpr static void processIdImpl(Ooo, int32_t /*depth*/, IdProcess)
            {
            }
        };

        template <typename Pattern>
        class PatternTraits<OooBinder<Pattern>>
        {
        public:
            template <typename Value>
            using AppResultTuple = typename PatternTraits<Pattern>::template AppResultTuple<Value>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, OooBinder<Pattern> const &oooBinderPat, int32_t depth, ContextT &context)
            {
                return matchPattern(std::forward<Value>(value), oooBinderPat.binder(), depth + 1, context);
            }
            constexpr static void processIdImpl(OooBinder<Pattern> const &oooBinderPat, int32_t depth, IdProcess idProcess)
            {
                processId(oooBinderPat.binder(), depth, idProcess);
            }
        };

        template <typename T>
        class IsOoo : public std::false_type
        {
        };

        template <>
        class IsOoo<Ooo> : public std::true_type
        {
        };

        template <typename T>
        class IsOooBinder : public std::false_type
        {
        };

        template <typename T>
        class IsOooBinder<OooBinder<T>> : public std::true_type
        {
        };

        template <typename T>
        constexpr auto isOooBinderV = IsOooBinder<std::decay_t<T>>::value;

        template <typename T>
        constexpr auto isOooOrBinderV = IsOoo<std::decay_t<T>>::value || isOooBinderV<T>;

        template <typename... Patterns>
        constexpr auto nbOooOrBinderV = ((isOooOrBinderV<Patterns> ? 1 : 0) + ...);

        static_assert(nbOooOrBinderV<int32_t &, Ooo const &, char const *, Wildcard, Ooo const> == 2);

        template <typename Tuple, std::size_t... I>
        constexpr size_t findOooIdxImpl(std::index_sequence<I...>)
        {
            return ((isOooOrBinderV<decltype(get<I>(std::declval<Tuple>()))> ? I : 0) + ...);
        }

        template <typename Tuple>
        constexpr size_t findOooIdx()
        {
            return findOooIdxImpl<Tuple>(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
        }

        static_assert(isOooOrBinderV<Ooo>);
        static_assert(isOooOrBinderV<OooBinder<int>>);
        static_assert(findOooIdx<std::tuple<int, OooBinder<int>, const char *>>() == 1);
        static_assert(findOooIdx<std::tuple<int, Ooo, const char *>>() == 1);

        using std::get;
        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t... I, typename ValueTuple, typename PatternTuple, typename ContextT>
        constexpr decltype(auto) matchPatternMultipleImpl(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth, ContextT &context, std::index_sequence<I...>)
        {
            auto const func = [&](auto &&value, auto &&pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1, context);
            };
            static_cast<void>(func);
            return (func(get<I + valueStartIdx>(std::forward<ValueTuple>(valueTuple)), std::get<I + patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t size, typename ValueTuple, typename PatternTuple, typename ContextT>
        constexpr decltype(auto) matchPatternMultiple(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth, ContextT &context)
        {
            return matchPatternMultipleImpl<valueStartIdx, patternStartIdx>(
                std::forward<ValueTuple>(valueTuple), patternTuple, depth, context, std::make_index_sequence<size>{});
        }

        template <std::size_t patternStartIdx, std::size_t... I, typename ValueVec, typename PatternTuple, typename ContextT>
        constexpr decltype(auto) matchPatternVecImpl(ValueVec &&valueVec, std::size_t valueStartIdx, PatternTuple &&patternTuple, int32_t depth, ContextT &context, std::index_sequence<I...>)
        {
            auto const func = [&](auto &&value, auto &&pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1, context);
            };
            static_cast<void>(func);
            return (func(std::forward<ValueVec>(valueVec).at(I + valueStartIdx), std::get<I + patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t patternStartIdx, std::size_t size, typename ValueVec, typename PatternTuple, typename ContextT>
        constexpr decltype(auto) matchPatternVec(ValueVec &&valueVec, std::size_t valueStartIdx, PatternTuple &&patternTuple, int32_t depth, ContextT &context)
        {
            return matchPatternVecImpl<patternStartIdx>(
                std::forward<ValueVec>(valueVec), valueStartIdx, patternTuple, depth, context, std::make_index_sequence<size>{});
        }

        template <std::size_t start, typename Indices, typename Tuple>
        class IndexedTypes;

        template <typename Tuple, std::size_t start, std::size_t... I>
        class IndexedTypes<start, std::index_sequence<I...>, Tuple>
        {
        public:
            using type = std::tuple<std::decay_t<decltype(std::get<start + I>(std::declval<Tuple>()))>...>;
        };

        template <std::size_t start, std::size_t end, class Tuple>
        class SubTypes
        {
            constexpr static auto tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple>>;
            static_assert(start <= end);
            static_assert(end <= tupleSize);

            using Indices = std::make_index_sequence<end - start>;

        public:
            using type = typename IndexedTypes<start, Indices, Tuple>::type;
        };

        template <std::size_t start, std::size_t end, class Tuple>
        using SubTypesT = typename SubTypes<start, end, Tuple>::type;

        static_assert(std::is_same_v<std::tuple<std::nullptr_t>, SubTypesT<3, 4, std::tuple<char, bool, int32_t, std::nullptr_t>>>);
        static_assert(std::is_same_v<std::tuple<char>, SubTypesT<0, 1, std::tuple<char, bool, int32_t, std::nullptr_t>>>);
        static_assert(std::is_same_v<std::tuple<>, SubTypesT<1, 1, std::tuple<char, bool, int32_t, std::nullptr_t>>>);
        static_assert(std::is_same_v<std::tuple<int32_t, std::nullptr_t>, SubTypesT<2, 4, std::tuple<char, bool, int32_t, std::nullptr_t>>>);

        template <typename ValueTuple>
        class IsArray : public std::false_type
        {
        };

        template <typename T, size_t s>
        class IsArray<std::array<T, s>> : public std::true_type
        {
        };

        template <typename ValueTuple>
        constexpr auto isArrayV = IsArray<std::decay_t<ValueTuple>>::value;

        template <typename... Patterns>
        class PatternTraits<Ds<Patterns...>>
        {
            constexpr static auto nbOooOrBinder = nbOooOrBinderV<Patterns...>;
            static_assert(nbOooOrBinder == 0 || nbOooOrBinder == 1);

        public:
            template <typename PsTuple, typename VsTuple>
            class PairPV;

            template <typename... Ps, typename... Vs>
            class PairPV<std::tuple<Ps...>, std::tuple<Vs...>>
            {
            public:
                using type = decltype(std::tuple_cat(std::declval<typename PatternTraits<Ps>::template AppResultTuple<Vs>>()...));
            };

            template <std::size_t nbOoos, typename ValueTuple>
            class AppResultForTupleHelper;

            template <typename... Values>
            class AppResultForTupleHelper<0, std::tuple<Values...>>
            {
            public:
                using type = decltype(std::tuple_cat(std::declval<typename PatternTraits<Patterns>::template AppResultTuple<Values>>()...));
            };

            template <typename... Values>
            class AppResultForTupleHelper<1, std::tuple<Values...>>
            {
                constexpr static auto idxOoo = findOooIdx<typename Ds<Patterns...>::Type>();
                // static_assert(!isOooBinderV<std::tuple_element_t<idxOoo, std::tuple<Patterns...>>>);
                using Ps0 = SubTypesT<0, idxOoo, std::tuple<Patterns...>>;
                using Vs0 = SubTypesT<0, idxOoo, std::tuple<Values...>>;
                constexpr static auto isBinder = isOooBinderV<std::tuple_element_t<idxOoo, std::tuple<Patterns...>>>;
                // <0, ...int32_t> to workaround compile failure for std::tuple<>.
                using OooResultTuple = typename std::conditional<isBinder, std::tuple<Span<typename std::tuple_element<0, std::tuple<std::decay_t<Values>..., int32_t>>::type>>, std::tuple<>>::type;
                using FirstHalfTuple = typename PairPV<Ps0, Vs0>::type;
                using Ps1 = SubTypesT<idxOoo + 1, sizeof...(Patterns), std::tuple<Patterns...>>;
                constexpr static auto diff = sizeof...(Values) - sizeof...(Patterns);
                using Vs1 = SubTypesT<idxOoo + 1 + diff, sizeof...(Values), std::tuple<Values...>>;
                using SecondHalfTuple = typename PairPV<Ps1, Vs1>::type;

            public:
                using type = decltype(std::tuple_cat(std::declval<FirstHalfTuple>(), std::declval<OooResultTuple>(), std::declval<SecondHalfTuple>()));
            };

            // TODO fix me.
            template <typename Tuple>
            using AppResultForTuple = typename AppResultForTupleHelper<nbOooOrBinder, decltype(drop<0>(std::declval<Tuple>()))>::type;

            template <typename Vector>
            using SpanTuple = std::conditional_t<nbOooOrBinder == 1, std::tuple<Span<typename Vector::value_type>>, std::tuple<>>;
            template <typename Vector>
            using AppResultForVector = decltype(std::tuple_cat(std::declval<SpanTuple<Vector>>(), std::declval<typename PatternTraits<Patterns>::template AppResultTuple<typename Vector::value_type>>()...));

            template <typename Value>
            class AppResultHelper
            {
            public:
                using type = AppResultForTuple<Value>;
            };

            template <typename... Args>
            class AppResultHelper<std::vector<Args...>>
            {
            public:
                using type = AppResultForVector<std::vector<Args...>>;
            };

            template <typename Value>
            using AppResultTuple = typename AppResultHelper<std::decay_t<Value>>::type;

            template <typename ValueTuple, typename ContextT>
            constexpr static auto matchPatternImpl(ValueTuple &&valueTuple, Ds<Patterns...> const &dsPat, int32_t depth, ContextT &context)
                -> decltype(std::tuple_size<std::decay_t<ValueTuple>>::value, bool{})
            {
                if constexpr (nbOooOrBinder == 0)
                {
                    return std::apply(
                        [&valueTuple, depth, &context](auto const &...patterns) {
                            return apply_(
                                [ depth, &context, &patterns... ](auto const &...values) constexpr {
                                    static_assert(sizeof...(patterns) == sizeof...(values));
                                    return (matchPattern(std::forward<decltype(values)>(values), patterns, depth + 1, context) && ...);
                                },
                                valueTuple);
                        },
                        dsPat.patterns());
                }
                else if constexpr (nbOooOrBinder == 1)
                {
                    constexpr auto idxOoo = findOooIdx<typename Ds<Patterns...>::Type>();
                    constexpr auto isBinder = isOooBinderV<std::tuple_element_t<idxOoo, std::tuple<Patterns...>>>;
                    constexpr auto isArray = isArrayV<ValueTuple>;
                    auto result = matchPatternMultiple<0, 0, idxOoo>(std::forward<ValueTuple>(valueTuple), dsPat.patterns(), depth, context);
                    constexpr auto valLen = std::tuple_size_v<std::decay_t<ValueTuple>>;
                    constexpr auto patLen = sizeof...(Patterns);
                    if constexpr (isArray)
                    {
                        if constexpr (isBinder)
                        {
                            auto const spanSize = valLen - (patLen - 1);
                            context.emplace_back(makeSpan(&valueTuple[idxOoo], spanSize));
                            using type = decltype(makeSpan(&valueTuple[idxOoo], spanSize));
                            result = result &&
                                     matchPattern(std::get<type>(context.back()), std::get<idxOoo>(dsPat.patterns()), depth, context);
                        }
                    }
                    else
                    {
                        static_assert(!isBinder);
                    }
                    return result && matchPatternMultiple<valLen - patLen + idxOoo + 1, idxOoo + 1, patLen - idxOoo - 1>(std::forward<ValueTuple>(valueTuple), dsPat.patterns(), depth, context);
                }
            }

            template <typename ValueVec, typename ContextT>
            constexpr static auto matchPatternImpl(ValueVec &&valueVec, Ds<Patterns...> const &dsPat, int32_t depth, ContextT &context)
                -> decltype(std::declval<ValueVec>().capacity(), bool{})
            {
                constexpr auto nbOooOrBinder = nbOooOrBinderV<Patterns...>;
                static_assert(nbOooOrBinder == 0 || nbOooOrBinder == 1);
                constexpr auto nbPat = sizeof...(Patterns);

                if constexpr (nbOooOrBinder == 0)
                {
                    // size mismatch for dynamic array is not an error;
                    if (valueVec.size() != nbPat)
                    {
                        return false;
                    }
                    return matchPatternVec<0, nbPat>(std::forward<ValueVec>(valueVec), 0, dsPat.patterns(), depth, context);
                }
                else if constexpr (nbOooOrBinder == 1)
                {
                    if (valueVec.size() < nbPat - 1)
                    {
                        return false;
                    }
                    constexpr auto idxOoo = findOooIdx<typename Ds<Patterns...>::Type>();
                    constexpr auto isBinder = isOooBinderV<std::tuple_element_t<idxOoo, std::tuple<Patterns...>>>;
                    auto result = matchPatternVec<0, idxOoo>(std::forward<ValueVec>(valueVec), 0, dsPat.patterns(), depth, context);
                    auto const valLen = valueVec.size();
                    constexpr auto patLen = sizeof...(Patterns);
                    if constexpr (isBinder)
                    {
                        auto const spanSize = valLen - (patLen - 1);
                        context.emplace_back(makeSpan(&valueVec[idxOoo], spanSize));
                        using type = decltype(makeSpan(&valueVec[idxOoo], spanSize));
                        result = result &&
                                 matchPattern(std::get<type>(context.back()), std::get<idxOoo>(dsPat.patterns()), depth, context);
                    }
                    return result &&
                           matchPatternVec<idxOoo + 1, patLen - idxOoo - 1>(std::forward<ValueVec>(valueVec), valLen - patLen + idxOoo + 1, dsPat.patterns(), depth, context);
                }
            }

            constexpr static void processIdImpl(Ds<Patterns...> const &dsPat, int32_t depth, IdProcess idProcess)
            {
                return std::apply(
                    [depth, idProcess](auto &&...patterns) {
                        return (processId(patterns, depth, idProcess), ...);
                    },
                    dsPat.patterns());
            }
        };

        template <typename Pattern, typename Pred>
        class PostCheck
        {
        public:
            constexpr explicit PostCheck(Pattern const &pattern, Pred const &pred)
                : mPattern{pattern}, mPred{pred}
            {
            }
            constexpr bool check() const
            {
                return mPred();
            }
            constexpr auto const &pattern() const
            {
                return mPattern;
            }

        private:
            Pattern const mPattern;
            Pred const mPred;
        };

        template <typename Pattern, typename Pred>
        class PatternTraits<PostCheck<Pattern, Pred>>
        {
        public:
            template <typename Value>
            using AppResultTuple = typename PatternTraits<Pattern>::template AppResultTuple<Value>;

            template <typename Value, typename ContextT>
            constexpr static auto matchPatternImpl(Value &&value, PostCheck<Pattern, Pred> const &postCheck, int32_t depth, ContextT &context)
            {
                return matchPattern(std::forward<Value>(value), postCheck.pattern(), depth + 1, context) && postCheck.check();
            }
            constexpr static void processIdImpl(PostCheck<Pattern, Pred> const &postCheck, int32_t depth, IdProcess idProcess)
            {
                processId(postCheck.pattern(), depth, idProcess);
            }
        };

        static_assert(std::is_same_v<PatternTraits<Wildcard>::template AppResultTuple<int32_t>, std::tuple<>>);
        static_assert(std::is_same_v<PatternTraits<int32_t>::template AppResultTuple<int32_t>, std::tuple<>>);
        constexpr auto x = [](auto &&t) { return t; };
        static_assert(std::is_same_v<PatternTraits<App<decltype(x), Wildcard>>::template AppResultTuple<int32_t>, std::tuple<>>);
        static_assert(std::is_same_v<PatternTraits<App<decltype(x), Wildcard>>::template AppResultTuple<std::array<int32_t, 3>>, std::tuple<std::array<int32_t, 3>>>);
        static_assert(std::is_same_v<PatternTraits<And<App<decltype(x), Wildcard>>>::template AppResultTuple<int32_t>, std::tuple<>>);

        template <typename Value, typename... PatternPairs>
        constexpr auto matchPatterns(Value &&value, PatternPairs const &...patterns)
        {
            using RetType = typename PatternPairsRetType<PatternPairs...>::RetType;
            using TypeTuple = decltype(std::tuple_cat(std::declval<typename PatternTraits<typename PatternPairs::PatternT>::template AppResultTuple<Value>>()...));

            // expression, has return value.
            if constexpr (!std::is_same_v<RetType, void>)
            {
                constexpr auto const func = [](auto const &pattern, auto &&value, RetType &result) constexpr->bool
                {
                    auto context = typename ContextTrait<TypeTuple>::ContextT{};
                    if (pattern.matchValue(std::forward<Value>(value), context))
                    {
                        result = pattern.execute();
                        processId(pattern, 0, IdProcess::kCANCEL);
                        return true;
                    }
                    return false;
                };
                RetType result{};
                bool const matched = (func(patterns, value, result) || ...);
                assert(matched);
                static_cast<void>(matched);
                return result;
            }
            else
            // statement, no return value, mismatching all patterns is not an error.
            {
                auto const func = [](auto const &pattern, auto &&value) -> bool {
                    auto context = typename ContextTrait<TypeTuple>::ContextT{};
                    if (pattern.matchValue(std::forward<Value>(value), context))
                    {
                        pattern.execute();
                        processId(pattern, 0, IdProcess::kCANCEL);
                        return true;
                    }
                    return false;
                };
                bool const matched = (func(patterns, value) || ...);
                static_cast<void>(matched);
            }
        }

    } // namespace impl

    // export symbols
    using impl::_;
    using impl::and_;
    using impl::app;
    using impl::ds;
    using impl::Id;
    using impl::meet;
    using impl::not_;
    using impl::ooo;
    using impl::or_;
    using impl::pattern;
    using impl::Span;
} // namespace matchit

#endif // MATCHIT_PATTERNS_H
#ifndef MATCHIT_UTILITY_H
#define MATCHIT_UTILITY_H

#include <variant>
#include <any>

namespace matchit
{
    namespace impl
    {

        template <typename T>
        constexpr auto cast = [](auto &&input) {
            return static_cast<T>(input);
        };

        constexpr auto deref = [](auto &&x) -> decltype(*x)& { return *x; };
        constexpr auto some = [](auto const pat) {
            return and_(app(cast<bool>, true), app(deref, pat));
        };

        constexpr auto none = app(cast<bool>, false);

        template <typename T>
        class AsPointerBase
        {
        public:
            template <typename B>
            constexpr auto operator()(B const &b) const
            {
                return dynamic_cast<T const *>(std::addressof(b));
            }
            template <typename... Types>
            constexpr auto operator()(std::variant<Types...> const &v) const
            {
                return std::get_if<T>(std::addressof(v));
            }
            constexpr auto operator()(std::any const &a) const
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

        template <typename T>
        constexpr AsPointer<T> asPointer;
        template <typename T>
        constexpr auto as = [](auto const pat) {
            return app(asPointer<T>, some(pat));
        };

        template <typename Value, typename Pattern>
        constexpr auto matched(Value&& v, Pattern&& p)
        {
            return match(std::forward<Value>(v))(
                pattern(std::forward<Pattern>(p)) = [] { return true; },
                pattern(_) = [] { return false; });
        }

    } // namespace impl
    using impl::as;
    using impl::none;
    using impl::some;
    using impl::matched;
} // namespace matchit

#endif // MATCHIT_UTILITY_H