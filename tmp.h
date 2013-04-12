/**
 * Reaver Library Licence
 *
 * Copyright (C) 2012-2013 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Michał "Griwes" Dominiak
 *
 **/

#include <type_traits>
#include <vector>
#include <tuple>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace reaver
{
    template<typename>
    struct is_vector : public std::false_type
    {
    };

    template<typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type
    {
    };

    template<typename T>
    struct is_tuple : public std::false_type
    {
    };

    template<typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : public std::true_type
    {
    };

    template<typename T>
    struct is_optional : public std::false_type
    {
    };

    template<typename T>
    struct is_optional<boost::optional<T>> : public std::true_type
    {
    };

    template<typename...>
    struct is_variant : public std::false_type
    {
    };

    template<typename... Ts>
    struct is_variant<boost::variant<Ts...>> : public std::true_type
    {
    };

    namespace _detail
    {
        template<typename T>
        std::tuple<T> _tuple(T && t)
        {
            return t;
        }

        template<typename... TupleTypes>
        const std::tuple<TupleTypes...> & _tuple(const std::tuple<TupleTypes...> & t)
        {
            return t;
        }

        template<typename... TupleTypes>
        std::tuple<TupleTypes...> && _tuple(std::tuple<TupleTypes...> && t)
        {
            return std::move(t);
        }
    }

    template<typename... T>
    auto tuple_val_cat(T &&... t) -> decltype(std::tuple_cat(_detail::_tuple(t)...))
    {
        return std::tuple_cat(_detail::_tuple(t)...);
    }

    template<typename>
    struct remove_tuple_references;

    template<typename... TupleTypes>
    struct remove_tuple_references<std::tuple<TupleTypes &...>>
    {
        using type = std::tuple<TupleTypes...>;
    };

    template<typename... Ts>
    struct make_tuple_type
    {
        using type = typename remove_tuple_references<decltype(tuple_val_cat(std::declval<Ts>()...))>::type;
    };

    template<typename T>
    struct remove_optional
    {
        using type = T;
    };

    template<typename T>
    struct remove_optional<boost::optional<T>>
    {
        using type = typename remove_optional<T>::type;
    };

    template<typename T, typename U>
    struct make_variant_type
    {
        using type = boost::variant<typename remove_optional<T>::type, typename remove_optional<U>::type>;
    };

    template<typename T, typename... Ts>
    struct make_variant_type<T, boost::variant<Ts...>>
    {
        using type = boost::variant<typename remove_optional<T>::type, typename remove_optional<Ts>::type...>;
    };

    template<typename... Ts, typename T>
    struct make_variant_type<boost::variant<Ts...>, T>
    {
        using type = boost::variant<typename remove_optional<Ts>::type..., typename remove_optional<T>::type>;
    };

    template<typename... T1s, typename... T2s>
    struct make_variant_type<boost::variant<T1s...>, boost::variant<T2s...>>
    {
        using type = boost::variant<typename remove_optional<T1s>::type..., typename remove_optional<T2s>::type...>;
    };

    template<int...>
    struct sequence
    {
    };

    template<int N, int ...S>
    struct generator : generator<N - 1, N - 1, S...>
    {
    };

    template<int ...S>
    struct generator<0, S...>
    {
        using type = sequence<S...>;
    };
}
