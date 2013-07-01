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

#include <vector>
#include <type_traits>

#include "helpers.h"
#include "skip.h"
#include "../lexer.h"

namespace reaver
{
    namespace parser
    {
        template<typename Tref>
        class and_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = void;

            and_parser(const T & ap) : _and{ ap }
            {
            }

            bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            bool match(std::vector<lexer::token>::const_iterator begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                return _and.match(begin, end, skip);
            }

        private:
            Tref _and;
        };
    }
}
