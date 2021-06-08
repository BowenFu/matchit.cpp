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

        static_assert(CanReset<const char, char &>::value);
        static_assert(CanReset<const char, const char &>::value);

        class IdTrait
        {
        public:
            template <typename Type, typename Value, std::enable_if_t<!CanReset<Type, Value>::value> * = nullptr>
            static auto matchValueImpl(std::unique_ptr<Type, Deleter> &ptr, Value &&value)
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

        class Ooo
        {
        };

        constexpr Ooo ooo;

        using std::get;
        namespace detail
        {
            template <class F, class Tuple, std::size_t... I>
            constexpr decltype(auto) apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>)
            {
                // This implementation is valid since C++20 (via P1065R2)
                // In C++17, a constexpr counterpart of std::invoke is actually needed here
                return std::invoke(std::forward<F>(f), get<I>(std::forward<Tuple>(t))...);
            }
        } // namespace detail

        template <class F, class Tuple>
        constexpr decltype(auto) apply_(F &&f, Tuple &&t)
        {
            return detail::apply_impl(
                std::forward<F>(f), std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple> > >{});
        }

        template <typename T, typename Tuple, std::size_t... I>
        constexpr size_t findIdxImpl(std::index_sequence<I...>)
        {
            return ((std::is_same_v<std::decay_t<decltype(get<I>(std::declval<Tuple>()))>, T> ? I : 0) + ...);
        }

        template <typename T, typename Tuple>
        constexpr size_t findIdx()
        {
            return findIdxImpl<T, Tuple>(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple> > >{});
        }

        static_assert(findIdx<Ooo, std::tuple<int, Ooo, const char*>>() == 1);

        using std::get;
        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t... I, typename ValueTuple, typename PatternTuple>
        decltype(auto) matchPatternMultipleImpl(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth, std::index_sequence<I...>)
        {
            auto const func = [&](auto&& value, auto&& pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1);
            };
            return (func(get<I+valueStartIdx>(valueTuple), get<I+patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t size, typename ValueTuple, typename PatternTuple>
        decltype(auto) matchPatternMultiple(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth)
        {
            return matchPatternMultipleImpl<valueStartIdx, patternStartIdx>(
                valueTuple, patternTuple, depth, std::make_index_sequence<size>{});
        }

        template <typename... Patterns>
        class PatternTraits<Ds<Patterns...> >
        {
        public:
            template <typename ValueTuple>
            static auto matchPatternImpl(ValueTuple &&valueTuple, Ds<Patterns...> const &dsPat, int32_t depth)
            {
                return std::apply(
                    [&valueTuple, depth, &dsPat](auto const &...patterns) {
                        return apply_(
                            [depth, &patterns..., &valueTuple, &dsPat](auto const &...values) {
                                auto constexpr nbOoo = (std::is_same_v<Ooo, Patterns> + ...);
                                static_assert(nbOoo == 0 || nbOoo == 1);
                                if constexpr (nbOoo == 0)
                                {
                                    static_cast<void>(valueTuple);
                                    static_cast<void>(dsPat);

                                    return (matchPattern(std::forward<decltype(values)>(values), patterns, depth + 1) && ...);
                                }
                                else if constexpr (nbOoo == 1)
                                {
                                    (static_cast<void>(patterns), ...);
                                    auto constexpr idxOoo = findIdx<Ooo, typename Ds<Patterns...>::Type>();
                                    auto result = matchPatternMultiple<0, 0, idxOoo>(valueTuple, dsPat.patterns(), depth);
                                    auto constexpr valLen = sizeof...(values);
                                    auto constexpr patLen = sizeof...(patterns);
                                    return result && matchPatternMultiple<valLen - patLen + idxOoo + 1, idxOoo + 1, patLen - idxOoo - 1>(valueTuple, dsPat.patterns(), depth);
                                }
                            },
                            valueTuple);
                    },
                    dsPat.patterns());
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
            {
                return matchPattern(std::forward<Value>(value), postCheck.pattern(), depth + 1) && postCheck.check();
            }
            static void resetIdImpl(PostCheck<Pattern, Pred> const &postCheck, int32_t depth)
            {
                resetId(postCheck.pattern(), depth);
            }
        };

        // static_assert(MatchFuncDefinedV<char[4], Id<const char *> >);
        // static_assert(MatchFuncDefinedV<std::tuple<>, Wildcard>);
        // static_assert(MatchFuncDefinedV<std::tuple<>, Ds<> >);
        // static_assert(!MatchFuncDefinedV<std::tuple<>, Ds<int> >);

        // static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char>, int> &,
        //                                 const Ds<char, Ds<char, Id<char> >, int> &>);
        // static_assert(!MatchFuncDefinedV<const int &, const Ds<char, Ds<char, Id<char> >, int> &>);

        // static_assert(MatchFuncDefinedV<char, char>);
        // static_assert(MatchFuncDefinedV<int, char>);
        // static_assert(MatchFuncDefinedV<char, int>);

        // static_assert(MatchFuncDefinedV<std::tuple<char>, Ds<char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, int, std::tuple<char, int> >,
        //                                 Ds<char, int, Ds<char, int> > >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, int, std::tuple<char, std::tuple<char, char>, int> >,
        //                                 Ds<char, int, Ds<char, Ds<char, char>, int> > >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, int, std::tuple<char, std::tuple<char, char>, int> >,
        //                                 Ds<char, int, Ds<char, Ds<char, Id<char> >, int> > >);
        // static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char>, int> &,
        //                                 const Ds<char, Ds<char, char>, int> &>);
        // static_assert(MatchFuncDefinedV<char &,
        //                                 Id<char> >);
        // static_assert(MatchFuncDefinedV<const std::tuple<char, char> &,
        //                                 const Ds<char, Id<char> > &>);
        // static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char> > &,
        //                                 const Ds<char, Ds<char, Id<char> > > &>);
        // static_assert(MatchFuncDefinedV<const std::tuple<std::tuple<char, char>, int> &,
        //                                 const Ds<Ds<char, Id<char> >, int> &>);
        // static_assert(MatchFuncDefinedV<const std::tuple<int, std::tuple<char, char>, int> &,
        //                                 const Ds<int, Ds<char, Id<char> >, int> &>);
        // static_assert(MatchFuncDefinedV<const std::tuple<char, std::tuple<char, char>, char> &,
        //                                 const Ds<char, Ds<char, Id<char> >, char> &>);
        // static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<int, int>, int>,
        //                                 Ds<int, Ds<int, Id<int> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, char>, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, char>, Ds<char, Id<char> > >);
        // static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<char, char>, int>, Ds<int, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int64_t>, Ds<char, Ds<char, Id<char> >, int64_t> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, long>, Ds<char, Ds<char, Id<char> >, long> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, unsigned>, Ds<char, Ds<char, Id<char> >, unsigned> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, unsigned> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, Wildcard> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, char>, Ds<char, Ds<char, Id<char> >, char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<std::tuple<char, std::tuple<char, char>, int> >, Ds<Ds<char, Ds<char, Id<char> >, int> > >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, char>, Ds<char, Id<char> > >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<int, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<Wildcard, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<char, char>);
        // static_assert(MatchFuncDefinedV<std::tuple<char, char>, Ds<char, char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char>, Ds<char> >);
        // static_assert(!MatchFuncDefinedV<std::tuple<char>, char>);
        // static_assert(MatchFuncDefinedV<std::tuple<char>, Ooo<char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char>, Wildcard>);
        // static_assert(MatchFuncDefinedV<std::tuple<std::tuple<char, char>, int>, Ds<Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<bool, std::tuple<char, char>, int>, Ds<bool, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<char, char>, int>, Ds<int, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<int, std::tuple<char, char>, char>, Ds<int, Ds<char, Id<char> >, char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, char>, Ds<char, Ds<char, Id<char> >, char> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char>, int>, Ds<char, Ds<char, Id<char> >, int> >);
        // static_assert(MatchFuncDefinedV<std::tuple<char, std::tuple<char, char> >, Ds<char, Ds<char, Id<char> > > >);
        // static_assert(MatchFuncDefinedV<std::tuple<int, int, int, int, int>, Ds<Ooo<int> > >);
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
