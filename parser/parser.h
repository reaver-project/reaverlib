/**
 * Reaver Library Licence
 *
 * Copyright (C) 2012-2013 Reaver Project Team:
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

#include <reaver/parser/lexer.h>

namespace reaver
{
    namespace parser
    {
        class parser
        {
        };

        template<typename T>
        class basic_parser : public parser
        {
        public:
            using value_type = T;

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            basic_parser(const lexer::token_description & desc)
            {
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            basic_parser(const lexer::token_definition<T> & def)
            {
            }

        private:
            uint64_t _expected_type;
        };

        // BIG TODO: template specializations so not and and parser are not handicapped
        // i.e., lack of ParserType::value_type shouldn't stop it from being used in other parsers

        template<typename T>
        class not_parser : public parser
        {
        };

        template<typename T>
        class and_parser : public parser
        {
        };

        template<typename T>
        class optional_parser : public parser
        {
            using value_type = boost::optional<typename T::value_type>;
        };

        template<typename T>
        class kleene_parser : public parser
        {
            using value_type = std::vector<typename T::value_type>;
        };

        template<typename T>
        class plus_parser : public parser
        {
            using value_type = std::vector<typename T::value_type>;
        };

        template<typename T, typename U>
        class variant_parser : public parser
        {
            using value_type = boost::variant<typename T::value_type, typename U::value_type>;
        };

        template<typename T, typename U>
        class sequence_parser : public parser
        {
            using value_type = std::tuple<typename T::value_type, typename U::value_type>;
        };

        template<typename T, typename U>
        class difference_parser : public parser
        {
            using value_type = typename T::value_type;
        };

        template<typename T, typename U>
        class seqor_parser : public parser
        {
            using value_type = std::tuple<boost::optional<typename T::value_type>, boost::optional<typename U::value_type>>;
        };

        template<typename T, typename U>
        class list_parser : public parser
        {
            using value_type = std::vector<typename T::value_type>;
        };

        template<typename T, typename U>
        class expect_parser;

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        not_parser<T> operator!(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        and_parser<T> operator&(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        optional_parser<T> operator-(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        plus_parser<T> operator+(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        kleene_parser<T> operator*(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        variant_parser<T, U> operator|(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        sequence_parser<T, U> operator<<(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        difference_parser<T, U> operator-(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        seqor_parser<T, U> operator||(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        list_parser<T, U> operator%(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }
    }
}
