/**
 * Reaver Library Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <boost/filesystem.hpp>

namespace reaver
{
    namespace wildcard { inline namespace _v1
    {
        inline bool match(const std::string & pattern, const std::string & string, std::size_t pattern_position = 0, std::size_t string_position = 0)
        {
            if (pattern_position == pattern.size() || string_position == string.size())
            {
                if (pattern_position == pattern.size() && string_position == string.size())
                {
                    return true;
                }

                if (pattern_position == pattern.size() - 1 && pattern[pattern_position] == '*')
                {
                    return true;
                }

                return false;
            }

            if (pattern[pattern_position] == '*')
            {
                return match(pattern, string, pattern_position, string_position + 1) || match(pattern, string, pattern_position + 1, string_position);
            }

            if (pattern[pattern_position] == '?' || pattern[pattern_position] == string[string_position])
            {
                return match(pattern, string, pattern_position + 1, string_position + 1);
            }

            return false;
        }
    }}
}
