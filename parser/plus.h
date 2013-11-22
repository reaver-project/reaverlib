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

#include <type_traits>
#include <vector>

#include <boost/optional.hpp>

#include "skip.h"
#include "helpers.h"
#include "../lexer/lexer.h"

namespace reaver
{
    namespace parser
    {
        template<typename Tref, typename = typename std::enable_if<is_optional<typename std::remove_reference<Tref>::type
            ::value_type>::value>::type>
        class plus_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = boost::optional<std::vector<typename T::value_type::value_type>>;

            plus_parser(const T & other) : _plus{ other }
            {
            }

            template<typename Iterator>
            value_type match(Iterator & begin, Iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename Iterator, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(Iterator & begin, Iterator end, Skip skip) const
            {
                value_type ret{ typename value_type::value_type{} };

                while (skip.match(begin, end)) {}
                auto val = _plus.match(begin, end, skip);

                if (!val)
                {
                    return {};
                }

                do
                {
                    ret->emplace_back(_detail::_constructor<typename value_type::value_type::value_type, typename T
                        ::value_type::value_type>::construct(*val));

                    while (skip.match(begin, end)) {}
                } while (val = _plus.match(begin, end, skip));

                return { std::move(ret) };
            }

        private:
            Tref _plus;
        };
    }
}
