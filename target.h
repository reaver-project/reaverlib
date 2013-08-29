/**
 * Miranda License
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

#include <string>

#include "exception.h"

namespace reaver
{
    namespace target
    {
        enum class arch
        {
            uninitialized,
            i386,
            i486,
            i586,
            i686,
            i786,
            x86_64
        };

        enum class os
        {
            uninitialized,
            none,
            linux
        };

        enum class env
        {
            uninitialized,
            elf
        };

        class uninitialized_target : public exception
        {
        public:
            uninitialized_target() : exception{ logger::crash }
            {
                *this << "unitialized reaver::target::triple object used.";
            }
        };

        class unknown_architecture : public exception
        {
        public:
            unknown_architecture() : exception{ logger::crash }
            {
                *this << "unknown architecture specified in target triple.";
            }
        };

        class unknown_os : public exception
        {
        public:
            unknown_os() : exception{ logger::crash }
            {
                *this << "unknown OS specified in target triple.";
            }
        };

        class unknown_environment : public exception
        {
        public:
            unknown_environment() : exception{ logger::crash }
            {
                *this << "unknown environment specified in target triple.";
            }
        };

        class triple
        {
        public:
            triple(std::string);

            triple() : _arch{}, _os{}, _env{}
            {
            }

            triple(enum arch a, enum os o, enum env e) : _arch{ a }, _os{ o }, _env{ e }
            {
            }

            enum arch arch() const
            {
                if (_arch == ::reaver::target::arch::uninitialized || _os == ::reaver::target::os::uninitialized
                    || _env == ::reaver::target::env::uninitialized)
                {
                    throw uninitialized_target{};
                }

                return _arch;
            }

            enum os os() const
            {
                if (_arch == ::reaver::target::arch::uninitialized || _os == ::reaver::target::os::uninitialized
                    || _env == ::reaver::target::env::uninitialized)
                {
                    throw uninitialized_target{};
                }

                return _os;
            }

            enum env env() const
            {
                if (_arch == ::reaver::target::arch::uninitialized || _os == ::reaver::target::os::uninitialized
                    || _env == ::reaver::target::env::uninitialized)
                {
                    throw uninitialized_target{};
                }

                return _env;
            }

            std::string arch_string() const;
            std::string os_string() const;
            std::string env_string() const;

        private:
            enum arch _arch;
            enum os _os;
            enum env _env;
        };

        inline std::ostream & operator<<(std::ostream & o, const reaver::target::triple & t)
        {
            return o << t.arch_string() << '-' << t.os_string() << '-' << t.env_string();
        }
    }
}
