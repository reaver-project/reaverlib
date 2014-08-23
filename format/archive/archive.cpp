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

#include <fstream>
#include <regex>

#include "format/archive.h"
#include "raf/raf.h"

std::unique_ptr<reaver::format::archive::archive> reaver::format::archive::read(const std::string & name)
{
    return read(std::ifstream{ name, std::ios::binary | std::ios::in });
}

std::unique_ptr<reaver::format::archive::archive> reaver::format::archive::read(std::istream && in)
{
    return read(in);
}

std::unique_ptr<reaver::format::archive::archive> reaver::format::archive::read(std::istream & in)
{
    if (!in)
    {
        throw invalid_stream{};
    }

    std::unique_ptr<reaver::format::archive::archive> ret = raf::read(in);

    if (!ret)
    {
        throw invalid_or_unsupported_file{};
    }

    return ret;
}

// TODO: this will require proper refactoring when other formats are supported
std::unique_ptr<reaver::format::archive::archive> reaver::format::archive::create(reaver::format::archive::format fmt,
    const std::vector<std::string> & inputs, bool recursive)
{
    if (fmt != format::raf)
    {
        return nullptr;
    }

    std::unique_ptr<archive> ret = std::make_unique<raf>();

    for (const auto & input : inputs)
    {
        if (boost::filesystem::exists(input))
        {
            if (boost::filesystem::is_directory(input))
            {
                if (recursive)
                {
                    for (boost::filesystem::recursive_directory_iterator it{ input }, end; it != end; ++it)
                    {
                        if (!boost::filesystem::is_directory(it->path()))
                        {
                            ret->add(it->path().string(), std::fstream{ it->path().string(), std::ios::in | std::ios::binary });
                        }
                    }
                }

                else
                {
                    throw reaver::exception(reaver::logger::error) << "cannot add a directory to an archive in non recursive mode; try adding -r to your RAF invocation.";
                }
            }

            else
            {
                ret->add(input, std::fstream{ input, std::ios::in | std::ios::binary });
            }
        }

        else
        {
            throw reaver::exception(reaver::logger::error) << "requested file `" << input << "` not found.";
        }
    }

    return ret;
}
