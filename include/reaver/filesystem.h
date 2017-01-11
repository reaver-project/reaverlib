/**
 * Reaver Library Licence
 *
 * Copyright © 2014, 2016-2017 Michał "Griwes" Dominiak
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

#include <boost/filesystem.hpp>

#include "wildcard.h"

namespace reaver
{
namespace filesystem
{
    inline namespace _v1
    {
        inline boost::filesystem::path make_relative(const boost::filesystem::path & path,
            const boost::filesystem::path & base = boost::filesystem::current_path())
        {
            auto absolute_base = boost::filesystem::absolute(base);
            auto absolute_path = boost::filesystem::absolute(path);

            if (absolute_base == absolute_path)
            {
                return {};
            }

            boost::filesystem::path relative;

            auto base_it = absolute_base.begin();
            auto path_it = absolute_path.begin();

            if (*base_it != *path_it)
            {
                return path;
            }

            while (base_it != absolute_base.end() && path_it != absolute_path.end() && *base_it == *path_it)
            {
                ++base_it;
                ++path_it;
            }

            while (base_it != absolute_base.end())
            {
                relative /= "..";
                ++base_it;
            }

            while (path_it != absolute_path.end())
            {
                relative /= *path_it;
                ++path_it;
            }

            return relative;
        }

        inline std::vector<boost::filesystem::path> wildcard(const std::string & pat, const boost::filesystem::path & base = boost::filesystem::current_path())
        {
            auto pattern = make_relative(base / pat);
            std::vector<boost::filesystem::path> state{ make_relative(base) };
            auto end = boost::filesystem::directory_iterator{};

            for (auto && segment : boost::filesystem::path(pattern))
            {
                auto last_state = std::move(state);

                if (segment == "*")
                {
                    for (auto && candidate : last_state)
                    {
                        if (!boost::filesystem::is_directory(base / candidate))
                        {
                            continue;
                        }

                        for (auto it = boost::filesystem::directory_iterator{ base / std::move(candidate) }; it != end; ++it)
                        {
                            state.push_back(make_relative(*it, base));
                        }
                    }
                }

                else if (segment == "**")
                {
                    auto end = boost::filesystem::recursive_directory_iterator{};
                    for (auto && candidate : last_state)
                    {
                        if (!boost::filesystem::is_directory(base / candidate))
                        {
                            continue;
                        }

                        state.push_back(candidate);
                        for (auto it = boost::filesystem::recursive_directory_iterator{ base / std::move(candidate) }; it != end; ++it)
                        {
                            state.push_back(make_relative(*it, base));
                        }
                    }
                }

                else
                {
                    for (auto && candidate : last_state)
                    {
                        if (!boost::filesystem::is_directory(base / candidate))
                        {
                            continue;
                        }

                        for (auto it = boost::filesystem::directory_iterator{ base / std::move(candidate) }; it != end; ++it)
                        {
                            if (wildcard::match(segment.string(), it->path().filename().string()))
                            {
                                state.push_back(make_relative(*it, base));
                            }
                        }
                    }
                }
            }

            return state;
        }

        inline std::vector<boost::filesystem::path> all_symlinked_paths(boost::filesystem::path path)
        {
            std::vector<boost::filesystem::path> ret;
            ret.push_back(path);

            while (boost::filesystem::is_symlink(path))
            {
                path = boost::filesystem::canonical(boost::filesystem::read_symlink(path), path.parent_path());
                ret.push_back(path);
            }

            return ret;
        }
    }
}
}
