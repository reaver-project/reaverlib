/**
 * Reaver Library Licence
 *
 * Copyright © 2013 Michał "Griwes" Dominiak
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

reaver::format::executable::format reaver::format::executable::make_format(const std::string & str)
{
    if (str.substr(0, 3) == "elf")
    {
        return format::elf;
    }

    if (str.substr(0, 3) == "rxf")
    {
        return format::rxf;
    }

    throw unknown_format{ str };
}

std::size_t reaver::format::executable::get_bitness(const std::string & str)
{
    switch (make_format(str))
    {
        case format::elf:
        return std::stoull(str.substr(3));

        case format::rxf:
        return 64;
    }
}

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::read(const std::string & filename)
{
    return read(std::ifstream{ filename, std::ios::binary | std::ios::in });
}

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::read(std::istream & in)
{
    if (!in)
    {
        throw invalid_istream{};
    }

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

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::read(std::istream && in)
{
    return read(in);
}
