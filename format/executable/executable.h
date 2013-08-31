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

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <istream>

#include "../../target.h"
#include "section.h"
#include "symbol.h"
#include "relocation.h"

namespace reaver
{
    namespace format
    {
        namespace executable
        {
            class invalid_or_unsupported_file : public exception
            {
            public:
                invalid_or_unsupported_file() : exception{ logger::crash }
                {
                    *this << "invalid or unsupported executable file format.";
                }
            };

            enum class format
            {
                elf,
                rxf                     // ReaverOS-specific executable format
                                        // more to follow...
            };

            enum class type
            {
                object_file,
                executable,
                static_library,
                dynamic_library,
                chunk_package           // ReaverOS-specific approach to dynamic linking
            };

            bool possible(format, type);

            class executable;

            std::unique_ptr<executable> open(std::string);
            std::unique_ptr<executable> read(std::istream &);
            std::unique_ptr<executable> create(format, type, const std::map<std::string, section> &, const std::map<
                std::string, symbol>, const std::vector<relocation> &, const std::vector<std::string> &);

            class executable
            {
            public:
                friend std::unique_ptr<executable> create(enum format, enum type, std::string, std::string, target::arch,
                    const std::map<std::string, section> &,  const std::map<std::string, symbol>, const std::vector<
                    relocation> &, const std::vector<std::string> &);

                virtual ~executable() {}

                std::string path() const
                {
                    return _path;
                }

                std::string name() const
                {
                    return _name;
                }

                enum format format() const
                {
                    return _format;
                }

                enum type type() const
                {
                    return _type;
                }

                reaver::target::arch architecture() const
                {
                    return _arch;
                }

                const std::map<std::string, section> & sections() const
                {
                    return _sections;
                }

                const std::map<std::string, symbol> & symbols() const
                {
                    return _symbols;
                }

                const std::map<std::string, relocation> & relocations() const
                {
                    return _relocations;
                }

                const std::vector<std::string> & libraries() const
                {
                    return _libraries;
                }

                virtual bool save(std::string) const = 0;
                virtual bool save(std::ostream &) const = 0;

            protected:
                std::string _name;
                std::string _path;
                enum format _format;
                enum type _type;
                reaver::target::arch _arch;

                std::map<std::string, section> _sections;
                std::map<std::string, symbol> _symbols;
                std::map<std::string, relocation> _relocations;
                std::vector<std::string> _libraries;
            };
        }
    }
}
