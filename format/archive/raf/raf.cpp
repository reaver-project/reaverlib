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

#include "raf.h"

namespace
{
    namespace _raf_version_1
    {
        struct _raf_header
        {
            std::uint8_t magic[4] = { 'r', 'a', 'f',  0xab };    // 'r' 'a' 'f' 0xAB
            std::uint8_t version = 1;                            // currently at 1
            std::uint8_t checksum = 0;
            std::uint8_t reserved[2] = {};
            std::uint64_t created = 0;                           // in Rose Epoch
            std::uint64_t modified = 0;                          // in Rose Epoch
            std::uint64_t size = sizeof(_raf_header);
            std::uint64_t file_count;
            std::uint64_t reserved2[3] = {};

            bool validate() const
            {
                if (magic[0] != 'r' || magic[1] != 'a' || magic[2] != 'f' || magic[3] != 0xab || version == 0)
                {
                    return false;
                }

                const std::uint8_t * ptr = reinterpret_cast<const std::uint8_t *>(this);
                std::uint8_t sum = 0;

                for (std::uint64_t i = 0; i < sizeof(_raf_header); ++i, ++ptr)
                {
                    sum += *ptr;
                }

                return !sum;
            }
        } __attribute__((packed));

        struct _file_header
        {
            std::uint64_t offset;
            std::uint64_t size;
            std::uint64_t filename_length;
            char filename[1];
        } __attribute__((packed));
    }
}

std::unique_ptr<reaver::format::archive::raf> reaver::format::archive::raf::read(std::istream & in)
{
    return nullptr;
}

std::unique_ptr<std::istream> reaver::format::archive::raf::operator[](const std::string & filename) const
{
    return nullptr;
}

void reaver::format::archive::raf::save(std::ostream & output) const
{
    if (!output)
    {
        throw invalid_stream{};
    }

    _raf_version_1::_raf_header header;
    header.file_count = _files.size();

    std::unordered_map<std::string, _raf_version_1::_file_header> file_headers;
    std::uintmax_t current_data_offset = 0;
    for (const auto & e : _files)
    {
        _raf_version_1::_file_header h;
        h.offset = current_data_offset;
        h.size = e.second.size();
        h.filename_length = e.first.size() - 1;

        file_headers.emplace(e.first, h);

        current_data_offset += h.size;
        header.size += sizeof(_raf_version_1::_file_header) + h.filename_length;
    }

    std::uintmax_t headers_size = header.size;
    header.size += current_data_offset;

    // checksum
    std::uint8_t checksum = 0;
    const std::uint8_t * ptr = reinterpret_cast<const std::uint8_t *>(&header);
    for (std::uint64_t i = 0; i < sizeof(_raf_version_1::_raf_header); ++i, ++ptr)
    {
        checksum -= *ptr;
    }

    header.checksum = checksum;

    assert(header.validate());

    output.write(reinterpret_cast<char *>(&header), sizeof(_raf_version_1::_raf_header));

    for (const auto & e : _files)
    {
        file_headers[e.first].offset += headers_size;
        output.write(reinterpret_cast<char *>(&file_headers[e.first]), sizeof(_raf_version_1::_file_header) - 1);
        output.write(&e.first[0], e.first.size());
    }

    for (const auto & e : _files)
    {
        output.write(&e.second[0], e.second.size());
    }
}

void reaver::format::archive::raf::unpack(const std::string & output_path) const
{
    throw reaver::exception(reaver::logger::error) << "unpacking RAF archives not supported yet.";
}

void reaver::format::archive::raf::add(const std::string & destination_filename, std::istream & input)
{
    if (!input)
    {
        throw invalid_stream{};
    }

    if (_files.count(destination_filename))
    {
        throw file_already_present{ destination_filename };
    }

    _files.emplace(destination_filename, std::vector<char>{ std::istreambuf_iterator<char>{ input.rdbuf() }, std::istreambuf_iterator<char>{} });
}
