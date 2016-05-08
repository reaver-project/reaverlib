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
            template<typename... Vectors>
            struct _concat;

            template<typename... Elements>
            struct _concat<vector<Elements...>>
            {
                using type = vector<Elements...>;
            };

            template<typename... Elements1, typename... Elements2>
            struct _concat<vector<Elements1...>, vector<Elements2...>>
            {
                using type = vector<Elements1..., Elements2...>;
            };

            template<typename First, typename Second, typename... Tail>
            struct _concat<First, Second, Tail...>
            {
                using type = typename _concat<typename _concat<First, Second>::type, Tail...>::type;
            };
        }

        template<typename... Vectors>
        using concat = typename _detail::_concat<Vectors...>::type;
    }}
}

