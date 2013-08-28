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

#include "helpers.h"
#include "skip.h"
#include "../lexer.h"

namespace reaver
{
    namespace parser
    {
        namespace _detail
        {
            template<typename Ret, typename First, typename Second>
            struct _sequence_constructor_helper
            {
                static Ret construct(const First & f, const Second & s)
                {
                    return _constructor<Ret, First, Second>::construct(f, s);
                }
            };

            template<typename Ret, typename First>
            struct _sequence_constructor_helper<Ret, First, void>
            {
                static Ret construct(const First & f, bool)
                {
                    return _constructor<Ret, First>::construct(f);
                }
            };

            template<typename Ret, typename Second>
            struct _sequence_constructor_helper<Ret, void, Second>
            {
                static Ret construct(bool, const Second & s)
                {
                    return _constructor<Ret, Second>::construct(s);
                }
            };
        }

        template<typename Tref, typename Uref, typename = typename std::enable_if<
            std::is_base_of<parser, typename std::remove_reference<Tref>::type>::value
            && std::is_base_of<parser, typename std::remove_reference<Uref>::type>::value
            && !(std::is_same<typename std::remove_reference<Tref>::type::value_type, void>::value
            && std::is_same<typename std::remove_reference<Uref>::type::value_type, void>::value)
            && (std::is_same<typename std::remove_reference<Tref>::type::value_type, void>::value
                || is_vector<typename std::remove_reference<Tref>::type::value_type>::value
                || is_optional<typename std::remove_reference<Tref>::type::value_type>::value)
            && (std::is_same<typename std::remove_reference<Uref>::type::value_type, void>::value
                || is_vector<typename std::remove_reference<Uref>::type::value_type>::value
                || is_optional<typename std::remove_reference<Uref>::type::value_type>::value)>::type>
        class sequence_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = boost::optional<typename std::conditional<
                std::is_same<typename T::value_type, void>::value,
                typename remove_optional<typename U::value_type>::type,
                typename std::conditional<
                    std::is_same<typename U::value_type, void>::value,
                    typename remove_optional<typename T::value_type>::type,
                    typename make_tuple_type<typename remove_optional<typename T::value_type>::type,
                        typename remove_optional<typename U::value_type>::type>::type
                >::type
            >::type>;

            sequence_parser(const T & first, const U & second) : _first{ first }, _second{ second }
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
                auto first_matched = _first.match(b, end, skip);

                if (!_matched(first_matched))
                {
                    return {};
                }

                while (skip.match(b, end)) {}
                auto second_matched = _second.match(b, end, skip);

                if (!_matched(second_matched))
                {
                    return {};
                }

                begin = b;

                return _detail::_sequence_constructor_helper<value_type, typename _detail::_true_type<typename T::
                    value_type>::type, typename _detail::_true_type<typename U::value_type>::type>::construct(
                    _detail::_pass_true_type(first_matched), _detail::_pass_true_type(second_matched));
            }

        private:
            template<typename V>
            struct _checker
            {
                static bool matched(const V & v)
                {
                    return v;
                }
            };

            template<typename V>
            struct _checker<std::vector<V>>
            {
                static bool matched(const std::vector<V> &)
                {
                    return true;
                }
            };

            template<typename V>
            bool _matched(const V & v) const
            {
                return _checker<V>::matched(v);
            }

            Tref _first;
            Uref _second;
        };
    }
}
