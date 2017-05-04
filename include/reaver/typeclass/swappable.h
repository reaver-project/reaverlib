/**
 * Reaver Library Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "sfinae_function.h"
#include "typeclass.h"

namespace reaver
{
inline namespace prelude
{
    inline namespace _v1
    {
        struct swappable
        {
            TYPECLASS_INSTANCE(typename T);
        };

        template<typename T>
        auto swap(T & a, T & b) SFINAE_FUNCTION(tc_instance<swappable, T>::swap(a, b));

        // clang-format off
        DEFAULT_INSTANCE(swappable, T)
        {
            template<typename U, typename = void>
            struct has_member_swap : std::false_type {};

            template<typename U>
            struct has_member_swap<U, std::void_t<decltype(std::declval<U &>().swap(std::declval<U &>()))>> : std::true_type {};

            template<typename U = T, typename std::enable_if<!has_member_swap<U>::value, int>::type...>
            static void swap(T & a, T & b)
                noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
            {
                auto tmp = std::move(a);
                a = std::move(b);
                b = std::move(tmp);
            }

            template<typename U = T, typename std::enable_if<has_member_swap<U>::value, int>::type...>
            static void swap(T & a, T & b) noexcept(noexcept(a.swap(b)))
            {
                a.swap(b);
            }
        };
    }
}
}
