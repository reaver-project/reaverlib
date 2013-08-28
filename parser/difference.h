/**
 * Reaver Library Licence
 *
 * Copyright (C) 2013 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
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
 * Michał "Griwes" Dominiak
 *
 **/

#pragma once

#include <type_traits>
#include <vector>

#include "helpers.h"
#include "skip.h"
#include "../lexer.h"

namespace reaver
{
    namespace parser
    {
        template<typename Tref, typename Uref, typename = typename std::enable_if<is_optional<typename
            std::remove_reference<Tref>::type::value_type>::value && is_optional<typename std::remove_reference<Uref>
            ::type::value_type>::value>::type>
        class difference_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = typename T::value_type;

            difference_parser(const T & match, const U & dont) : _match{ match }, _dont{ dont }
            {
            }

            template<typename Iterator>
            value_type match(Iterator & begin, Iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename Iterator, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(Iterator & begin, Iterator end, Skip skip) const
            {
                while (skip.match(begin, end)) {}

                auto b = begin;
                auto m = _match.match(b, end, skip);

                auto b2 = begin;
                auto d = _dont.match(b2, end, skip);

                if (m && !d)
                {
                    begin = b;
                    return { m };
                }

                return {};
            }

        private:
            Tref _match;
            Uref _dont;
        };
    }
}
