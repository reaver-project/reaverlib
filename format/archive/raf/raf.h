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

#include <unordered_map>

#include "format/archive.h"

namespace reaver
{
    namespace format
    {
        namespace archive
        {
            class raf : public archive
            {
            public:
                static std::unique_ptr<raf> read(std::istream &);

                virtual ~raf() {}

                virtual std::unique_ptr<std::istream> operator[](const std::string &) const override;

                virtual void save(std::ostream &) const override;
                virtual void unpack(const std::string &) const override;
                virtual void add(const std::string &, std::istream &) override;

            private:
                std::unordered_map<std::string, std::vector<char>> _files;
            };
        }
    }
}
