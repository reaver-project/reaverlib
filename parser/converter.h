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

#include <vector>
#include <type_traits>

#include "../lexer.h"

namespace reaver
{
    namespace parser
    {
        namespace _detail
        {
            template<typename Ret>
            class _converter
            {
            public:
                virtual ~_converter() {}

                virtual bool match(std::vector<lexer::token>::const_iterator &, std::vector<lexer::token>::const_iterator)
                    const = 0;
                virtual Ret get() const = 0;

                template<typename T>
                void set_skip(T && skip)
                {
                    _skip.set_skip(skip);
                }

            protected:
                _skip_wrapper _skip;
            };

            template<typename Ret, typename Parserref, typename = typename std::enable_if<!std::is_same<typename
                std::remove_reference<Parserref>::type::value_type, void>::value>::type>
            class _converter_impl : public _converter<Ret>
            {
            public:
                using Parser = typename std::remove_reference<Parserref>::type;

                _converter_impl(const Parser & p) : _parser{ p }
                {
                }

                virtual bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>
                    ::const_iterator end) const
                {
                    while (this->_skip.match(begin, end)) {}

                    _value = _parser.match(begin, end, this->_skip);

                    return _is_matched(_value);
                }

                virtual Ret get() const
                {
                    return _constructor<Ret, typename _true_type<typename Parser::value_type>::type>::construct(
                        _pass_true_type(_value));
                }

            private:
                Parserref _parser;
                mutable typename Parser::value_type _value;
            };

            template<typename Parserref>
            class _converter_impl<void, Parserref> : public _converter<void>
            {
            public:
                using Parser = typename std::remove_reference<Parserref>::type;

                _converter_impl(const Parser & p) : _parser{ p }
                {
                }

                virtual bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>
                    ::const_iterator end) const
                {
                    while (this->_skip.match(begin, end)) {}

                    return _is_matched(_parser.match(begin, end, this->_skip));
                }

                virtual void get() const
                {
                }

            private:
                Parserref _parser;
            };
        }
    }
}
