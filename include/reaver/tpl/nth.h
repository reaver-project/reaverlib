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

#include <cstddef>

#include "vector.h"

namespace reaver
{
namespace tpl
{
    inline namespace _v1
    {
        namespace _detail
        {
            template<typename Vector, std::size_t N>
            struct _nth;

            template<typename Head, typename... Elements, std::size_t N>
            struct _nth<vector<Head, Elements...>, N>
            {
                static_assert(sizeof...(Elements) + 1 >= N);
                using type = typename _nth<vector<Elements...>, N - 1>::type;
            };

            template<typename Head, typename... Elements>
            struct _nth<vector<Head, Elements...>, 0>
            {
                using type = Head;
            };
        }

        template<typename Vector, std::size_t N>
        using nth = typename _detail::_nth<Vector, N>::type;
    }
}
}
