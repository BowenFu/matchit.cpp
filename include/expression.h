#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include <type_traits>

namespace matchit
{
    namespace impl
    {
        template <typename T>
        class Expr : public T
        {
        public:
            using T::operator();
        };

        template <typename T>
        auto expr(T t)
        {
            return Expr<T>{t};
        }

        template <typename T>
        auto nullary(Id<T> const& id)
        {
            return expr([&] { return *id; });
        }

        template <typename T>
        auto nullary(T const& v)
        {
            return expr([&] { return v; });
        }

        template <typename T>
        class EvalTraits
        {
        public:
            constexpr static auto evalImpl(T const& v)
            {
                return v;
            }
        };

        template <typename T>
        class EvalTraits<Expr<T>>
        {
        public:
            constexpr static auto evalImpl(Expr<T> const& expr)
            {
                return expr();
            }
        };

        template <typename T>
        class EvalTraits<Id<T>>
        {
        public:
            constexpr static auto evalImpl(Id<T> const& id)
            {
                return *id;
            }
        };

        template <typename T>
        auto eval(T const& t)
        {
            return EvalTraits<T>::evalImpl(t);
        }

        template <typename T>
        class IsExprOrId : public std::false_type {};

        template <typename T>
        class IsExprOrId<Id<T>> : public std::true_type {};

        template <typename T>
        class IsExprOrId<Expr<T>> : public std::true_type {};

        #define BINARY_OP(op)                                                  \
        template <typename T, typename U, typename = std::enable_if_t<IsExprOrId<T>::value || IsExprOrId<U>::value>>                               \
        auto operator op (T const& t, U const& u)                       \
        {                                                               \
            return expr([&] { return eval(t) op eval(u); });            \
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

    } // namespace impl
    using impl::nullary;
    using impl::operator+;
    using impl::operator*;
} // namespace matchit

#endif // _EXPRESSION_H_