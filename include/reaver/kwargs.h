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

#include <utility>
#include <array>

#include "relaxed_constexpr.h"

namespace reaver
{
    namespace kwargs { inline namespace _v1
    {
        template<typename T>
        struct kwarg
        {
            using value_type = T;

            constexpr kwarg(T t) : value{ std::move(t) }
            {
            }

            T value;
        };

        struct any
        {
            template<typename T>
            constexpr any(T &&)
            {
            }
        };

        template<typename K, typename... Args>
        constexpr auto get(any, Args &&... args)
        {
            return get<K>(std::forward<Args>(args)...);
        }

        template<typename K, typename... Args>
        constexpr auto get(K && kw, Args &&...)
        {
            return kw.value;
        }

        template<typename K>
        constexpr auto get_or(typename K::value_type const & def)
        {
            return def;
        }

        template<typename K, typename... Args>
        constexpr auto get_or(typename K::value_type const & def, any, Args &&... args)
        {
            return get_or<K>(def, std::forward<Args>(args)...);
        }

        template<typename K, typename... Args>
        constexpr auto get_or(typename K::value_type const & def, K && kw, Args &&...)
        {
            return kw.value;
        }

        template<typename K, typename First, typename... Args>
        relaxed_constexpr auto get_nth(std::size_t i, First && first, Args &&... args)
        {
            if (!i)
            {
                return get<K>(std::forward<First>(first), std::forward<Args>(args)...);
            }

            return get_nth<K>(i - 1, std::forward<Args>(args)...);
        }

        template<typename K, typename First, typename... Args>
        relaxed_constexpr auto get_nth(std::size_t i, First && first)
        {
            if (!i)
            {
                return get<K>(std::forward<First>(first));
            }

            throw std::out_of_range("there are not enough keyword arguments with selected key in the pack");
        }

        namespace _detail
        {
            template<typename K, typename... Args>
            struct _count;

            template<typename K>
            struct _count<K>
            {
                static constexpr std::size_t value = 0;
            };

            template<typename K, typename... Args>
            struct _count<K, K, Args...>
            {
                static constexpr std::size_t value = _count<K, Args...>::value + 1;
            };

            template<typename K, typename Other, typename... Args>
            struct _count<K, Other, Args...>
            {
                static constexpr std::size_t value = _count<K, Args...>::value;
            };
        }

        template<typename K, typename... Args>
        relaxed_constexpr auto get_all(Args &&... args)
        {
            std::array<typename K::value_type, _detail::_count<K, Args...>::value> array;

            for (std::size_t i = 0; i < array.size(); ++i)
            {
                array[i] = get_nth<K>(i, std::forward<Args>(args)...);
            }

            return array;
        }
    }}
}
