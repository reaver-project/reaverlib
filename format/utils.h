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

#include <cstdint>
#include <istream>

#include <boost/detail/endian.hpp>

#ifdef BOOST_PDP_ENDIAN
#error "PDP endianness is not supported."
#endif

namespace reaver
{
    namespace format
    {
        namespace utils
        {
            namespace _detail
            {
                template<typename T>
                T _swap(T i)
                {
                    union
                    {
                        T i;
                        unsigned char c[sizeof(T)];
                    } source, ret;

                    source.i = i;

                    for (std::size_t i = 0; i < sizeof(T); ++i)
                    {
                        ret.c[i] = source.c[sizeof(T) - i - 1];
                    }

                    return ret.i;
                }
            }

            template<typename T>
            T le_to_host(T i)
            {
#ifdef BOOST_LITTLE_ENDIAN
                return i;
#else
                return _detail::_swap(i);
#endif
            }

            template<typename T>
            T be_to_host(T i)
            {
#ifdef BOOST_BIG_ENDIAN
                return i;
#else
                return _detail::_swap(i);
#endif
            }

            template<typename T>
            void read(T & t, std::istream & in, bool little = true)
            {
                in.read(reinterpret_cast<char *>(&t), sizeof(T));

                if (little)
                {
                    t = le_to_host(t);
                }

                else
                {
                    t = be_to_host(t);
                }
            }
        }
    }
}
