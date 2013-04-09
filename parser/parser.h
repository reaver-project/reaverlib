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

/**
 * Watch out, crazy template code ahead. All hope abandon ye who enter here.
 *
 * Lexer is full of run-time, er, somethings, this one is full of compile time
 * ones.
 */

#pragma once

#include <type_traits>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <reaver/parser/lexer.h>

namespace reaver
{
    namespace parser
    {
        // TMP classes
        template<typename T>
        struct is_vector : public std::false_type
        {
        };

        template<typename T, typename A>
        struct is_vector<std::vector<T, A>> : public std::true_type
        {
        };

        template<typename T>
        struct is_tuple : public std::false_type
        {
        };

        template<typename... Ts>
        struct is_tuple<std::tuple<Ts...>> : public std::true_type
        {
        };

        template<typename T, typename U>
        struct make_tuple_type
        {
            using type = std::tuple<T, U>;
        };

        template<typename T, typename... Ts>
        struct make_tuple_type<T, std::tuple<Ts...>>
        {
            using type = std::tuple<T, Ts...>;
        };

        template<typename... Ts, typename T>
        struct make_tuple_type<std::tuple<Ts...>, T>
        {
            using type = std::tuple<Ts..., T>;
        };

        template<typename... T1s, typename... T2s>
        struct make_tuple_type<std::tuple<T1s...>, std::tuple<T2s...>>
        {
            using type = std::tuple<T1s..., T2s...>;
        };

        class parser
        {
        };

        template<typename T>
        class rule : public parser
        {
        public:
            using value_type = T;

            template<typename U, typename = typename std::enable_if<std::is_base_of<parser, U>::value &&
                std::is_constructible<T, typename U::value_type>::value>::type>
            rule(const U & parser)
            {
            }

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            rule(const lexer::token_description & desc)
            {
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            rule(const lexer::token_definition<T> & def)
            {
            }

            template<typename U>
            rule(const lexer::token_definition<U> &)
            {
                static_assert(false, "You cannot create a rule directly from token definition with another match type.");
            }

            template<typename U, typename = typename std::enable_if<std::is_base_of<parser, U>::value &&
                std::is_constructible<T, typename U::value_type>::value>::type>
            rule & operator=(const U & parser)
            {
                return *this;
            }

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            rule & operator=(const lexer::token_description & desc)
            {
                return * this;
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            rule & operator=(const lexer::token_definition<T> & def)
            {
                return *this;
            }

            template<typename U>
            rule & operator=(const lexer::token_definition<U> &)
            {
                static_assert(false, "You cannot create a rule directly from token definition with another match type.");
                return *this;
            }

        private:
            uint64_t _expected_type;
        };

        template<typename T>
        rule<T> & token(const lexer::token_definition<T> & def)
        {
            return { def };
        }

        template<typename T>
        class not_parser : public parser
        {
            using value_type = void;
        };

        template<typename T>
        class and_parser : public parser
        {
            using value_type = void;
        };

        template<typename T>
        class optional_parser : public parser
        {
            using value_type = typename std::conditional<std::is_same<typename T::value_type, void>::value,
                boost::optional<typename T::value_type>, void>::type;
        };

        template<typename T>
        class kleene_parser : public parser
        {
            using value_type = typename std::conditional<std::is_same<typename T::value_type, void>::value,
                std::vector<typename T::value_type>, void>::type;
        };

        template<typename T>
        class plus_parser : public parser
        {
            using value_type = typename std::conditional<std::is_same<typename T::value_type, void>::value,
                std::vector<typename T::value_type>, void>::type;
        };

        template<typename T, typename U>
        class variant_parser : public parser
        {
            using value_type = typename std::conditional<
                std::is_same<typename T::value_type, void>::value,
                typename std::conditional<
                    std::is_same<typename U::value_type, void>::value,
                    void,
                    boost::optional<typename U::value_type>
                >::type,
                boost::variant<typename T::value_type, typename U::value_type>
            >::type;
        };

        template<typename T, typename U>
        class sequence_parser : public parser
        {
            using value_type = typename std::conditional<
                !std::is_same<typename T::value_type, void>::value
                    && std::is_same<typename T::value_type, typename U::value_type>::value,
                typename std::conditional<
                    is_vector<typename T::value_type>::value,
                    typename T::value_type,
                    std::vector<typename T::value_type>
                >::type,
                typename std::conditional<
                    std::is_same<typename T::value_type, void>::value,
                    typename std::conditional<
                        std::is_same<typename U::value_type, void>::value,
                        void,
                        std::vector<typename U::value_type>
                    >::type,
                    typename make_tuple_type<typename T::value_type, typename U::value_type>::type
                >::type
            >::type; // this is not even my final form! ...and probably some bugs on the way
        };

        // TODO: finish and check this crazy template wankery

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
