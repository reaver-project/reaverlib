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

#include <string>
#include <vector>

namespace reaver
{
    namespace format
    {
        namespace executable
        {
            class section
            {
            public:
                section(std::vector<std::uint8_t> blob) : _blob{ std::move(blob) }
                {
                }

                std::string name() const
                {
                    return _name;
                }

                const std::vector<std::uint8_t> & blob() const
                {
                    return _blob;
                }

            private:
                std::string _name;
                std::vector<std::uint8_t> _blob;
            };
        }
    }
}
