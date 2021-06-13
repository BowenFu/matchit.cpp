#ifndef _PATTERNS_H_
#define _PATTERNS_H_

#include <memory>
#include <tuple>
#include <functional>
#include <vector>
#include <variant>
#include <type_traits>

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
        class PrependUnique<T, std::tuple<Ts...> >
        {
            auto static constexpr unique = !WithinTypes<T, Ts...>::value;

        public:
            using type = std::conditional_t<unique, std::tuple<T, Ts...>, std::tuple<Ts...> >;
        };

        template <typename T, typename Tuple>
        using PrependUniqueT = typename PrependUnique<T, Tuple>::type;

        template <typename Tuple>
        class Unique;

        template <typename Tuple>
        using UniqueT = typename Unique<Tuple>::type;

        template <>
        class Unique<std::tuple<> >
        {
        public:
            using type = std::tuple<>;
        };

        template <typename T, typename... Ts>
        class Unique<std::tuple<T, Ts...> >
        {
        public:
            using type = PrependUniqueT<T, UniqueT<std::tuple<Ts...> > >;
        };

        static_assert(std::is_same_v<std::tuple<int32_t>, UniqueT<std::tuple<int32_t, int32_t> > >);
        static_assert(std::is_same_v<std::tuple<std::tuple<>, int32_t>, UniqueT<std::tuple<int32_t, std::tuple<>, int32_t> > >);

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
            auto constexpr tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple> >;
            static_assert(start <= end);
            static_assert(end <= tupleSize);
            return detail::subtupleImpl<start>(
                std::forward<Tuple>(t),
                std::make_index_sequence<end - start>{});
        }

        template <std::size_t start, class Tuple>
        constexpr decltype(auto) drop(Tuple &&t)
        {
            auto constexpr tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple> >;
            static_assert(start <= tupleSize);
            return subtuple<start, tupleSize>(std::forward<Tuple>(t));
        }

        template <std::size_t len, class Tuple>
        constexpr decltype(auto) take(Tuple &&t)
        {
            auto constexpr tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple> >;
            static_assert(len <= tupleSize);
            return subtuple<0, len>(std::forward<Tuple>(t));
        }

        template <typename Pattern>
        class PatternTraits;

        template <typename... PatternPair>
        class PatternPairsRetType
        {
        public:
            using RetType = std::common_type_t<typename PatternPair::RetType...>;
        };

        enum class IdProcess : int32_t
        {
            kCANCEL,
            kCONFIRM
        };

        template <typename Pattern>
        void processId(Pattern const &pattern, int32_t depth, IdProcess idProcess)
        {
            PatternTraits<Pattern>::processIdImpl(pattern, depth, idProcess);
        }

        template <typename... Ts>
        class Context
        {
        public:
            std::vector<std::variant<std::monostate, Ts...> > mMemHolder;
        };

        template <typename T>
        class ContextTrait;

        template <typename... Ts>
        class ContextTrait<std::tuple<Ts...> >
        {
        public:
            using ContextT = Context<Ts...>;
        };

        template <typename Pattern>
        class ScopeGuard
        {
        public:
            ScopeGuard(Pattern &pattern)
                : mPattern{pattern}
            {
            }
            ScopeGuard(ScopeGuard const &) = delete;
            ScopeGuard(ScopeGuard &&) = delete;
            auto operator=(ScopeGuard const &) = delete;
            auto operator=(ScopeGuard &&) = delete;
            ~ScopeGuard()
            {
                processId(mPattern, 0, IdProcess::kCANCEL);
            }

        private:
            Pattern &mPattern;
        };

        template <typename Value, typename Pattern, typename ConctextT>
        auto matchPattern(Value &&value, Pattern const &pattern, int32_t depth, ConctextT &context)
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

            PatternPair(Pattern const &pattern, Func const &func)
                : mPattern{pattern}, mHandler{func}
            {
            }
            template <typename Value, typename ContextT>
            bool matchValue(Value &&value, ContextT &context) const
            {
                return matchPattern(std::forward<Value>(value), mPattern, /*depth*/ 0, context);
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
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, Pattern const &pattern, int32_t /* depth */, ContextT & /*context*/)
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
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            static bool matchPatternImpl(Value &&, Pattern const &, int32_t, ContextT &)
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
            using AppResultTuple = decltype(std::tuple_cat(typename PatternTraits<Patterns>::template AppResultTuple<Value>{}...));

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, Or<Patterns...> const &orPat, int32_t depth, ContextT &context)
            {
                constexpr auto patSize = sizeof...(Patterns);
                return std::apply(
                           [&value, depth, &context](auto const &...patterns) {
                               return (matchPattern(value, patterns, depth + 1, context) || ...);
                           },
                           take<patSize - 1>(orPat.patterns())) ||
                       matchPattern(std::forward<Value>(value), get<patSize - 1>(orPat.patterns()), depth + 1, context);
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
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, Meet<Pred> const &meetPat, int32_t /* depth */, ContextT &)
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

        auto constexpr y = 1;
        static_assert(std::holds_alternative<int32_t const *>(std::variant<std::monostate, const int32_t *>{&y}));

        // Debug<std::decay_t<const int32_t*>> z;

        template <typename Unary, typename Pattern>
        class PatternTraits<App<Unary, Pattern> >
        {
            template <typename Value>
            using AppResult = std::invoke_result_t<Unary, Value>;
            template <typename Value>
            using AppResultCurTuple = std::conditional_t<std::is_lvalue_reference_v<AppResult<Value> >, std::tuple<>, std::tuple<AppResult<Value> > >;

        public:
            template <typename Value>
            using AppResultTuple = decltype(std::tuple_cat(std::declval<AppResultCurTuple<Value> >(), std::declval<typename PatternTraits<Pattern>::template AppResultTuple<AppResult<Value> > >()));

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, App<Unary, Pattern> const &appPat, int32_t depth, ContextT &context)
            {
                if constexpr (std::is_lvalue_reference_v<AppResult<Value> >)
                {
                    return matchPattern(std::invoke(appPat.unary(), value), appPat.pattern(), depth + 1, context);
                }
                else
                {
                    context.mMemHolder.emplace_back(std::invoke(appPat.unary(), value));
                    decltype(auto) result = std::get<std::decay_t<AppResult<Value> > >(context.mMemHolder.back());
                    return matchPattern(std::forward<AppResult<Value>>(result), appPat.pattern(), depth + 1, context);
                }
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
            using AppResultTuple = decltype(std::tuple_cat(std::declval<typename PatternTraits<Patterns>::template AppResultTuple<Value> >()...));

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, And<Patterns...> const &andPat, int32_t depth, ContextT &context)
            {
                constexpr auto patSize = sizeof...(Patterns);
                return std::apply(
                           [&value, depth, &context](auto const &...patterns) {
                               return (matchPattern(value, patterns, depth + 1, context) && ...);
                           },
                           take<patSize - 1>(andPat.patterns())) &&
                       matchPattern(std::forward<Value>(value), get<patSize - 1>(andPat.patterns()), depth + 1, context);
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
            using AppResultTuple = typename PatternTraits<Pattern>::template AppResultTuple<Value>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, Not<Pattern> const &notPat, int32_t depth, ContextT &context)
            {
                return !matchPattern(std::forward<Value>(value), notPat.pattern(), depth + 1, context);
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
        struct CanRef : std::false_type
        {
        };

        template <typename Type>
        using ValueVariant = std::conditional_t<std::is_abstract_v<Type>, std::variant<std::monostate, Type const *>, std::variant<std::monostate, Type, Type const *> >;

        template <typename Type, typename Value>
        struct CanRef<Type, Value, std::void_t<decltype(std::declval<ValueVariant<Type>&>() = &std::declval<Value>())> >
            : std::negation<std::is_rvalue_reference<Value>>
        {
        };

        static_assert(CanRef<char, char &>::value);
        static_assert(CanRef<const char, char &>::value);
        static_assert(CanRef<const char, const char &>::value);
        static_assert(CanRef<std::unique_ptr<int32_t> const, std::unique_ptr<int32_t> const &>::value);
        static_assert(CanRef<std::tuple<int &, int &> const, std::tuple<int &, int &> const &>::value);
        static_assert(!CanRef<std::unique_ptr<int32_t>, std::unique_ptr<int32_t>&&>::value);

        template <typename... Ts>
        class Overload : public Ts...
        {
        public:
            using Ts::operator()...;
        };

        template <typename... Ts>
        auto overload(Ts &&...ts)
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
                ValueVariant<Type> mValue;
                int32_t mDepth;
                auto hasValue() const
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
                        mValue);
                }
                decltype(auto) value() const
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
                                return *reinterpret_cast<Type const*>(1);
                            }),
                        mValue);
                }

                decltype(auto) mutableValue()
                {
                    return std::visit(
                        overload(
                            [](Type &v) -> Type & {
                                return v;
                            },
                            [](Type const *) -> Type & {
                                assert(false && "Cannot get mutableValue for pointer type!");
                                return *reinterpret_cast<Type*>(1);
                            },
                            [](std::monostate &) -> Type & {
                                assert(false && "invalid state!");
                                return *reinterpret_cast<Type*>(1);
                            }),
                        mValue);
                }
                void reset(int32_t depth)
                {
                    if (mDepth - depth >= 0)
                    {
                        mValue = {};
                        mDepth = depth;
                    }
                }
                void confirm(int32_t depth)
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
                static auto matchValueImpl(ValueVariant<Type> &v, Value &&value, std::false_type /* CanRef */)
                {
                    v = std::forward<Value>(value);
                }
                template <typename Value>
                static auto matchValueImpl(ValueVariant<Type> &v, Value &&value, std::true_type /* CanRef */)
                {
                    v = &value;
                }
            };

            mutable std::shared_ptr<Block> mBlock = std::make_shared<Block>();

        public:
            template <typename Value>
            auto matchValue(Value &&v) const
            {
                if (hasValue())
                {
                    return value() == v;
                }
                IdTrait::matchValueImpl(mBlock->mValue, std::forward<Value>(v), CanRef<Type, Value>{});
                return true;
            }
            void reset(int32_t depth) const
            {
                return mBlock->reset(depth);
            }
            void confirm(int32_t depth) const
            {
                return mBlock->confirm(depth);
            }
            bool hasValue() const
            {
                return mBlock->hasValue();
            }
            Type const &value() const
            {
                return mBlock->value();
            }
            Type const &operator*() const
            {
                return value();
            }
            Type &&move()
            {
                return std::move(mBlock->mutableValue());
            }
        };

        template <typename Type>
        class PatternTraits<Id<Type> >
        {
        public:
            template <typename Value>
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, Id<Type> const &idPat, int32_t /* depth */, ContextT &)
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
            T const *mData;
            size_t mSize;
        };

        template <typename T>
        auto makeSpan(T const *data, size_t size)
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
            using AppResultTuple = std::tuple<>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&, Ooo, int32_t /*depth*/, ContextT &)
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
            using AppResultTuple = typename PatternTraits<Pattern>::template AppResultTuple<Value>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, OooBinder<Pattern> const &oooBinderPat, int32_t depth, ContextT &context)
            {
                return matchPattern(std::forward<Value>(value), oooBinderPat.binder(), depth + 1, context);
            }
            static void processIdImpl(OooBinder<Pattern> const &oooBinderPat, int32_t depth, IdProcess idProcess)
            {
                processId(oooBinderPat.binder(), depth, idProcess);
            }
        };

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
        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t... I, typename ValueTuple, typename PatternTuple, typename ContextT>
        decltype(auto) matchPatternMultipleImpl(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth, ContextT &context, std::index_sequence<I...>)
        {
            auto const func = [&](auto &&value, auto &&pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1, context);
            };
            static_cast<void>(func);
            return (func(get<I + valueStartIdx>(std::forward<ValueTuple>(valueTuple)), std::get<I + patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t valueStartIdx, std::size_t patternStartIdx, std::size_t size, typename ValueTuple, typename PatternTuple, typename ContextT>
        decltype(auto) matchPatternMultiple(ValueTuple &&valueTuple, PatternTuple &&patternTuple, int32_t depth, ContextT &context)
        {
            return matchPatternMultipleImpl<valueStartIdx, patternStartIdx>(
                std::forward<ValueTuple>(valueTuple), patternTuple, depth, context, std::make_index_sequence<size>{});
        }

        template <std::size_t patternStartIdx, std::size_t... I, typename ValueVec, typename PatternTuple, typename ContextT>
        decltype(auto) matchPatternVecImpl(ValueVec &&valueVec, std::size_t valueStartIdx, PatternTuple &&patternTuple, int32_t depth, ContextT &context, std::index_sequence<I...>)
        {
            auto const func = [&](auto &&value, auto &&pattern) {
                return matchPattern(std::forward<decltype(value)>(value), pattern, depth + 1, context);
            };
            static_cast<void>(func);
            return (func(std::forward<ValueVec>(valueVec).at(I + valueStartIdx), std::get<I + patternStartIdx>(patternTuple)) && ...);
        }

        template <std::size_t patternStartIdx, std::size_t size, typename ValueVec, typename PatternTuple, typename ContextT>
        decltype(auto) matchPatternVec(ValueVec &&valueVec, std::size_t valueStartIdx, PatternTuple &&patternTuple, int32_t depth, ContextT &context)
        {
            return matchPatternVecImpl<patternStartIdx>(
                std::forward<ValueVec>(valueVec), valueStartIdx, patternTuple, depth, context, std::make_index_sequence<size>{});
        }

        template <typename T>
        class IsOoo : public std::false_type
        {
        };

        template <>
        class IsOoo<Ooo> : public std::true_type
        {
        };

        template <typename T>
        class IsOoo<OooBinder<T> > : public std::true_type
        {
        };

        template <typename T>
        auto constexpr isOooV = IsOoo<T>::value;

        template <typename... Patterns>
        auto constexpr nbOooV = ((isOooV<std::decay_t<Patterns> > ? 1 : 0) + ...);

        static_assert(nbOooV<int32_t &, Ooo const &, char const *, Wildcard, Ooo const> == 2);

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
            auto constexpr static tupleSize = std::tuple_size_v<std::remove_reference_t<Tuple> >;
            static_assert(start <= end);
            static_assert(end <= tupleSize);

            using Indices = std::make_index_sequence<end - start>;

        public:
            using type = typename IndexedTypes<start, Indices, Tuple>::type;
        };

        template <std::size_t start, std::size_t end, class Tuple>
        using SubTypesT = typename SubTypes<start, end, Tuple>::type;

        static_assert(std::is_same_v<std::tuple<std::nullptr_t>, SubTypesT<3, 4, std::tuple<char, bool, int32_t, std::nullptr_t> > >);
        static_assert(std::is_same_v<std::tuple<char>, SubTypesT<0, 1, std::tuple<char, bool, int32_t, std::nullptr_t> > >);
        static_assert(std::is_same_v<std::tuple<>, SubTypesT<1, 1, std::tuple<char, bool, int32_t, std::nullptr_t> > >);
        static_assert(std::is_same_v<std::tuple<int32_t, std::nullptr_t>, SubTypesT<2, 4, std::tuple<char, bool, int32_t, std::nullptr_t> > >);

        template <typename... Patterns>
        class PatternTraits<Ds<Patterns...> >
        {
            auto constexpr static nbOoo = nbOooV<Patterns...>;
            static_assert(nbOoo == 0 || nbOoo == 1);

        public:
            template <typename PsTuple, typename VsTuple>
            class PairPV;

            template <typename... Ps, typename... Vs>
            class PairPV<std::tuple<Ps...>, std::tuple<Vs...> >
            {
            public:
                using type = decltype(std::tuple_cat(std::declval<typename PatternTraits<Ps>::template AppResultTuple<Vs> >()...));
            };

            template <std::size_t nbOoos, typename ValueTuple>
            class AppResultForTupleHelper;

            template <typename... Values>
            class AppResultForTupleHelper<0, std::tuple<Values...> >
            {
            public:
                using type = decltype(std::tuple_cat(std::declval<typename PatternTraits<Patterns>::template AppResultTuple<Values> >()...));
            };

            template <typename... Values>
            class AppResultForTupleHelper<1, std::tuple<Values...> >
            {
                auto constexpr static idxOoo = findIdx<Ooo, typename Ds<Patterns...>::Type>();
                using Ps0 = SubTypesT<0, idxOoo, std::tuple<Patterns...> >;
                using Vs0 = SubTypesT<0, idxOoo, std::tuple<Values...> >;
                using FirstHalfTuple = typename PairPV<Ps0, Vs0>::type;
                using Ps1 = SubTypesT<idxOoo + 1, sizeof...(Patterns), std::tuple<Patterns...> >;
                constexpr static auto diff = sizeof...(Values) - sizeof...(Patterns);
                using Vs1 = SubTypesT<idxOoo + 1 + diff, sizeof...(Values), std::tuple<Values...> >;
                using SecondHalfTuple = typename PairPV<Ps1, Vs1>::type;

            public:
                using type = decltype(std::tuple_cat(std::declval<FirstHalfTuple>(), std::declval<SecondHalfTuple>()));
            };

            // TODO fix me.
            template <typename Tuple>
            using AppResultForTuple = typename AppResultForTupleHelper<nbOoo, decltype(drop<0>(std::declval<Tuple>()))>::type;

            template <typename Vector>
            using SpanTuple = std::conditional_t<nbOoo == 1, std::tuple<Span<typename Vector::value_type> >, std::tuple<> >;
            template <typename Vector>
            using AppResultForVector = decltype(std::tuple_cat(std::declval<SpanTuple<Vector> >(), std::declval<typename PatternTraits<Patterns>::template AppResultTuple<typename Vector::value_type> >()...));

            template <typename Value>
            class AppResultHelper
            {
            public:
                using type = AppResultForTuple<Value>;
            };

            template <typename... Args>
            class AppResultHelper<std::vector<Args...> >
            {
            public:
                using type = AppResultForVector<std::vector<Args...> >;
            };

            template <typename Value>
            using AppResultTuple = typename AppResultHelper<std::decay_t<Value> >::type;

            template <typename ValueTuple, typename ContextT>
            static auto matchPatternImpl(ValueTuple &&valueTuple, Ds<Patterns...> const &dsPat, int32_t depth, ContextT &context)
                -> decltype(std::tuple_size<std::decay_t<ValueTuple> >::value, bool{})
            {
                if constexpr (nbOoo == 0)
                {
                    return std::apply(
                        [&valueTuple, depth, &context](auto const &...patterns) {
                            return apply_(
                                [depth, &context, &patterns...](auto const &...values) {
                                    static_assert(sizeof...(patterns) == sizeof...(values));
                                    return (matchPattern(std::forward<decltype(values)>(values), patterns, depth + 1, context) && ...);
                                },
                                valueTuple);
                        },
                        dsPat.patterns());
                }
                else if constexpr (nbOoo == 1)
                {
                    auto constexpr idxOoo = findIdx<Ooo, typename Ds<Patterns...>::Type>();
                    auto result = matchPatternMultiple<0, 0, idxOoo>(std::forward<ValueTuple>(valueTuple), dsPat.patterns(), depth, context);
                    auto constexpr valLen = std::tuple_size_v<std::decay_t<ValueTuple> >;
                    auto constexpr patLen = sizeof...(Patterns);
                    return result && matchPatternMultiple<valLen - patLen + idxOoo + 1, idxOoo + 1, patLen - idxOoo - 1>(std::forward<ValueTuple>(valueTuple), dsPat.patterns(), depth, context);
                }
            }

            template <typename ValueVec, typename ContextT>
            static auto matchPatternImpl(ValueVec &&valueVec, Ds<Patterns...> const &dsPat, int32_t depth, ContextT &context)
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
                    return matchPatternVec<0, nbPat>(std::forward<ValueVec>(valueVec), 0, dsPat.patterns(), depth, context);
                }
                else if constexpr (nbOoo == 1)
                {
                    if (valueVec.size() < nbPat - 1)
                    {
                        return false;
                    }
                    auto constexpr idxOoo = findIdx<Ooo, typename Ds<Patterns...>::Type>();
                    auto result = matchPatternVec<0, idxOoo>(std::forward<ValueVec>(valueVec), 0, dsPat.patterns(), depth, context);
                    auto const valLen = valueVec.size();
                    auto constexpr patLen = sizeof...(Patterns);
                    auto const spanSize = valLen - (patLen - 1);
                    // FIXME, add to AppResultTuple
                    context.mMemHolder.emplace_back(makeSpan(&valueVec[idxOoo], spanSize));
                    using type = decltype(makeSpan(&valueVec[idxOoo], spanSize));
                    return result &&
                            // can forward span
                           matchPattern(std::get<type>(context.mMemHolder.back()), std::get<idxOoo>(dsPat.patterns()), depth, context) &&
                           matchPatternVec<idxOoo + 1, patLen - idxOoo - 1>(std::forward<ValueVec>(valueVec), valLen - patLen + idxOoo + 1, dsPat.patterns(), depth, context);
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
            using AppResultTuple = typename PatternTraits<Pattern>::template AppResultTuple<Value>;

            template <typename Value, typename ContextT>
            static auto matchPatternImpl(Value &&value, PostCheck<Pattern, Pred> const &postCheck, int32_t depth, ContextT &context)
            {
                return matchPattern(std::forward<Value>(value), postCheck.pattern(), depth + 1, context) && postCheck.check();
            }
            static void processIdImpl(PostCheck<Pattern, Pred> const &postCheck, int32_t depth, IdProcess idProcess)
            {
                processId(postCheck.pattern(), depth, idProcess);
            }
        };

        static_assert(std::is_same_v<PatternTraits<Wildcard>::template AppResultTuple<int32_t>, std::tuple<> >);
        static_assert(std::is_same_v<PatternTraits<int32_t>::template AppResultTuple<int32_t>, std::tuple<> >);
        auto constexpr x = [](int32_t) -> int32_t { return 0; };
        static_assert(std::is_same_v<PatternTraits<App<decltype(x), Wildcard> >::template AppResultTuple<int32_t>, std::tuple<int32_t> >);
        static_assert(std::is_same_v<PatternTraits<And<App<decltype(x), Wildcard> > >::template AppResultTuple<int32_t>, std::tuple<int32_t> >);

        template <typename Value, typename... Patterns>
        using TypeSetTuple = UniqueT<decltype(std::tuple_cat(std::declval<typename PatternTraits<Patterns>::template AppResultTuple<Value> >()...))>;

        template <typename Value, typename... PatternPairs>
        auto matchPatterns(Value &&value, PatternPairs const &...patterns)
        {
            using RetType = typename PatternPairsRetType<PatternPairs...>::RetType;
            using TypeSetT = TypeSetTuple<Value, typename PatternPairs::PatternT...>;

            if constexpr (!std::is_same_v<RetType, void>)
            {
                RetType result{};
                auto const func = [&result, &value](auto const &pattern) -> bool {
                    auto context = typename ContextTrait<TypeSetT>::ContextT{};
                    ScopeGuard<decltype(pattern)> guard{pattern};
                    if (pattern.matchValue(std::forward<Value>(value), context))
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
            else
            {
                auto const func = [&value](auto const &pattern) -> bool {
                    auto context = typename ContextTrait<TypeSetT>::ContextT{};
                    ScopeGuard<decltype(pattern)> guard{pattern};
                    if (pattern.matchValue(std::forward<Value>(value), context))
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

#endif // _PATTERNS_H_
