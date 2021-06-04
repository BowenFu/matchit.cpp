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

#define BINARY_OP(op)                                                                                                   \
    template <typename T, typename U, typename = std::enable_if_t<IsNullaryOrId<T>::value || IsNullaryOrId<U>::value> > \
    auto operator op(T const &t, U const &u)                                                                            \
    {                                                                                                                   \
        return nullary([&] { return eval(t) op eval(u); });                                                                \
    }

        BINARY_OP(+)
        BINARY_OP(-)
        BINARY_OP(*)
        BINARY_OP(/)
        BINARY_OP(<)
        BINARY_OP(<=)
        BINARY_OP(==)
        BINARY_OP(>=)
        BINARY_OP(>)

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

        // TODO, need to distinguish nullary / unary exprs.
#define BINARY_OP_ARG(op)                                                                                                       \
    template <typename T, typename U, typename = std::enable_if_t<IsUnaryOrWildcard<T>::value || IsUnaryOrWildcard<U>::value> > \
    auto operator op(T const &t, U const &u)                                                                                    \
    {                                                                                                                           \
        return expr([&](auto &&arg) { return eval(t, arg) op eval(u, arg); });                                                  \
    }

    } // namespace impl
    using impl::expr;
    // ADL
    // using impl::operator+;
    // using impl::operator*;
    // using impl::operator==;
} // namespace matchit

#endif // _EXPRESSION_H_