#ifndef _PATTERNS_H_
#define _PATTERNS_H_

#include <memory>
#include <tuple>
#include <functional>

namespace matchit
{
    namespace impl
    {
        template <typename Pattern>
        class PatternTraits;

        template <typename Pattern>
        void resetId(Pattern const &pattern, int32_t depth = 0)
        {
            PatternTraits<Pattern>::resetIdImpl(pattern, depth);
        }

        template <typename Value, typename Pattern>
        auto matchPattern(Value &&value, Pattern const &pattern, int32_t depth = 0)
            -> decltype(PatternTraits<Pattern>::matchPatternImpl(std::forward<Value>(value), pattern, depth))
        {
            auto const result = PatternTraits<Pattern>::matchPatternImpl(std::forward<Value>(value), pattern, depth);
            if (!result)
            {
                resetId(pattern, depth);
            }
            return result;
        }

        template <typename Pattern, typename Func>
        class PatternPair
        {
        public:
            using RetType = std::invoke_result_t<Func>;

            PatternPair(Pattern const &pattern, Func const &func)
                : mPattern{pattern}, mHandler{func}
            {
            }
            template <typename Value>
            bool matchValue(Value &&value) const
            {
                resetId(mPattern);
                return matchPattern(std::forward<Value>(value), mPattern);
            }
            auto execute() const
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
            explicit PatternHelper(Pattern const &pattern)
                : mPattern{pattern}
            {
            }
            template <typename Func>
            auto operator=(Func const &func)
            {
                return PatternPair<Pattern, Func>{mPattern, func};
            }
            template <typename Pred>
            auto when(Pred const &pred)
            {
                return PatternHelper<PostCheck<Pattern, Pred> >(PostCheck(mPattern, pred));
            }

        private:
            Pattern const mPattern;
        };

        template <typename Pattern>
        auto pattern(Pattern const &p)
        {
            return PatternHelper<std::decay_t<Pattern> >{p};
        }

        template <typename... Patterns>
        class Ds;

        template <typename... Patterns>
        auto ds(Patterns const &...patterns) -> Ds<Patterns...>;

        template <typename First, typename... Patterns>
        auto pattern(First const &f, Patterns const &...ps)
        {
            return PatternHelper<Ds<First, Patterns...> >{ds(f, ps...)};
        }

        template <typename Pattern>
        class PatternTraits
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, Pattern const &pattern, int32_t /* depth */)
                -> decltype(pattern == std::forward<Value>(value))
            {
                return pattern == std::forward<Value>(value);
            }
            static void resetIdImpl(Pattern const &, int32_t /*depth*/)
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
            static bool matchPatternImpl(Value &&, Pattern const &, int32_t)
            {
                return true;
            }
            static void resetIdImpl(Pattern const &, int32_t /*depth*/)
            {
            }
        };

        template <typename... Patterns>
        class Or
        {
        public:
            explicit Or(Patterns const &...patterns)
                : mPatterns{patterns...}
            {
            }
            auto const &patterns() const
            {
                return mPatterns;
            }

        private:
            std::tuple<Patterns...> mPatterns;
        };

        template <typename... Patterns>
        auto or_(Patterns const &...patterns)
        {
            return Or<Patterns...>{patterns...};
        }

        template <typename... Patterns>
        class PatternTraits<Or<Patterns...> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, Or<Patterns...> const &orPat, int32_t depth)
                -> decltype((matchPattern(std::forward<Value>(value), std::declval<Patterns>(), depth + 1) || ...))
            {
                return std::apply(
                    [&value, depth](Patterns const &...patterns) {
                        return (matchPattern(std::forward<Value>(value), patterns, depth + 1) || ...);
                    },
                    orPat.patterns());
            }
            static void resetIdImpl(Or<Patterns...> const &orPat, int32_t depth)
            {
                return std::apply(
                    [depth](Patterns const &...patterns) {
                        return (resetId(patterns, depth), ...);
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
        auto meet(Pred const &pred)
        {
            return Meet<Pred>{pred};
        }

        template <typename Pred>
        class PatternTraits<Meet<Pred> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, Meet<Pred> const &meetPat, int32_t /* depth */)
                -> decltype(meetPat(std::forward<Value>(value)))
            {
                return meetPat(std::forward<Value>(value));
            }
            static void resetIdImpl(Meet<Pred> const &, int32_t /*depth*/)
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
            auto const &unary() const
            {
                return mUnary;
            }
            auto const &pattern() const
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

        template <typename Unary, typename Pattern>
        class PatternTraits<App<Unary, Pattern> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, App<Unary, Pattern> const &appPat, int32_t depth)
                -> decltype(matchPattern(std::invoke(appPat.unary(), std::forward<Value>(value)), appPat.pattern(), depth + 1))
            {
                return matchPattern(std::invoke(appPat.unary(), std::forward<Value>(value)), appPat.pattern(), depth + 1);
            }
            static void resetIdImpl(App<Unary, Pattern> const &appPat, int32_t depth)
            {
                return resetId(appPat.pattern(), depth);
            }
        };

        template <typename... Patterns>
        class And
        {
        public:
            explicit And(Patterns const &...patterns)
                : mPatterns{patterns...}
            {
            }
            auto const &patterns() const
            {
                return mPatterns;
            }

        private:
            std::tuple<Patterns...> mPatterns;
        };

        template <typename... Patterns>
        auto and_(Patterns const &...patterns)
        {
            return And<Patterns...>{patterns...};
        }

        template <typename... Patterns>
        class PatternTraits<And<Patterns...> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, And<Patterns...> const &andPat, int32_t depth)
                -> decltype((matchPattern(std::forward<Value>(value), std::declval<Patterns>(), depth + 1) && ...))
            {
                return std::apply(
                    [&value, depth](Patterns const &...patterns) {
                        return (matchPattern(std::forward<Value>(value), patterns, depth + 1) && ...);
                    },
                    andPat.patterns());
            }
            static void resetIdImpl(And<Patterns...> const &andPat, int32_t depth)
            {
                return std::apply(
                    [depth](Patterns const &...patterns) {
                        return (resetId(patterns, depth), ...);
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
        auto not_(Pattern const &pattern)
        {
            return Not<Pattern>{pattern};
        }

        template <typename Pattern>
        class PatternTraits<Not<Pattern> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, Not<Pattern> const &notPat, int32_t depth)
                -> decltype(!matchPattern(std::forward<Value>(value), notPat.pattern(), depth + 1))
            {
                return !matchPattern(std::forward<Value>(value), notPat.pattern(), depth + 1);
            }
            static void resetIdImpl(Not<Pattern> const &notPat, int32_t depth)
            {
                resetId(notPat.pattern(), depth);
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

        template <typename Ptr, typename Value, typename = std::void_t<> >
        struct CanReset : std::false_type
        {
        };

        template <typename Type, typename Value>
        struct CanReset<Type, Value, std::void_t<decltype(std::declval<std::unique_ptr<Type, Deleter> >().reset(&std::declval<Value>()))> >
            : std::true_type
        {
        };

        template <typename T>
        class Debug;

        // Debug<decltype(&std::declval<char&>())> x{};

        template <typename Type, typename Value>
        using t = decltype(
            std::declval<std::unique_ptr<Type, Deleter> >().reset(&std::declval<Value>()));

        static_assert(CanReset<const char, char &>::value);
        static_assert(CanReset<const char, const char &>::value);

        class IdTrait
        {
        public:
            template <typename Type, typename Value, std::enable_if_t<!CanReset<Type, Value>::value> * = nullptr>
            static auto matchValueImpl(std::unique_ptr<Type, Deleter> &ptr, Value &&value)
                -> decltype(ptr = std::unique_ptr<Type, Deleter>(new Type{std::forward<Value>(value)}, Deleter{true}), void())
            {
                ptr = std::unique_ptr<Type, Deleter>(new Type{std::forward<Value>(value)}, Deleter{true});
            }
            template <typename Type, typename Value, std::enable_if_t<CanReset<Type, Value>::value>* = nullptr>
            static auto matchValueImpl(std::unique_ptr<Type, Deleter> &ptr, Value &&value)
            {
                ptr.reset(&value);
                ptr.get_deleter().mOwn = false;
            }
        };

        template <typename Type>
        class Id
        {
            using PtrT = std::unique_ptr<Type const, Deleter>;
            mutable std::shared_ptr<PtrT> mValue = std::make_shared<PtrT>();
            mutable std::shared_ptr<int32_t> mDepth = std::make_shared<int32_t>(0);

        public:
            template <typename Value>
            auto matchValue(Value &&value, int32_t depth) const
                -> decltype(**mValue == value, IdTrait::matchValueImpl(*mValue, std::forward<Value>(value)), bool{})
            {
                if (*mValue)
                {
                    return **mValue == value;
                }
                IdTrait::matchValueImpl(*mValue, std::forward<Value>(value));
                *mDepth = depth;
                return true;
            }
            void reset(int32_t depth) const
            {
                if (*mDepth >= depth)
                {
                    (*mValue).reset();
                }
            }
            Type const &value() const
            {
                assert(*mValue);
                return **mValue;
            }
            bool hasValue() const
            {
                return *mValue != nullptr;
            }
            Type const &operator*() const
            {
                return value();
            }
        };

        template <typename Type>
        class PatternTraits<Id<Type> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, Id<Type> const &idPat, int32_t depth)
                -> decltype(idPat.matchValue(std::forward<Value>(value), depth))
            {
                return idPat.matchValue(std::forward<Value>(value), depth);
            }
            static void resetIdImpl(Id<Type> const &idPat, int32_t depth)
            {
                idPat.reset(depth);
            }
        };

        template <typename... Patterns>
        class Ds
        {
        public:
            explicit Ds(Patterns const &...patterns)
                : mPatterns{patterns...}
            {
            }
            auto const &patterns() const
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
                    std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T> > > >;
            };
            template <typename T>
            using AddConstToPointerT = typename AddConstToPointer<T>::type;

            static_assert(std::is_same_v<AddConstToPointerT<void *>, void const *>);
            static_assert(std::is_same_v<AddConstToPointerT<int32_t>, int32_t>);

        public:
            using Type = std::tuple<AddConstToPointerT<std::decay_t<Patterns> >...>;

        private:
            Type mPatterns;
        };

        template <typename... Patterns>
        auto ds(Patterns const &...patterns) -> Ds<Patterns...>
        {
            return Ds<Patterns...>{patterns...};
        }

        template <typename Value, typename Pattern, typename = std::void_t<> >
        struct MatchFuncDefined : std::false_type
        {
        };

        template <typename Value, typename Pattern>
        struct MatchFuncDefined<Value, Pattern, std::void_t<decltype(matchPattern(std::declval<Value>(), std::declval<Pattern>()))> >
            : std::true_type
        {
        };

        template <typename Value, typename Pattern>
        inline constexpr bool MatchFuncDefinedV = MatchFuncDefined<Value, Pattern>::value;

        template <typename ValuesTuple, typename PatternsTuple>
        bool tryOooMatch(ValuesTuple const &values, PatternsTuple const &patterns, int32_t depth);

        template <typename Pattern>
        class IsOoo;

        template <typename Pattern>
        inline constexpr bool isOooV = IsOoo<std::decay_t<Pattern> >::value;

        template <typename ValuesTuple, typename PatternsTuple, typename Enable = void>
        class TupleMatchHelper
        {
            template <typename VT = ValuesTuple>
            static bool tupleMatchImpl(VT &&values, PatternsTuple const &patterns, int32_t /*depth*/) = delete;
        };

        using std::get;
        template <std::size_t N, typename Tuple, std::size_t... I>
        auto dropImpl(Tuple &&t, std::index_sequence<I...>)
        {
            // Fixme, use std::forward_as_tuple when possible.
            return std::forward_as_tuple(get<I + N>(std::forward<Tuple>(t))...);
        }

        // std::tuple_size_v cannot work with SFINAE with GCC, use std::tuple_size<>::value instead.
        template <std::size_t N, typename Tuple>
        auto drop(Tuple &&t)
            -> decltype(dropImpl<N>(
                std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple> >::value - N>{}))
        {
            return dropImpl<N>(
                std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple> >::value - N>{});
        }

        template <typename ValuesTuple, typename PatternHead, typename... PatternTail>
        class TupleMatchHelper<ValuesTuple, std::tuple<PatternHead, PatternTail...>, std::enable_if_t<!isOooV<PatternHead> > >
        {
        public:
            template <typename VT = ValuesTuple>
            static auto tupleMatchImpl(VT &&values, std::tuple<PatternHead, PatternTail...> const &patterns, int32_t depth)
                -> decltype(matchPattern(get<0>(std::forward<VT>(values)), get<0>(patterns), depth + 1) && TupleMatchHelper<decltype(drop<1>(values)), decltype(drop<1>(patterns))>::tupleMatchImpl(drop<1>(values), drop<1>(patterns), depth))
            {
                return matchPattern(get<0>(std::forward<VT>(values)), get<0>(patterns), depth + 1) && TupleMatchHelper<decltype(drop<1>(std::forward<VT>(values))), decltype(drop<1>(patterns))>::tupleMatchImpl(drop<1>(std::forward<VT>(values)), drop<1>(patterns), depth);
            }
        };

        template <typename PatternHead, typename... PatternTail>
        class TupleMatchHelper<std::tuple<>, std::tuple<PatternHead, PatternTail...>, std::enable_if_t<!isOooV<PatternHead> > >
        {
        public:
            template <typename VT = std::tuple<> >
            static bool tupleMatchImpl(VT &&values, std::tuple<PatternHead, PatternTail...> const &patterns, int32_t /* depth */) = delete;
        };

        template <typename ValuesTuple>
        class TupleMatchHelper<ValuesTuple, std::tuple<> >
        {
        public:
            template <typename VT = std::tuple<> >
            static auto tupleMatchImpl(VT &&, std::tuple<>, int32_t /*depth*/)
            {
                return false;
            }
        };

        template <typename ValuesTuple, typename PatternHead, typename... PatternTail>
        class TupleMatchHelper<ValuesTuple, std::tuple<PatternHead, PatternTail...>, std::enable_if_t<isOooV<PatternHead> > >
        {
        public:
            template <typename VT = std::tuple<> >
            static auto tupleMatchImpl(VT const&values, std::tuple<PatternHead, PatternTail...> const &patterns, int32_t depth)
                -> decltype(tryOooMatch(values, patterns, depth))
            {
                return tryOooMatch(values, patterns, depth);
            }
        };

        template <>
        class TupleMatchHelper<std::tuple<>, std::tuple<> >
        {
        public:
            template <typename VT = std::tuple<> >
            static auto tupleMatchImpl(VT&&, std::tuple<>, int32_t /*depth*/)
            {
                return true;
            }
        };

        template <typename... Patterns>
        class PatternTraits<Ds<Patterns...> >
        {
        public:
            template <typename ValueTuple>
            static auto matchPatternImpl(ValueTuple &&valueTuple, Ds<Patterns...> const &dsPat, int32_t depth)
                -> decltype(TupleMatchHelper<ValueTuple, typename Ds<Patterns...>::Type>::tupleMatchImpl(std::forward<ValueTuple>(valueTuple), dsPat.patterns(), depth))
            {
                return TupleMatchHelper<ValueTuple, typename Ds<Patterns...>::Type>::tupleMatchImpl(std::forward<ValueTuple>(valueTuple), dsPat.patterns(), depth);
            }
            static void resetIdImpl(Ds<Patterns...> const &dsPat, int32_t depth)
            {
                return std::apply(
                    [depth](auto &&...patterns) {
                        return (resetId(patterns, depth), ...);
                    },
                    dsPat.patterns());
            }

        private:
        };

        template <typename Pattern>
        class Ooo;

        template <typename Pattern>
        class IsOoo : public std::false_type
        {
        };

        template <typename Pattern>
        class IsOoo<Ooo<Pattern> > : public std::true_type
        {
        };

        static_assert(isOooV<Ooo<int> > == true);
        static_assert(isOooV<Ooo<int &> > == true);
        static_assert(isOooV<Ooo<int const &> > == true);
        static_assert(isOooV<Ooo<int &&> > == true);
        static_assert(isOooV<int> == false);
        static_assert(isOooV<const Ooo<Wildcard> &> == true);

        template <typename Tuple, std::size_t... I>
        auto takeImpl(Tuple &&t, std::index_sequence<I...>)
        {
            return std::forward_as_tuple(get<I>(std::forward<Tuple>(t))...);
        }

        template <std::size_t N, typename Tuple>
        auto take(Tuple &&t)
        {
            return takeImpl(
                std::forward<Tuple>(t),
                std::make_index_sequence<N>{});
        }

        template <typename ValuesTuple, typename PatternsTuple>
        bool tryOooMatch(ValuesTuple const &values, PatternsTuple const &patterns, int32_t depth)
        {
            if constexpr (std::tuple_size_v<PatternsTuple> == 0)
            {
                return std::tuple_size_v<ValuesTuple> == 0;
            }
            else if constexpr (isOooV<std::tuple_element_t<0, PatternsTuple> >)
            {
                auto index = std::make_index_sequence<std::tuple_size_v<ValuesTuple> + 1>{};
                return tryOooMatchImpl(values, patterns, depth, index);
            }
            else if constexpr (std::tuple_size_v<ValuesTuple> >= 1)
            {
                if constexpr (MatchFuncDefinedV<std::tuple_element_t<0, ValuesTuple>, std::tuple_element_t<0, PatternsTuple> >)
                {
                    return matchPattern(get<0>(values), std::get<0>(patterns), depth + 1) && tryOooMatch(drop<1>(values), drop<1>(patterns), depth);
                }
            }
            return false;
        }

        class OooMatchBreak : public std::exception
        {
        };

        template <std::size_t I, typename ValuesTuple, typename PatternsTuple>
        bool tryOooMatchImplHelper(ValuesTuple const &values, PatternsTuple const &patterns, int32_t depth)
        {
            if constexpr (I == 0)
            {
                return (tryOooMatch(values, drop<1>(patterns), depth));
            }
            else if constexpr (I > 0)
            {
                if constexpr (MatchFuncDefinedV<decltype(take<I>(values)), std::tuple_element_t<0, PatternsTuple> >)
                {
                    if (!PatternTraits<std::decay_t<std::tuple_element_t<0, PatternsTuple>> >::matchPatternImplSingle(get<I - 1>(values), get<0>(patterns), depth))
                    {
                        throw OooMatchBreak();
                    }

                    if (!tryOooMatch(drop<I>(values), drop<1>(patterns), depth))
                    {
                        return false;
                    }
                    return true;
                }
            }
            throw OooMatchBreak();
        }

        template <typename ValuesTuple, typename PatternsTuple, std::size_t... I>
        bool tryOooMatchImpl(ValuesTuple const &values, PatternsTuple const &patterns, int32_t depth, std::index_sequence<I...>)
        {
            try
            {
                return ((tryOooMatchImplHelper<I>(values, patterns, depth)) || ...);
            }
            catch (const OooMatchBreak &)
            {
                return false;
            }
        }

        template <typename Pattern>
        class Ooo
        {
        public:
            explicit Ooo(Pattern const &pattern)
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
        auto ooo(Pattern const &pattern)
        {
            return Ooo<Pattern>{pattern};
        }

        // TODO: ooo does not support move to Id.
        template <typename Pattern>
        class PatternTraits<Ooo<Pattern> >
        {
        public:
            template <typename... Values>
            static auto matchPatternImpl(std::tuple<Values...> const &valueTuple, Ooo<Pattern> const &oooPat, int32_t depth)
                -> decltype((matchPattern(std::declval<Values>(), oooPat.pattern(), depth + 1) && ...))
            {
                return std::apply(
                    [&oooPat, depth](Values const &...values) {
                        auto result = (matchPattern(values, oooPat.pattern(), depth + 1) && ...);
                        return result;
                    },
                    valueTuple);
            }
            template <typename Value>
            static auto matchPatternImplSingle(Value &&value, Ooo<Pattern> const &oooPat, int32_t depth)
                -> decltype(matchPattern(std::forward<Value>(value), oooPat.pattern(), depth + 1))
            {
                return matchPattern(std::forward<Value>(value), oooPat.pattern(), depth + 1);
            }
            static void resetIdImpl(Ooo<Pattern> const &oooPat, int32_t depth)
            {
                resetId(oooPat.pattern(), depth);
            }
        };

        template <typename Pattern, typename Pred>
        class PostCheck
        {
        public:
            explicit PostCheck(Pattern const &pattern, Pred const &pred)
                : mPattern{pattern}, mPred{pred}
            {
            }
            bool check() const
            {
                return mPred();
            }
            auto const &pattern() const
            {
                return mPattern;
            }

        private:
            Pattern const mPattern;
            Pred const mPred;
        };

        template <typename Pattern, typename Pred>
        class PatternTraits<PostCheck<Pattern, Pred> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, PostCheck<Pattern, Pred> const &postCheck, int32_t depth)
                -> decltype(matchPattern(std::forward<Value>(value), postCheck.pattern(), depth + 1) && postCheck.check())
            {
                return matchPattern(std::forward<Value>(value), postCheck.pattern(), depth + 1) && postCheck.check();
            }
            static void resetIdImpl(PostCheck<Pattern, Pred> const &postCheck, int32_t depth)
            {
                resetId(postCheck.pattern(), depth);
            }
        };

        static_assert(MatchFuncDefinedV<char[4], Id<const char *> >);
        static_assert(MatchFuncDefinedV<std::tuple<>, Wildcard>);
        static_assert(MatchFuncDefinedV<std::tuple<>, Ds<> >);
        static_assert(!MatchFuncDefinedV<std::tuple<>, Ds<int> >);

        static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char>, int> &,
                                        const Ds<char, Ds<char, Id<char> >, int> &>);
        static_assert(!MatchFuncDefinedV<const int &, const Ds<char, Ds<char, Id<char> >, int> &>);

        static_assert(MatchFuncDefinedV<char, char>);
        static_assert(MatchFuncDefinedV<int, char>);
        static_assert(MatchFuncDefinedV<char, int>);

        static_assert(MatchFuncDefinedV<std::tuple<char>, Ds<char> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, int, std::tuple<char, int> >,
                                        Ds<char, int, Ds<char, int> > >);
        static_assert(MatchFuncDefinedV<std::tuple<char, int, std::tuple<char, std::tuple<char, char>, int> >,
                                        Ds<char, int, Ds<char, Ds<char, char>, int> > >);
        static_assert(MatchFuncDefinedV<std::tuple<char, int, std::tuple<char, std::tuple<char, char>, int> >,
                                        Ds<char, int, Ds<char, Ds<char, Id<char> >, int> > >);
        static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char>, int> &,
                                        const Ds<char, Ds<char, char>, int> &>);
        static_assert(MatchFuncDefinedV<char &,
                                        Id<char> >);
        static_assert(MatchFuncDefinedV<const std::tuple<char, char> &,
                                        const Ds<char, Id<char> > &>);
        static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char> > &,
                                        const Ds<char, Ds<char, Id<char> > > &>);
        static_assert(MatchFuncDefinedV<const std::tuple<std::tuple<char, char>, int> &,
                                        const Ds<Ds<char, Id<char> >, int> &>);
        static_assert(MatchFuncDefinedV<const std::tuple<int, std::tuple<char, char>, int> &,
                                        const Ds<int, Ds<char, Id<char> >, int> &>);
        static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char>, char> &,
                                        const Ds<char, Ds<char, Id<char> >, char> &>);
        static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<int, int>, int>,
                                        Ds<int, Ds<int, Id<int> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, char>, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, char>, Ds<char, Id<char> > >);
        static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<char, char>, int>, Ds<int, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int64_t>, Ds<char, Ds<char, Id<char> >, int64_t> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, long>, Ds<char, Ds<char, Id<char> >, long> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, unsigned>, Ds<char, Ds<char, Id<char> >, unsigned> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, unsigned> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, Wildcard> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, char> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, char>, Ds<char, Ds<char, Id<char> >, char> >);
        static_assert(MatchFuncDefinedV<std::tuple<std::tuple<char, std::tuple<char, char>, int> >, Ds<Ds<char, Ds<char, Id<char> >, int> > >);
        static_assert(MatchFuncDefinedV<std::tuple<char, char>, Ds<char, Id<char> > >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<int, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<Wildcard, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<char, char>);
        static_assert(MatchFuncDefinedV<std::tuple<char, char>, Ds<char, char> >);
        static_assert(MatchFuncDefinedV<std::tuple<char>, Ds<char> >);
        static_assert(!MatchFuncDefinedV<std::tuple<char>, char>);
        static_assert(MatchFuncDefinedV<std::tuple<char>, Ooo<char> >);
        static_assert(MatchFuncDefinedV<std::tuple<char>, Wildcard>);
        static_assert(MatchFuncDefinedV<std::tuple<std::tuple<char, char>, int>, Ds<Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<bool, std::tuple<char, char>, int>, Ds<bool, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<char, char>, int>, Ds<int, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<char, char>, char>, Ds<int, Ds<char, Id<char> >, char> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, char>, Ds<char, Ds<char, Id<char> >, char> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char> >, Ds<char, Ds<char, Id<char> > > >);
        static_assert(MatchFuncDefinedV<std::tuple<int, int, int, int, int>, Ds<Ooo<int> > >);
    } // namespace impl

    // export symbols
    using impl::_;
    using impl::and_;
    using impl::app;
    using impl::ds;
    using impl::Id;
    using impl::matchPattern;
    using impl::meet;
    using impl::not_;
    using impl::ooo;
    using impl::or_;
    using impl::pattern;
} // namespace matchit

#endif // _PATTERNS_H_
