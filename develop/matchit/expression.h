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
        constexpr auto expr(Id<T> &id)
        {
            return nullary([&]
                           { return *id; });
        }

        template <typename T>
        constexpr auto expr(T const &v)
        {
            return nullary([&]
                           { return v; });
        }

        template <typename T> 
        constexpr auto toNullary(Nullary<T> const &v)
        {
            return v;
        }

        template <typename T>
        constexpr auto toNullary(T &&v)
        {
            if constexpr (std::is_invocable_v<std::decay_t<T>>)
            {
                return v;
            }
            else
            {
                return expr(v);
            }
        }

        // for constant
        template <typename T>
        class EvalTraits
        {
        public:
            template <typename... Args>
            constexpr static decltype(auto) evalImpl(T const &v, Args const &...)
            {
                return v;
            }
        };

        template <typename T>
        class EvalTraits<Nullary<T>>
        {
        public:
            constexpr static decltype(auto) evalImpl(Nullary<T> const &e) { return e(); }
        };

        // Only allowed in nullary
        template <typename T>
        class EvalTraits<Id<T>>
        {
        public:
            constexpr static decltype(auto) evalImpl(Id<T> const &id)
            {
                return *const_cast<Id<T> &>(id);
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
            constexpr static decltype(auto) evalImpl(Unary<T> const &e, Arg const &arg)
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
            constexpr static decltype(auto) evalImpl(Wildcard const &, Arg const &arg)
            {
                return arg;
            }
        };

        template <typename T, typename... Args>
        constexpr decltype(auto) evaluate_(T const &t, Args const &...args)
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
    constexpr auto operator op(T const &t)                                  \
    {                                                                       \
        return nullary([&] { return op evaluate_(t); });                         \
    }

#define BIN_OP_FOR_NULLARY(op)                                                 \
    template <typename T, typename U,                                          \
              std::enable_if_t<isNullaryOrIdV<T> || isNullaryOrIdV<U>, bool> = \
                  true>                                                        \
    constexpr auto operator op(T const &t, U const &u)                         \
    {                                                                          \
        return nullary([&] { return evaluate_(t) op evaluate_(u); });                    \
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
        BIN_OP_FOR_NULLARY(^)

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
    constexpr auto operator op(T const &t)                                      \
    {                                                                           \
        return unary([&](auto &&arg) constexpr { return op evaluate_(t, arg); });    \
    }

#define BIN_OP_FOR_UNARY(op)                                                   \
    template <typename T, typename U,                                          \
              std::enable_if_t<isUnaryOrWildcardV<T> || isUnaryOrWildcardV<U>, \
                               bool> = true>                                   \
    constexpr auto operator op(T const &t, U const &u)                         \
    {                                                                          \
        return unary([&](auto &&arg) constexpr {                               \
            return evaluate_(t, arg) op evaluate_(u, arg);                               \
        });                                                                    \
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
        BIN_OP_FOR_UNARY(^)

#undef BIN_OP_FOR_UNARY

    } // namespace impl
    using impl::expr;
} // namespace matchit

#endif // MATCHIT_EXPRESSION_H
