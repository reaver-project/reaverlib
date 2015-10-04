/**
 * Reaver Library Licence
 *
 * Copyright © 2012-2013 Michał "Griwes" Dominiak
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
 **/

#pragma once

#include <type_traits>
#include <vector>
#include <tuple>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace reaver
{
inline namespace __v1
{
    template<typename>
    struct is_char : public std::false_type
    {
    };

    template<>
    struct is_char<char> : public std::true_type
    {
    };

    template<>
    struct is_char<char16_t> : public std::true_type
    {
    };

    template<>
    struct is_char<char32_t> : public std::true_type
    {
    };

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

    template<typename T, typename U>
    struct make_tuple_type
    {
        using type = std::tuple<T, U>;
    };

    template<typename T, typename... Us>
    struct make_tuple_type<T, std::tuple<Us...>>
    {
        using type = std::tuple<T, Us...>;
    };

    template<typename... Ts, typename U>
    struct make_tuple_type<std::tuple<Ts...>, U>
    {
        using type = std::tuple<Ts..., U>;
    };

    template<typename... Ts, typename... Us>
    struct make_tuple_type<std::tuple<Ts...>, std::tuple<Us...>>
    {
        using type = std::tuple<Ts..., Us...>;
    };

    template<typename T>
    struct remove_optional
    {
        using type = T;
    };

    template<typename T>
    struct remove_optional<boost::optional<T>>
    {
        using type = T;
    };

    template<typename T, typename U>
    struct make_variant_type
    {
        using type = boost::variant<typename remove_optional<T>::type, typename remove_optional<U>::type>;
    };

    namespace _detail
    {
        template<typename... Ts>
        struct _type_sequence
        {
        };

        template<typename...>
        struct _make_variant_type_sequence_helper;

        template<typename First, typename... Ts, typename... Types>
        struct _make_variant_type_sequence_helper<_type_sequence<Types...>, First, Ts...>
        {
            using type = typename _make_variant_type_sequence_helper<_type_sequence<Types..., First>, Ts...>::type;
        };

        template<typename... Ts, typename... Types>
        struct _make_variant_type_sequence_helper<_type_sequence<Types...>, boost::detail::variant::void_, Ts...>
        {
            using type = _type_sequence<Types...>;
        };

        template<typename... Types>
        struct _make_variant_type_sequence_helper<_type_sequence<Types...>>
        {
            using type = _type_sequence<Types...>;
        };

        template<typename, typename>
        struct _make_variant_helper;

        template<typename... Types1, typename... Types2>
        struct _make_variant_helper<_type_sequence<Types1...>, _type_sequence<Types2...>>
        {
            using type = boost::variant<Types1..., Types2...>;
        };
    }

    template<typename T, typename... Ts>
    struct make_variant_type<T, boost::variant<Ts...>>
    {
        using type = typename _detail::_make_variant_helper<
            typename _detail::_make_variant_type_sequence_helper<
                _detail::_type_sequence<>,
                typename remove_optional<Ts>::type...,
                typename remove_optional<T>::type
            >::type,
            _detail::_type_sequence<>
        >::type;
    };

    template<typename... Ts, typename T>
    struct make_variant_type<boost::variant<Ts...>, T>
    {
        using type = typename _detail::_make_variant_helper<
            typename _detail::_make_variant_type_sequence_helper<
                _detail::_type_sequence<>,
                typename remove_optional<T>::type,
                typename remove_optional<Ts>::type...
            >::type,
            _detail::_type_sequence<>
        >::type;
    };

    template<typename... T1s, typename... T2s>
    struct make_variant_type<boost::variant<T1s...>, boost::variant<T2s...>>
    {
        using type = typename _detail::_make_variant_helper<
            typename _detail::_make_variant_type_sequence_helper<
                _detail::_type_sequence<>,
                typename remove_optional<T1s>::type...
            >::type,
            typename _detail::_make_variant_type_sequence_helper<
                _detail::_type_sequence<>,
                typename remove_optional<T2s>::type...
            >::type
        >::type;
    };

    template<typename T>
    struct make_variant_type<T, T>
    {
        using type = T;
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

    template<typename First, typename...>
    struct first_type
    {
        using type = First;
    };

    template<typename...>
    struct append_first_type;

    template<typename... First, typename... Second>
    struct append_first_type<std::tuple<First...>, std::tuple<Second...>>
    {
        using type = std::tuple<First..., typename first_type<Second...>::type>;
    };

    template<typename... Ts>
    struct append_first_type<std::tuple<Ts...>, std::tuple<>>
    {
        using type = std::tuple<Ts...>;
    };

    template<typename First, typename... Ts>
    struct cut_first
    {
        using type = std::tuple<Ts...>;
    };

    template<bool... Bools>
    struct all;

    template<>
    struct all<> : std::true_type
    {
    };

    template<bool... Rest>
    struct all<true, Rest...> : std::integral_constant<bool, all<Rest...>::value>
    {
    };

    template<bool... Rest>
    struct all<false, Rest...> : std::false_type
    {
    };
}
}
