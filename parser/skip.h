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

#include <memory>
#include <vector>

#include "helpers.h"
#include "../lexer.h"

namespace reaver
{
    namespace parser
    {
        namespace _detail
        {
            class _skip_wrapper_impl_base
            {
            public:
                virtual ~_skip_wrapper_impl_base() {}

                virtual bool match(std::vector<lexer::token>::const_iterator &, std::vector<lexer::token>::const_iterator)
                    const = 0;
            };

            template<typename T>
            class _skip_wrapper_impl : public _skip_wrapper_impl_base
            {
            public:
                _skip_wrapper_impl(const T & skip) : _skip_ref{ skip }
                {
                }

                virtual bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>
                    ::const_iterator end) const
                {
                    return _skip_ref.match(begin, end);
                }

            private:
                T _skip_ref;
            };

            class _skip_wrapper : public parser
            {
            public:
                bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
                {
                    return _skip->match(begin, end);
                }

                template<typename T>
                void set_skip(T && t)
                {
                    _skip = std::shared_ptr<_skip_wrapper_impl_base>{ new _skip_wrapper_impl<T>{ t } };
                }

            private:
                std::shared_ptr<_skip_wrapper_impl_base> _skip;
            };

            class _def_skip : public parser
            {
            public:
                using value_type = bool;

                bool match(std::vector<lexer::token>::const_iterator &, std::vector<lexer::token>::const_iterator) const
                {
                    return false;
                }
            };
        }
    }
}
