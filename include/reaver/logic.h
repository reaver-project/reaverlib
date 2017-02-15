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

namespace reaver
{
inline namespace _v1
{
    template<bool... Bools>
    struct any_of;

    template<bool... Rest>
    struct any_of<false, Rest...> : any_of<Rest...>
    {
    };

    template<bool... Rest>
    struct any_of<true, Rest...> : std::true_type
    {
    };

    template<>
    struct any_of<> : std::false_type
    {
    };

    template<bool... Bools>
    struct all_of : std::false_type
    {
    };

    template<bool... Tail>
    struct all_of<true, Tail...> : all_of<Tail...>
    {
    };

    template<>
    struct all_of<true> : std::true_type
    {
    };
}
}
