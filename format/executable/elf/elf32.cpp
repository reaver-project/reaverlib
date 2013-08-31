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

#include "elf32.h"

void reaver::format::executable::elf32::_read_header(std::istream & in)
{
    in.read(reinterpret_cast<char *>(_elf_header._ident), 16);

    if (!(_elf_header._ident[_magic0_index] == 0x7F && _elf_header._ident[_magic1_index] == 'E' && _elf_header._ident[_magic2_index] == 'L'
        && _elf_header._ident[_magic3_index] == 'F') || _elf_header._ident[_version_index] == _none_version)
    {
        throw reaver::format::executable::invalid_or_unsupported_file{};
    }

    bool little = _elf_header._ident[_data_index] == _lsb;
    using reaver::format::utils::read;

    read(_elf_header._type, in, little);
    read(_elf_header._machine, in, little);
    read(_elf_header._version, in, little);
    read(_elf_header._entry, in, little);
    read(_elf_header._program_header_offset, in, little);
    read(_elf_header._section_header_offset, in, little);
    read(_elf_header._flags, in, little);
    read(_elf_header._header_size, in, little);
    read(_elf_header._program_header_entry_size, in, little);
    read(_elf_header._program_header_entry_count, in, little);
    read(_elf_header._section_header_entry_size, in, little);
    read(_elf_header._section_header_entry_count, in, little);
    read(_elf_header._string_section_header_index, in, little);
}

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::elf32::read(std::istream & in)
{
    std::unique_ptr<reaver::format::executable::elf32> ret{ new reaver::format::executable::elf32{} };

    ret->_read_header(in);
    if (ret->_elf_header._ident[_class_index] != _machine::_386 || ret->_elf_header._ident[_data_index] != _lsb)
    {
        throw invalid_or_unsupported_file{};
    }

    ret->_arch = target::arch::i386;

    return std::move(ret);
}
