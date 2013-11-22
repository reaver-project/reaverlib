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

#include "helpers.h"
#include "skip.h"

namespace reaver
{
    namespace parser
    {
        template<typename T = void, typename Iterator = lexer::iterator, typename = typename std::enable_if<is_char<
            typename std::iterator_traits<Iterator>::value_type>::value>::type>
        class literal
        {
        public:
            using value_type = void;
            using char_type = typename std::iterator_traits<Iterator>::value_type;

            literal(char_type c) : _str{ c }
            {
            }

            literal(std::basic_string<char_type> str) : _str{ std::move(str) }
            {
            }

            bool match(Iterator & begin, Iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip>
            bool match(Iterator & begin, Iterator end, Skip skip)
            {
                while (skip.match(begin, end)) {}

                if (begin == end)
                {
                    return false;
                }

                auto b = begin;
                auto strb = _str.cbegin();

                while (strb != _str.end())
                {
                    if (b == end)
                    {
                        return false;
                    }

                    if (*b != *strb)
                    {
                        return false;
                    }
                }

                begin = b;

                return true;
            }

        private:
            std::basic_string<char_type> _str;
        };
    }
}
