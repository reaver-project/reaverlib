/**
 * Reaver Library Licence
 *
 * Copyright © 2013, 2015 Michał "Griwes" Dominiak
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
#include <type_traits>

namespace reaver { inline namespace _v1
{
    template<std::size_t I>
    struct increment : std::integral_constant<std::size_t, I + 1> {};

    template<std::size_t I, std::size_t J>
    struct different : std::integral_constant<bool, I != J> {};

    template<std::size_t I, std::size_t J>
    struct less : std::integral_constant<bool, I < J> {};

    namespace _detail
    {
        template<std::size_t, std::size_t, template<std::size_t> class, template<std::size_t, std::size_t> class, template<std::size_t> class, bool>
        struct _static_for;

        template<std::size_t Begin, std::size_t End, template<std::size_t> class Func, template<std::size_t, std::size_t> class Compare,
            template<std::size_t> class Advance>
        struct _static_for<Begin, End, Func, Compare, Advance, true>
        {
            static constexpr std::size_t next = Advance<Begin>();

            static void exec()
            {
                Func<Begin>{}();
                _static_for<next, End, Func, Compare, Advance, Compare<next, End>{}>::exec();
            }
        };

        template<std::size_t Begin, std::size_t End, template<std::size_t> class Func, template<std::size_t, std::size_t> class Compare,
            template<std::size_t> class Advance>
        struct _static_for<Begin, End, Func, Compare, Advance, false>
        {
            static void exec()
            {
            }
        };
    }

    template<std::size_t Begin, std::size_t End, template<std::size_t> class Func, template<std::size_t, std::size_t> class Compare = less,
        template<std::size_t> class Advance = increment>
    struct static_for
    {
        static void exec()
        {
            _detail::_static_for<Begin, End, Func, Compare, Advance, Compare<Begin, End>{}>::exec();
        }
    };
}}
