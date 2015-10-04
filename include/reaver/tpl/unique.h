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

#include <type_traits>

#include "vector.h"
#include "../logic.h"

namespace reaver
{
    namespace tpl { inline namespace _v1
    {
        namespace _detail
        {
            template<typename... Ts>
            struct _unique;

            template<typename... Inserted, typename Head, typename... Tail>
            struct _unique<tpl::vector<Inserted...>, Head, Tail...>
            {
                using type = typename std::conditional<
                    all_of<true, !std::is_same<Head, Inserted>::value...>::value,
                    typename _unique<tpl::vector<Inserted..., Head>, Tail...>::type,
                    typename _unique<tpl::vector<Inserted...>, Tail...>::type
                >::type;
            };

            template<typename... Inserted>
            struct _unique<tpl::vector<Inserted...>>
            {
                using type = tpl::vector<Inserted...>;
            };
        }

        template<typename... Ts>
        using unique = typename _detail::_unique<tpl::vector<>, Ts...>::type;
    }}
}
