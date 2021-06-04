#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

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
        class Unary : public T
        {
        public:
            using T::operator();
        };

        template <typename T>
        auto nullary(T t)
        {
            return Nullary<T>{t};
        }

        template <typename T>
        auto unary(T t)
        {
            return Unary<T>{t};
        }

        template <typename T, bool own>
        auto expr(Id<T, own> const &id)
        {
            return nullary([&] { return *id; });
        }

        template <typename T>
        auto expr(T const &v)
        {
            return nullary([&] { return v; });
        }

        // for constant
        template <typename T>
        class EvalTraits
        {
        public:
            template <typename... Args>
            constexpr static auto evalImpl(T const &v, Args const &...args)
            {
                return v;
            }
        };

        template <typename T>
        class EvalTraits<Nullary<T> >
        {
        public:
            constexpr static auto evalImpl(Nullary<T> const &e)
            {
                return e();
            }
        };

        // Only allowed in nullary
        template <typename T, bool own>
        class EvalTraits<Id<T, own> >
        {
        public:
            constexpr static auto evalImpl(Id<T, own> const &id)
            {
                return *id;
            }
        };

        template <typename T>
        class EvalTraits<Unary<T> >
        {
        public:
            template <typename Arg>
            constexpr static auto evalImpl(Unary<T> const &e, Arg const &arg)
            {
                return e(arg);
            }
        };

        class Placeholder;
        template <>
        class EvalTraits<Placeholder>
        {
        public:
            template <typename Arg>
            constexpr static auto evalImpl(Placeholder const &e, Arg const &arg)
            {
                return arg;
            }
        };

        template <typename T>
        class EvalTraits<Meet<T> >
        {
        public:
            template <typename Arg>
            constexpr static auto evalImpl(Meet<T> const &e, Arg const &arg)
            {
                return e.predicate()(arg);
            }
        };

        class Wildcard;
        template <>
        class EvalTraits<Wildcard>
        {
        public:
            template <typename Arg>
            constexpr static auto evalImpl(Wildcard const &e, Arg const &arg)
            {
                return arg;
            }
        };

        template <typename T, typename... Args>
        auto eval(T const &t, Args const &...args)
        {
            return EvalTraits<T>::evalImpl(t, args...);
        }

        template <typename T>
        class IsNullaryOrId : public std::false_type
        {
        };

        template <typename T, bool own>
        class IsNullaryOrId<Id<T, own> > : public std::true_type
        {
        };

        template <typename T>
        class IsNullaryOrId<Nullary<T> > : public std::true_type
        {
        };

#define UN_OP_FOR_NULLARY(op)                                                     \
    template <typename T, std::enable_if_t<IsNullaryOrId<T>::value, bool> = true> \
    auto operator op(T const &t)                                                  \
    {                                                                             \
        return nullary([&] { return op eval(t); });                               \
    }

#define BIN_OP_FOR_NULLARY(op)                                                                                           \
    template <typename T, typename U, std::enable_if_t<IsNullaryOrId<T>::value || IsNullaryOrId<U>::value, bool> = true> \
    auto operator op(T const &t, U const &u)                                                                             \
    {                                                                                                                    \
        return nullary([&] { return eval(t) op eval(u); });                                                              \
    }

        // ADL will find these operators.
        UN_OP_FOR_NULLARY(!)
        UN_OP_FOR_NULLARY(-)

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

        class Placeholder
        {
        };
        constexpr Placeholder _1;

        template <typename T>
        class IsUnaryOrPlaceholder : public std::false_type
        {
        };

        template <>
        class IsUnaryOrPlaceholder<Placeholder> : public std::true_type
        {
        };

        template <typename T>
        class IsUnaryOrPlaceholder<Unary<T> > : public std::true_type
        {
        };

#define UN_OP_FOR_UNARY(op)                                                              \
    template <typename T, std::enable_if_t<IsUnaryOrPlaceholder<T>::value, bool> = true> \
    auto operator op(T const &t)                                                         \
    {                                                                                    \
        return unary([&](auto &&arg) { return op eval(t, arg); });                       \
    }

#define BIN_OP_FOR_UNARY(op)                                                                                                           \
    template <typename T, typename U, std::enable_if_t<IsUnaryOrPlaceholder<T>::value || IsUnaryOrPlaceholder<U>::value, bool> = true> \
    auto operator op(T const &t, U const &u)                                                                                           \
    {                                                                                                                                  \
        return unary([&](auto &&arg) { return eval(t, arg) op eval(u, arg); });                                                        \
    }

        UN_OP_FOR_UNARY(!)
        UN_OP_FOR_UNARY(-)

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

        // Meet
        template <typename T>
        class IsMeetOrWildcard : public std::false_type
        {
        };

        template <>
        class IsMeetOrWildcard<Wildcard> : public std::true_type
        {
        };

        template <typename T>
        class IsMeetOrWildcard<Meet<T> > : public std::true_type
        {
        };

#define UN_OP_FOR_MEET(op)                                                              \
    template <typename T, std::enable_if_t<IsMeetOrWildcard<T>::value, bool> = true> \
    auto operator op(T const &t)                                                         \
    {                                                                                    \
        return meet([&](auto &&arg) { return op eval(t, arg); });                       \
    }

#define BIN_OP_FOR_MEET(op)                                                                                                           \
    template <typename T, typename U, std::enable_if_t<IsMeetOrWildcard<T>::value || IsMeetOrWildcard<U>::value, bool> = true> \
    auto operator op(T const &t, U const &u)                                                                                           \
    {                                                                                                                                  \
        return meet([&](auto &&arg) { return eval(t, arg) op eval(u, arg); });                                                        \
    }

        UN_OP_FOR_MEET(!)
        UN_OP_FOR_MEET(-)

        BIN_OP_FOR_MEET(+)
        BIN_OP_FOR_MEET(-)
        BIN_OP_FOR_MEET(*)
        BIN_OP_FOR_MEET(/)
        BIN_OP_FOR_MEET(%)
        BIN_OP_FOR_MEET(<)
        BIN_OP_FOR_MEET(<=)
        BIN_OP_FOR_MEET(==)
        BIN_OP_FOR_MEET(!=)
        BIN_OP_FOR_MEET(>=)
        BIN_OP_FOR_MEET(>)
        BIN_OP_FOR_MEET(||)
        BIN_OP_FOR_MEET(&&)

    } // namespace impl
    using impl::expr;
    using impl::_1;
} // namespace matchit

#endif // _EXPRESSION_H_