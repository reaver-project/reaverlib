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

#include <boost/filesystem.hpp>

namespace reaver
{
    namespace filesystem { inline namespace _v1
    {
        inline boost::filesystem::path make_relative(const boost::filesystem::path & base, const boost::filesystem::path & path)
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
    }}
}
