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

#include "i386.h"

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::elf32_i386::read(reaver::format::
    executable::elf32::_header header, std::istream & in)
{
    std::unique_ptr<elf32_i386> ret{ new elf32_i386{} };
    ret->_elf_header = header;

    ret->_read_program_header(in);

    ret->_sections = ret->_read_section_headers(in);
    ret->_section_names_string_table = ret->_read_section(in, ret->_elf_header.string_section_header_index);

    return std::move(ret);
}
