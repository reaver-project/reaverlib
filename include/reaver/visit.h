/**
 * Reaver Library Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <boost/variant.hpp>

#include "tmp.h"

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<typename Void, typename...>
        struct _filtered_common_type;

        template<typename Last>
        struct _filtered_common_type<void, Last>
        {
            using type = Last;
        };

        template<typename Last, typename... Voids>
        struct _filtered_common_type<typename std::enable_if<all<std::is_same<Voids, boost::detail::variant::void_>::value...>::value>::type, Last, Voids...>
        {
            using type = Last;
        };

        template<typename First, typename... Rest>
        struct _filtered_common_type<typename std::enable_if<!all<std::is_same<Rest, boost::detail::variant::void_>::value...>::value>::type, First, Rest...>
            : std::common_type<First, typename _filtered_common_type<void, Rest...>::type>
        {
        };

        template<typename... Rest>
        struct _filtered_common_type<void, boost::detail::variant::void_, Rest...> : _filtered_common_type<void, Rest...>
        {
        };

        template<typename Lambda>
        struct _call_forwarder
        {
            template<typename Arg, typename std::enable_if<!std::is_same<boost::detail::variant::void_, Arg>::value, int>::type = 0>
            auto operator()(Arg && arg) -> decltype(std::declval<Lambda>()(std::forward<Arg>(arg)));
            boost::detail::variant::void_ operator()(const boost::detail::variant::void_ & v);
        };
    }

    template<typename Lambda, typename... Variants>
    struct visitor
    {
        Lambda lambda;

        using result_type = typename _detail::_filtered_common_type<void, decltype(std::declval<_detail::_call_forwarder<Lambda>>()(std::declval<Variants>()))...>::type;

        template<typename Arg>
        auto operator()(Arg && arg) const
        {
            return lambda(std::forward<Arg>(arg));
        }

        const boost::detail::variant::void_ & operator()(const boost::detail::variant::void_ & v) const
        {
            return v;
        }
    };

    template<typename Lambda, typename... Variants>
    auto visit(Lambda && lambda, const boost::variant<Variants...> & variant)
    {
        return boost::apply_visitor(visitor<Lambda, Variants...>{ std::forward<Lambda>(lambda) }, variant);
    }
}}
