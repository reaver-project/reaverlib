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

                virtual bool save(std::string) const { return false; }
                virtual bool save(std::ostream &) const { return false; }

            private:
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
                    _386
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
                    unsigned char _ident[_nident];
                    enum _type _type;
                    enum _machine _machine;
                    enum _version _version;
                    _addr _entry;
                    _off _program_header_offset;
                    _off _section_header_offset;
                    _word _flags;
                    _half _header_size;
                    _half _program_header_entry_size;
                    _half _program_header_entry_count;
                    _half _section_header_entry_size;
                    _half _section_header_entry_count;
                    _half _string_section_header_index;
                };

                void _read_header(std::istream &);

                _header _elf_header;
            };
        }
    }
}
