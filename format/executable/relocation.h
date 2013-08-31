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

namespace reaver
{
    namespace format
    {
        namespace executable
        {
            class relocation
            {
            public:
                relocation(std::string section, std::uintmax_t offset, std::string symbol, std::uintmax_t size, std::uintmax_t addend)
                    : _section{ std::move(section) }, _offset{ offset }, _symbol{ std::move(symbol) }, _size{ size }, _addend{ addend }
                {
                }

                std::string section() const
                {
                    return _section;
                }

                std::uintmax_t offset() const
                {
                    return _offset;
                }

                std::string symbol() const
                {
                    return _symbol;
                }

                std::uintmax_t size() const
                {
                    return _size;
                }

                std::uintmax_t addend() const
                {
                    return _addend;
                }

            private:
                std::string _section;
                std::uintmax_t _offset;
                std::string _symbol;
                std::uintmax_t _size;
                std::uintmax_t _addend;
            };
        }
    }
}
