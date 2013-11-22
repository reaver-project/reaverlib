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

#include <memory>
#include <cstdint>

#include <boost/optional.hpp>

#include "elf.h"
#include "../../utils.h"

namespace reaver
{
    namespace format
    {
        namespace executable
        {
            class elf32 : public elf
            {
            public:
                static std::unique_ptr<executable> read(std::istream &);

            protected:
                using _addr = std::uint32_t;
                using _half = std::uint16_t;
                using _off = std::uint32_t;
                using _sword = std::int32_t;
                using _word = std::uint32_t;

                enum _ident_index
                {
                    _magic0_index,
                    _magic1_index,
                    _magic2_index,
                    _magic3_index,
                    _class_index,
                    _data_index,
                    _version_index,
                    _nident = 16
                };

                enum class _type : _half
                {
                    _none,
                    _relocatable,
                    _executable,
                    _shared,
                    _core
                };

                enum _machine : _half
                {
                    _none_machine,
                    _386 = 3
                };

                enum _version : _word
                {
                    _none_version,
                    _current
                };

                enum _data : unsigned char
                {
                    _invalid,
                    _lsb,
                    _msb
                };

                struct _header
                {
                    unsigned char ident[_nident];
                    _type type;
                    _machine machine;
                    _version version;
                    _addr entry;
                    _off program_header_offset;
                    _off section_header_offset;
                    _word flags;
                    _half header_size;
                    _half program_header_entry_size;
                    _half program_header_entry_count;
                    _half section_header_entry_size;
                    _half section_header_entry_count;
                    _half string_section_header_index;
                };

                struct _program_header
                {
                    _word type;
                    _off offset;
                    _addr virtual_address;
                    _addr physical_address;
                    _word file_size;
                    _word memory_size;
                    _word flags;
                    _word align;
                };

                enum class _section_type : _word
                {
                    _null,
                    _progbits,
                    _symtab,
                    _strtab,
                    _rela,
                    _hash,
                    _dynamic,
                    _note,
                    _nobits,
                    _rel,
                    _shlib,
                    _dynsym
                };

                enum _section_flags : _word
                {
                    _write = 0x1,
                    _alloc = 0x2,
                    _exec = 0x4
                };

                struct _section_header
                {
                    _word name;
                    _section_type type;
                    _word flags;
                    _addr address;
                    _off offset;
                    _word size;
                    _word link;
                    _word info;
                    _word align;
                    _word entry_size;
                };

                static _header _read_header(std::istream &);
                void _read_program_header(std::istream &);
                std::vector<_section_header> _read_section_headers(std::istream &);
                std::vector<std::uint8_t> _read_section(std::istream &, std::size_t);

                _header _elf_header;
                boost::optional<_program_header> _prog_header;
                section _section_names_string_table;
                section _string_table;
                std::vector<_section_header> _sections;
            };
        }
    }
}
