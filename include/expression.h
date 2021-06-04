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

        template <typename T, bool own>
        auto nullary(Id<T, own> const& id)
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
            template <typename... Args>
            constexpr static auto evalImpl(T const& v, Args const&... args)
            {
                return v;
            }
        };

        template <typename T>
        class EvalTraits<Expr<T>>
        {
        public:
            template <typename... Args>
            constexpr static auto evalImpl(Expr<T> const& e, Args const&... args)
            {
                return e(args...);
            }
        };

        template <typename T, bool own>
        class EvalTraits<Id<T, own>>
        {
        public:
            template <typename... Args>
            constexpr static auto evalImpl(Id<T, own> const& id, Args const&... args)
            {
                return *id;
            }
        };

        template <typename T, typename... Args>
        auto eval(T const& t, Args const&... args)
        {
            return EvalTraits<T>::evalImpl(t, args...);
        }

        template <typename T>
        class IsExprOrId : public std::false_type {};

        template <typename T, bool own>
        class IsExprOrId<Id<T, own>> : public std::true_type {};

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

        #define BINARY_OP_ARG(op)                                                  \
        template <typename T, typename U, typename = std::enable_if_t<std::is_same_v<T, Wildcard> || std::is_same_v<U, Wildcard>>>                               \
        auto operator op (T const& t, U const& u)                       \
        {                                                               \
            return expr([&] (auto&& arg) { return eval(t, arg) op eval(u, arg); });            \
        }

    } // namespace impl
    using impl::nullary;
    // ADL
    // using impl::operator+;
    // using impl::operator*;
    // using impl::operator==;
} // namespace matchit

#endif // _EXPRESSION_H_