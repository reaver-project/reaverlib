/**
 * Reaver Library Licence
 *
 * Copyright © 2013 Michał "Griwes" Dominiak
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

namespace reaver
{
inline namespace __v1
{
    template<uint64_t I>
    struct increment
    {
        static constexpr uint64_t advance()
        {
            return I + 1;
        }
    };

    template<uint64_t I, uint64_t J>
    struct different
    {
        static constexpr bool check()
        {
            return I != J;
        }
    };

    template<uint64_t I, uint64_t J>
    struct less
    {
        static constexpr bool check()
        {
            return I < J;
        }
    };

    namespace _detail
    {
        template<uint64_t, uint64_t, template<uint64_t> class, template<uint64_t, uint64_t> class, template<uint64_t> class, bool>
        struct _static_for;

        template<uint64_t Begin, uint64_t End, template<uint64_t> class Func, template<uint64_t, uint64_t> class Compare,
            template<uint64_t> class Advance>
        struct _static_for<Begin, End, Func, Compare, Advance, true>
        {
            static constexpr uint64_t next = Advance<Begin>::advance();

            static void exec()
            {
                Func<Begin>{}();

                _static_for<next, End, Func, Compare, Advance, Compare<next, End>::check()>::exec();
            }
        };

        template<uint64_t Begin, uint64_t End, template<uint64_t> class Func, template<uint64_t, uint64_t> class Compare,
            template<uint64_t> class Advance>
        struct _static_for<Begin, End, Func, Compare, Advance, false>
        {
            static void exec()
            {
            }
        };
    }

    template<uint64_t Begin, uint64_t End, template<uint64_t> class Func, template<uint64_t, uint64_t> class Compare = less,
        template<uint64_t> class Advance = increment>
    struct static_for
    {
        static void exec()
        {
            _detail::_static_for<Begin, End, Func, Compare, Advance, Compare<Begin, End>::check()>::exec();
        }
    };
}
}
