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

        enum class IdProcess
        {
            kCANCEL,
            kCONFIRM
        };

        template <typename Pattern>
        void processId(Pattern const &pattern, int32_t depth, IdProcess idProcess)
        {
            PatternTraits<Pattern>::processIdImpl(pattern, depth, idProcess);
        }

        template <typename Value, typename Pattern>
        auto matchPattern(Value &&value, Pattern const &pattern, int32_t depth = 0)
        {
            auto const result = PatternTraits<Pattern>::matchPatternImpl(std::forward<Value>(value), pattern, depth);
            auto const process = result ? IdProcess::kCONFIRM : IdProcess::kCANCEL;
            processId(pattern, depth, process);
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
                processId(mPattern, 0, IdProcess::kCANCEL);
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
            static void processIdImpl(Pattern const &, int32_t /*depth*/, IdProcess)
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
            static void processIdImpl(Pattern const &, int32_t /*depth*/, IdProcess)
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
            static void processIdImpl(Or<Patterns...> const &orPat, int32_t depth, IdProcess idProcess)
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
            static void processIdImpl(Meet<Pred> const &, int32_t /*depth*/, IdProcess)
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
            static void processIdImpl(App<Unary, Pattern> const &appPat, int32_t depth, IdProcess idProcess)
            {
                return processId(appPat.pattern(), depth, idProcess);
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
            static void processIdImpl(And<Patterns...> const &andPat, int32_t depth, IdProcess idProcess)
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
            static void processIdImpl(Not<Pattern> const &notPat, int32_t depth, IdProcess idProcess)
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
            template <typename Type, typename Value, std::enable_if_t<CanReset<Type, Value>::value> * = nullptr>
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
            auto matchValue(Value &&value) const
            {
                if (*mValue)
                {
                    return **mValue == value;
                }
                IdTrait::matchValueImpl(*mValue, std::forward<Value>(value));
                return true;
            }
            void reset(int32_t depth) const
            {
                if (*mDepth - depth >= 0)
                {
                    (*mValue).reset();
                    *mDepth = depth;
                }
            }
            void confirm(int32_t depth) const
            {
                if (*mDepth > depth || *mDepth == 0)
                {
                    assert(depth == *mDepth - 1 || depth == *mDepth || *mDepth == 0);
                    *mDepth = depth;
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
            static auto matchPatternImpl(Value &&value, Id<Type> const &idPat, int32_t /* depth */)
            {
                return idPat.matchValue(std::forward<Value>(value));
            }
            static void processIdImpl(Id<Type> const &idPat, int32_t depth, IdProcess idProcess)
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

        template <typename T>
        class Span
        {
        public:
            T const* mData;
            size_t mSize;
        };

        template <typename T>
        auto makeSpan(T const* data, size_t size)
        {
            return Span<T>{data, size};
        }

        template <typename T>
        bool operator==(Span<T> const &lhs, Span<T> const &rhs)
        {
            return lhs.mData == rhs.mData && lhs.mSize == rhs.mSize;
        }

        template <typename T>
        class OooBinder
        {
            Id<T> mId;
        public:
            OooBinder(Id<T> const& id)
            : mId{id}
            {}
            decltype(auto) binder() const
            {
                return mId;
            }
        };

        class Ooo
        {
        public:
            template <typename T>
            auto operator()(Id<T> id) const
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
            static auto matchPatternImpl(Value &&, Ooo, int32_t /*depth*/)
            {
                return true;
            }
            static void processIdImpl(Ooo, int32_t /*depth*/, IdProcess)
            {
            }
        };

        template <typename Pattern>
        class PatternTraits<OooBinder<Pattern> >
        {
        public:
            template <typename Value>
            static auto matchPatternImpl(Value &&value, OooBinder<Pattern> const &oooBinderPat, int32_t depth)
            {
                return matchPattern(std::forward<Value>(value), oooBinderPat.binder(), depth + 1);
            }
            static void processIdImpl(OooBinder<Pattern> const &oooBinderPat, int32_t depth, IdProcess idProcess)
            {
                processId(oooBinderPat.binder(), depth, idProcess);
            }
        };

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

        static_assert(findIdx<Ooo, std::tuple<int, Ooo, const char *> >() == 1);

        using std::get;
        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t... I, typename ValueTuple, typename PatternTuple>
        decltype(auto) matchPatternMultipleImpl(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth, std::index_sequence<I...>)
        {
            auto const func = [&](auto &&value, auto &&pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1);
            };
            static_cast<void>(func);
            return (func(get<I + valueStartIdx>(valueTuple), std::get<I + patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t size, typename ValueTuple, typename PatternTuple>
        decltype(auto) matchPatternMultiple(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth)
        {
            return matchPatternMultipleImpl<valueStartIdx, patternStartIdx>(
                valueTuple, patternTuple, depth, std::make_index_sequence<size>{});
        }

        template <std::size_t patternStartIdx, std::size_t... I, typename ValueVec, typename PatternTuple>
        decltype(auto) matchPatternVecImpl(ValueVec &&valueVec, std::size_t valueStartIdx, PatternTuple &&patternTuple, int32_t depth, std::index_sequence<I...>)
        {
            auto const func = [&](auto &&value, auto &&pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1);
            };
            static_cast<void>(func);
            return (func(valueVec.at(I + valueStartIdx), std::get<I + patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t patternStartIdx, std::size_t size, typename ValueVec, typename PatternTuple>
        decltype(auto) matchPatternVec(ValueVec &&valueVec, std::size_t valueStartIdx, PatternTuple &&patternTuple, int32_t depth)
        {
            return matchPatternVecImpl<patternStartIdx>(
                valueVec, valueStartIdx, patternTuple, depth, std::make_index_sequence<size>{});
        }

        template <typename T>
        class IsOoo : public std::false_type {};

        template <>
        class IsOoo<Ooo> : public std::true_type {};

        template <typename T>
        class IsOoo<OooBinder<T>> : public std::true_type {};

        template <typename T>
        auto constexpr isOooV = IsOoo<T>::value;

        template <typename... Patterns>
        auto constexpr nbOooV = ((isOooV<std::decay_t<Patterns> > ? 1 : 0) + ...);

        static_assert(nbOooV<int32_t&, Ooo const&, char const *, Wildcard, Ooo const> == 2);
                                
        template <typename... Patterns>
        class PatternTraits<Ds<Patterns...> >
        {
        public:
            template <typename ValueTuple>
            static auto matchPatternImpl(ValueTuple &&valueTuple, Ds<Patterns...> const &dsPat, int32_t depth)
            -> decltype(std::tuple_size<std::decay_t<ValueTuple>>::value, bool{})
            {
                auto constexpr nbOoo = nbOooV<Patterns...>;
                static_assert(nbOoo == 0 || nbOoo == 1);

                if constexpr (nbOoo == 0)
                {
                    return std::apply(
                        [&valueTuple, depth](auto const &...patterns) {
                            return apply_(
                                [depth, &patterns...](auto const &...values) {

                                    static_assert(sizeof...(patterns) == sizeof...(values));
                                    return (matchPattern(std::forward<decltype(values)>(values), patterns, depth + 1) && ...);
                                },
                                valueTuple);
                        },
                        dsPat.patterns());
                }
                else if constexpr (nbOoo == 1)
                {
                    auto constexpr idxOoo = findIdx<Ooo, typename Ds<Patterns...>::Type>();
                    auto result = matchPatternMultiple<0, 0, idxOoo>(valueTuple, dsPat.patterns(), depth);
                    auto constexpr valLen = std::tuple_size_v<std::decay_t<ValueTuple>>;
                    auto constexpr patLen = sizeof...(Patterns);
                    return result && matchPatternMultiple<valLen - patLen + idxOoo + 1, idxOoo + 1, patLen - idxOoo - 1>(valueTuple, dsPat.patterns(), depth);
                }
            }

            template <typename ValueVec>
            static auto matchPatternImpl(ValueVec &&valueVec, Ds<Patterns...> const &dsPat, int32_t depth)
            -> decltype(std::declval<ValueVec>().capacity(), bool{})
            {
                auto constexpr nbOoo = nbOooV<Patterns...>;
                static_assert(nbOoo == 0 || nbOoo == 1);
                auto constexpr nbPat = sizeof...(Patterns);

                if constexpr (nbOoo == 0)
                {
                    // size mismatch for dynamic array is not an error;
                    if (valueVec.size() != nbPat)
                    {
                        return false;
                    }
                    return matchPatternVec<0, nbPat>(std::forward<ValueVec>(valueVec), 0, dsPat.patterns(), depth);
                }
                else if constexpr (nbOoo == 1)
                {
                    if (valueVec.size() < nbPat - 1)
                    {
                        return false;
                    }
                    auto constexpr idxOoo = findIdx<Ooo, typename Ds<Patterns...>::Type>();
                    auto result = matchPatternVec<0, idxOoo>(std::forward<ValueVec>(valueVec), 0, dsPat.patterns(), depth);
                    auto const valLen = valueVec.size();
                    auto constexpr patLen = sizeof...(Patterns);
                    auto const spanSize = valLen - (patLen - 1);
                    return result &&
                           matchPattern(makeSpan(&valueVec[idxOoo], spanSize), std::get<idxOoo>(dsPat.patterns())) &&
                           matchPatternVec<idxOoo + 1, patLen - idxOoo - 1>(std::forward<ValueVec>(valueVec), valLen - patLen + idxOoo + 1, dsPat.patterns(), depth);
                }
            }

            static void processIdImpl(Ds<Patterns...> const &dsPat, int32_t depth, IdProcess idProcess)
            {
                return std::apply(
                    [depth, idProcess](auto &&...patterns) {
                        return (processId(patterns, depth, idProcess), ...);
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
            static void processIdImpl(PostCheck<Pattern, Pred> const &postCheck, int32_t depth, IdProcess idProcess)
            {
                processId(postCheck.pattern(), depth, idProcess);
            }
        };

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
    using impl::Span;
    using impl::makeSpan;
} // namespace matchit

#endif // _PATTERNS_H_
