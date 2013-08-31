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

#include <fstream>

#include "executable.h"
#include "elf/elf.h"
#include "rxf/rxf.h"

bool reaver::format::executable::possible(reaver::format::executable::format format, reaver::format::executable::type type)
{
    switch (format)
    {
        case format::elf:
            switch (type)
            {
                case type::object_file:
                case type::executable:
                case type::dynamic_library:
                    return true;
                default:
                    return false;
            }

        case format::rxf:
            switch (type)
            {
                case type::object_file:
                case type::executable:
                case type::chunk_package:
                    return true;
                default:
                    return false;
            }
    }
}

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::open(std::string filename)
{
    std::ifstream in{ filename, std::ios::binary | std::ios::in };
    return read(in);
}

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::read(std::istream & in)
{
    std::unique_ptr<reaver::format::executable::executable> ret = elf::read(in);

    if (!ret)
    {
        ret = rxf::read(in);
    }

    if (!ret)
    {
        throw invalid_or_unsupported_file{};
    }

    return std::move(ret);
}
