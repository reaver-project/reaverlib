/**
 * Reaver Library Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

#include "vector.h"

namespace reaver
{
    namespace tpl { inline namespace _v1
    {
        namespace _detail
        {
            template<typename Vector, template<typename> typename Predicate>
            struct _filter;

            template<template<typename> typename Predicate>
            struct _filter<vector<>, Predicate>
            {
                using type = vector<>;
            };

            template<typename FilteredVector, typename Vector, template<typename> typename Predicate>
            struct _filter_impl;

            template<typename... Filtered, template<typename> typename Predicate>
            struct _filter_impl<vector<Filtered...>, vector<>, Predicate>
            {
                using type = vector<Filtered...>;
            };

            template<typename... Filtered, typename Head, typename... Tail, template<typename> typename Predicate>
            struct _filter_impl<vector<Filtered...>, vector<Head, Tail...>, Predicate>
            {
                using type = typename _filter_impl<typename std::conditional<
                    Predicate<Head>::value,
                    vector<Filtered..., Head>,
                    vector<Filtered...>
                >::type, vector<Tail...>, Predicate>::type;
            };

            template<typename... Elements, template<typename> typename Predicate>
            struct _filter<vector<Elements...>, Predicate>
            {
                using type = typename _filter_impl<vector<>, vector<Elements...>, Predicate>::type;
            };
        }

        template<typename Vector, template<typename> typename Predicate>
        using filter = typename _detail::_filter<Vector, Predicate>::type;
    }}
}

