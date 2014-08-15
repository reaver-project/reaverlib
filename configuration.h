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

#include <map>

#include <boost/type_index.hpp>
#include <boost/any.hpp>

namespace reaver { inline namespace _v1
{
    class configuration
    {
    public:
        template<typename T, typename... Args>
        void set(Args &&... args)
        {
            _map[boost::typeindex::type_id<T>()] = typename T::type{ std::forward<Args>(args)... };
        }

        template<typename T>
        typename T::type & get()
        {
            return boost::any_cast<typename T::type &>(_map.at(boost::typeindex::type_id<T>()));
        }

        template<typename T>
        const typename T::type & get() const
        {
            return boost::any_cast<const typename T::type &>(_map.at(boost::typeindex::type_id<T>()));
        }

    private:
        std::map<boost::typeindex::type_index, boost::any> _map;
    };
}}
