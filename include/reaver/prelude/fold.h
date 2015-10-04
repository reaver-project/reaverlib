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
    inline namespace prelude { inline namespace _v1
    {
        template<typename F, typename T>
        inline constexpr auto foldr(F &&, T && t)
        {
            return std::forward<T>(t);
        }

        template<typename F, typename T, typename U, typename... Ts>
        inline constexpr auto foldr(F && f, T && t, U && u, Ts &&... ts)
        {
            return f(std::forward<U>(u), foldr(f, std::forward<T>(t), std::forward<Ts>(ts)...));
        }

        template<typename F, typename T>
        inline constexpr auto foldl(F &&, T && t)
        {
            return std::forward<T>(t);
        }

        template<typename F, typename T, typename U, typename... Ts>
        inline constexpr auto foldl(F && f, T && t, U && u, Ts &&... ts)
        {
            return foldl(f, f(std::forward<T>(t), std::forward<U>(u)), std::forward<Ts>(ts)...);
        }
    }}
}

