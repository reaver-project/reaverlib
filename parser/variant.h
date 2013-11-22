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
        namespace _detail
        {
            template<typename>
            struct _is_variant_parser : public std::false_type
            {
            };

            template<typename T>
            struct _is_variant_parser<typename std::enable_if<is_variant<typename T::value_type::value_type>::value, T>::type>
                : public std::true_type
            {
            };
        }

        template<typename Tref, typename Uref, typename = typename std::enable_if<(is_optional<typename
            std::remove_reference<Tref>::type::value_type>::value || _detail::_is_variant_parser<Tref>::value)
            && (is_optional<typename std::remove_reference<Uref>::type::value_type>::value || _detail::_is_variant_parser<Uref>
            ::value)>::type>
        class variant_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = boost::optional<typename make_variant_type<typename remove_optional<typename T::value_type>::type,
                typename remove_optional<typename U::value_type>::type>::type>;

            variant_parser(const T & first, const U & second) : _first{ first }, _second{ second }
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
                auto b = begin;

                while (skip.match(b, end)) {}

                auto f = _first.match(b, end, skip);

                if (f)
                {
                    begin = b;
                    return { _detail::_constructor<value_type, typename T::value_type::value_type>::construct(*f) };
                }

                b = begin;
                auto s = _second.match(b, end, skip);

                if (s)
                {
                    begin = b;
                    return { _detail::_constructor<value_type, typename U::value_type::value_type>::construct(*s) };
                }

                return {};
            }

        private:
            Tref _first;
            Uref _second;
        };
    }
}
