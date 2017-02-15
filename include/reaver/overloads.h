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

#include <utility>

namespace reaver
{
inline namespace _v1
{
    template<unsigned I>
    struct choice : choice<I + 1>
    {
    };

    template<>
    struct choice<10>
    {
    };

    struct select_overload : choice<0>
    {
    };

    template<typename T, typename... Ts>
    struct overload_set : T, overload_set<Ts...>
    {
        overload_set(T t, Ts... ts) : T{ std::forward<T>(t) }, overload_set<Ts...>{ std::forward<Ts>(ts)... }
        {
        }

        using T::operator();
        using overload_set<Ts...>::operator();
    };

    template<typename T>
    struct overload_set<T> : T
    {
        overload_set(T t) : T{ std::forward<T>(t) }
        {
        }

        using T::operator();
    };

    template<typename... Ts>
    auto make_overload_set(Ts &&... ts)
    {
        return overload_set<Ts...>{ std::forward<Ts>(ts)... };
    }
}
}
