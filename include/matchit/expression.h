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
        auto nullary(T t)
        {
            return Nullary<T>{t};
        }

        template <typename T>
        auto expr(Id<T> const &id)
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
            constexpr static auto evalImpl(T const &v, Args const &...)
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
        template <typename T>
        class EvalTraits<Id<T> >
        {
        public:
            constexpr static auto evalImpl(Id<T> const &id)
            {
                return *id;
            }
        };

        template <typename Pred> class Meet;
    
        // Unary is an alias of Meet.
        template <typename T>
        using Unary = Meet<T>;

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

        class Wildcard;
        template <>
        class EvalTraits<Wildcard>
        {
        public:
            template <typename Arg>
            constexpr static auto evalImpl(Wildcard const &, Arg const &arg)
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

        template <typename T>
        class IsNullaryOrId<Id<T> > : public std::true_type
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
        class IsUnaryOrWildcard<Unary<T> > : public std::true_type
        {
        };


        // unary is an alias of meet.
        template <typename T>
        auto unary(T&& t)
        {
            return meet(std::forward<T>(t));
        }

#define UN_OP_FOR_UNARY(op)                                                              \
    template <typename T, std::enable_if_t<IsUnaryOrWildcard<T>::value, bool> = true> \
    auto operator op(T const &t)                                                         \
    {                                                                                    \
        return unary([&](auto &&arg) { return op eval(t, arg); });                       \
    }

#define BIN_OP_FOR_UNARY(op)                                                                                                           \
    template <typename T, typename U, std::enable_if_t<IsUnaryOrWildcard<T>::value || IsUnaryOrWildcard<U>::value, bool> = true> \
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

    } // namespace impl
    using impl::expr;
} // namespace matchit

#endif // _EXPRESSION_H_