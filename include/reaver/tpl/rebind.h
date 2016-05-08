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
            template<typename Vector, template<typename...> typename Another>
            struct _rebind;

            template<typename... Types, template<typename...> typename Another>
            struct _rebind<tpl::vector<Types...>, Another>
            {
                using type = Another<Types...>;
            };

            template<typename Other>
            struct _unbind;

            template<template<typename...> typename Template, typename... Ts>
            struct _unbind<Template<Ts...>>
            {
                using type = tpl::vector<Ts...>;
            };
        }

        template<typename Vector, template<typename...> typename Another>
        using rebind = typename _detail::_rebind<Vector, Another>::type;

        template<typename Other>
        using unbind = typename _detail::_unbind<Other>::type;
    }}
}

