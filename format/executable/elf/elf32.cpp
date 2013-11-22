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
#include "i386.h"

std::unique_ptr<reaver::format::executable::executable> reaver::format::executable::elf32::read(std::istream & in)
{
    _header header = _read_header(in);

    if (header.machine == _machine::_386)
    {
        if (header.ident[_data_index] != _lsb)
        {
            throw invalid_elf_file{};
        }

        return elf32_i386::read(header, in);
    }

    return nullptr;
}

reaver::format::executable::elf32::_header reaver::format::executable::elf32::_read_header(std::istream & in)
{
    using reaver::format::utils::read;

    _header header;
    in.read(reinterpret_cast<char *>(header.ident), 16);

    if (!(header.ident[_magic0_index] == 0x7F && header.ident[_magic1_index] == 'E' && header.ident[_magic2_index] == 'L'
        && header.ident[_magic3_index] == 'F') || header.ident[_version_index] == _none_version)
    {
        throw reaver::format::executable::invalid_or_unsupported_file{};
    }

    bool little = header.ident[_data_index] == _lsb;

    read(header.type, in, little);
    read(header.machine, in, little);
    read(header.version, in, little);
    read(header.entry, in, little);
    read(header.program_header_offset, in, little);
    read(header.section_header_offset, in, little);
    read(header.flags, in, little);
    read(header.header_size, in, little);
    read(header.program_header_entry_size, in, little);
    read(header.program_header_entry_count, in, little);
    read(header.section_header_entry_size, in, little);
    read(header.section_header_entry_count, in, little);
    read(header.string_section_header_index, in, little);

    return header;
}

void reaver::format::executable::elf32::_read_program_header(std::istream & in)
{
    using reaver::format::utils::read;

    if (_elf_header.program_header_offset)
    {
        auto g = in.tellg();
        bool little = _elf_header.ident[_data_index] == _lsb;
        _prog_header = _program_header{};

        in.seekg(_elf_header.program_header_offset);
        read(_prog_header->type, in, little);
        read(_prog_header->offset, in, little);
        read(_prog_header->virtual_address, in, little);
        read(_prog_header->physical_address, in, little);
        read(_prog_header->file_size, in, little);
        read(_prog_header->memory_size, in, little);
        read(_prog_header->flags, in, little);
        read(_prog_header->align, in, little);

        std::cout << std::hex << _prog_header->virtual_address << std::endl;
        std::cout << _prog_header->physical_address << std::endl;

        in.seekg(g);
    }
}

std::vector<reaver::format::executable::elf32::_section_header> reaver::format::executable::elf32::_read_section_headers(
    std::istream & in)
{
    return {};
}

std::vector<uint8_t> reaver::format::executable::elf32::_read_section(std::istream & in, std::size_t index)
{
    return {};
}
