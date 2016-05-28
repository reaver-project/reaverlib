/**
 * Reaver Library Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "unit.h"

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<bool Condition, typename Then, bool ForeverFalse = false>
        struct _static_if
        {
            static constexpr bool is_taken = true && !ForeverFalse;

            _static_if(Then && then) : then{ std::forward<Then>(then) }
            {
            }

            ~_static_if() noexcept(false)
            {
                then(unit{});
            }

            Then then;

            template<typename Else>
            auto static_else(Else && else_) const
            {
                return _static_if<false, Else, true>{ std::forward<Else>(else_) };
            }

            template<typename Condition_, typename ElseIf>
            auto static_else_if(Condition_, ElseIf && else_if) const
            {
                return _static_if<false, ElseIf, true>{ std::forward<ElseIf>(else_if) };
            }
        };

        template<typename Then, bool ForeverFalse>
        struct _static_if<false, Then, ForeverFalse>
        {
            static constexpr bool is_taken = false;

            _static_if(Then &&)
            {
            }

            ~_static_if()
            {
            }

            template<typename Else>
            auto static_else(Else && else_) const
            {
                return _static_if<true && !ForeverFalse, Else, ForeverFalse>{ std::forward<Else>(else_) };
            }

            template<typename Condition_, typename ElseIf>
            auto static_else_if(Condition_ cond, ElseIf && else_if) const
            {
                return _static_if<cond() && !ForeverFalse, ElseIf, ForeverFalse>{ std::forward<ElseIf>(else_if) };
            }
        };
    }

    template<typename Bool, typename Then>
    auto static_if(Bool b, Then && then)
    {
        return _detail::_static_if<b(), Then>{ std::forward<Then>(then) };
    }
}}

