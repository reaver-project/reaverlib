/**
 * Reaver C++ Standard Library Implementation Licence
 *
 * Copyright (C) 2012-2013 Reaver Project Team:
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

#include <string>
#include <vector>
#include <unordered_set>

namespace reaver
{
    namespace lexer
    {
        class tokens_description
        {

        };

        class token
        {
            uint64_t _type;
        };

        class regex_token : public token
        {
            std::string _regex;
        };

        class set_token : public token
        {
            std::unordered_set<std::string> _values;
        };

        template<typename InputIterator>
        std::vector<token> tokenize(InputIterator, InputIterator, const tokens_description &);

        inline std::vector<token> tokenize(const std::string & str, const tokens_description & def)
        {
            return tokenize(str.begin(), str.end(), def);
        }

        inline std::vector<token> tokenize(std::istream & str, const tokens_description & def)
        {
            return tokenize(std::istreambuf_iterator<char>(str.rdbuf()), std::istreambuf_iterator<char>(), def);
        }
    }
}
