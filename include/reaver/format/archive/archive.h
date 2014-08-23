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

#include <memory>
#include <string>
#include <istream>
#include <vector>
#include <map>
#include <fstream>

#include <boost/filesystem.hpp>

#include "error.h"

namespace reaver
{
    namespace format
    {
        namespace archive
        {
            class invalid_or_unsupported_file : public exception
            {
            public:
                invalid_or_unsupported_file() : exception{ logger::error }
                {
                    *this << "invalid or unsupported archive file format.";
                }
            };

            class invalid_stream : public exception
            {
            public:
                invalid_stream() : exception{ logger::error }
                {
                    *this << "invalid stream passed to reaver::format::archive.";
                }
            };

            class unknown_format : public exception
            {
            public:
                unknown_format(const std::string & str) : exception{ logger::error }
                {
                    *this << "unknown format: `" << style::style(style::colors::bgray, style::colors::def, style::styles::bold)
                        << str << style::style() << "`.";
                }
            };

            class file_already_present : public exception
            {
            public:
                file_already_present(const std::string & str) : exception{ logger::error }
                {
                    *this << "file `" << style::style(style::colors::bgray, style::colors::def, style::styles::bold) << str
                        << style::style() << "` already present in the archive.";
                }
            };

            enum class format
            {
                raf,
                craf
            };

            class archive;

            std::unique_ptr<archive> read(const std::string & filename);
            std::unique_ptr<archive> read(std::istream & stream);
            std::unique_ptr<archive> read(std::istream && stream);
            std::unique_ptr<archive> create(format fmt, const std::vector<std::string> & inputs, bool recursive = false);
            std::unique_ptr<archive> create(format fmt, const std::map<std::string, std::string> & inputs_map, bool recursive = false);
            std::unique_ptr<archive> create(format fmt, const std::map<std::string, std::unique_ptr<std::istream>> & stream_map);

            class archive
            {
            public:
                virtual ~archive() {}

                virtual std::unique_ptr<std::istream> operator[](const std::string & filename) const = 0;

                virtual void save(std::ostream & stream) const = 0;

                virtual void save(const std::string & filename) const
                {
                    save(std::fstream{ filename, std::ios::out | std::ios::binary });
                }

                virtual void save(std::ostream && stream) const
                {
                    save(stream);
                }

                virtual void unpack(const std::string & filename) const = 0;

                virtual void unpack(const boost::filesystem::path & path) const
                {
                    unpack(path.string());
                }

                virtual void add(const std::string & filename, std::istream & stream) = 0;

                virtual void add(const std::string & filename)
                {
                    add(filename, std::fstream{ filename, std::ios::in | std::ios::binary });
                }

                virtual void add(const std::string & target_filename, const std::string & input_filename)
                {
                    add(target_filename, std::fstream{ input_filename, std::ios::in | std::ios::binary });
                }

                virtual void add(const std::string & filename, std::istream && stream)
                {
                    add(filename, stream);
                }
            };
        }
    }
}
