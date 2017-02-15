/**
 * Reaver Library Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

#include <pthread.h>

// on linuxes, this will pretty much always include pthread, so the above
// include is not harmful
// (it'd be if it was the only one that included the symbols)
#include "exception.h"

namespace reaver
{
inline namespace _v1
{
    class tls_creation_exception : public exception
    {
    public:
        tls_creation_exception() : exception{ logger::error }
        {
            *this << "Failed to create a TLS key.";
        }
    };

    namespace _detail
    {
#ifdef __unix__
        using _handle = pthread_key_t;
        inline void _initialize(_handle & h)
        {
            if (pthread_key_create(&h, nullptr))
            {
                throw tls_creation_exception{};
            }
        }

        inline void * _read(_handle h)
        {
            return pthread_getspecific(h);
        }

        inline void _write(_handle h, void * ptr)
        {
            pthread_setspecific(h, ptr);
        }
#endif
    }

    template<typename T>
    class tls_variable
    {
    public:
        static_assert(sizeof(T) <= sizeof(void *),
            "tls_variable is currently only available for "
            "types of sizes up to the size of `void *`.");
        static_assert(std::is_trivially_destructible<T>::value && std::is_trivial<T>::value, "tls_variable is currently only available for trivial types.");

        tls_variable(T initial = T{})
        {
            _detail::_initialize(_handle);
            *this = initial;
        }

        operator T() const
        {
            // to avoid errors about narrowing conversions
            auto ptr = _detail::_read(_handle);
            return *reinterpret_cast<T *>(&ptr);
        }

        auto & operator=(const T & t)
        {
            _detail::_write(_handle, reinterpret_cast<void *>(t));
            return *this;
        }

    private:
        _detail::_handle _handle;
    };
}
}
