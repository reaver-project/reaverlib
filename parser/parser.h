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
#include <reaver/tmp.h>

namespace reaver
{
    namespace parser
    {
        namespace _detail
        {
            // TODO: deep removal of optionals when constructing variants

            template<typename Ret, typename... Args>
            struct _constructor
            {
                static Ret construct(const Args &... args)
                {
                    return Ret{ args... };
                }
            };

            template<typename Ret, typename... Args>
            struct _constructor<boost::optional<Ret>, Args...>
            {
                static boost::optional<Ret> construct(const Args &... args)
                {
                    return { Ret{ args... } };
                }
            };

            template<typename Ret, typename Arg>
            struct _constructor<Ret, boost::optional<Arg>>
            {
                static Ret construct(boost::optional<Arg> arg)
                {
                    return _constructor<Ret, Arg>::construct(*arg);
                }
            };

            template<typename Ret, typename Arg>
            struct _constructor<boost::optional<Ret>, boost::optional<Arg>>
            {
                static boost::optional<Ret> construct(boost::optional<Arg> arg)
                {
                    return { Ret{ *arg } };
                }
            };

            template<typename...>
            struct _unpacker;

            template<typename Ret, int... Seq>
            struct _unpacker<Ret, sequence<Seq...>>
            {
                template<typename T>
                static Ret _unpack(T && tuple)
                {
                    return Ret{ std::get<Seq>(tuple)... };
                }
            };

            template<typename Ret, typename T>
            struct _constructor<Ret, typename std::enable_if<is_tuple<T>::value, T>::type>
            {
                // construct an object from a tuple
                static Ret construct(const T & tuple)
                {
                    return _unpacker<Ret, generator<std::tuple_size<T>::value>>::unpack(std::forward<T>(tuple));
                }
            };

            template<typename Ret, typename T>
            struct _constructor<Ret, typename std::enable_if<is_vector<Ret>::value && is_vector<T>::value, T>::type>
            {
                // construct vector of objects from a vector of values
                static Ret construct(const T & vec)
                {
                    Ret ret(vec.size());

                    for (auto it = vec.begin(); it != vec.end(); ++it)
                    {
                        ret.emplace_back(_constructor<typename T::value_type>::construct(*it));
                    }

                    return ret;
                }
            };

            template<typename Ret, typename T1, typename T2>
            struct _constructor<Ret, T1, T2>
            {
                template<typename = typename std::enable_if<is_tuple<T1>::value && is_tuple<T2>::value>::type>
                static Ret construct(T1 && tuple1, T2 && tuple2)
                {
                    return _unpacker<Ret, generator<std::tuple_size<T1>::value>, generator<std::tuple_size<T2>::value>>
                        ::unpack(std::forward<T1>(tuple1), std::forward<T2>(tuple2));
                }
            };

            template<typename Ret>
            class _converter
            {
            public:
                virtual ~_converter() {}

                virtual Ret get(std::vector<lexer::token>::iterator &, std::vector<lexer::token>::iterator) = 0;
            };

            template<typename Ret, typename Parser, typename = typename std::enable_if<!std::is_same<typename Parser::value_type,
                void>::value>::type>
            class _converter_impl : public _converter<Ret>
            {
            public:
                _converter_impl(const Parser & p) : _parser{ p }
                {
                }

                virtual Ret get(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
                {
                    return _constructor<Ret, typename Parser::value_type>::construct(_parser.match(begin, end));
                }

            private:
                Parser _parser;
            };
        }

        class parser
        {
        };

        template<typename T>
        class rule : public parser
        {
        public:
            using value_type = boost::optional<T>;

            rule() : _type{ -1 }, _converter{}
            {
            }

            rule(const rule & rhs) : _type{ rhs._type }, _allowed_values{ rhs._allowed_values }, _converter{ rhs._converter }
            {
            }

            template<typename U, typename = typename std::enable_if<std::is_base_of<parser, U>::value &&
                std::is_constructible<T, typename U::value_type::value_type>::value>::type>
            rule(const U & p) : _type{ -1 }, _converter{ new _detail::_converter_impl<T, U>{ p } }
            {
            }

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            rule(const lexer::token_description & desc) : _type(desc.type())
            {
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            rule(const lexer::token_definition<T> & def) : _type(def.type())
            {
            }

            template<typename U>
            rule(const lexer::token_definition<U> &)
            {
                static_assert(std::is_same<T, U>::value, "You cannot create a rule directly from token definition with another match type.");
            }

            template<typename U, typename = typename std::enable_if<std::is_base_of<parser, U>::value &&
                std::is_constructible<T, typename U::value_type>::value>::type>
            rule & operator=(const U & parser)
            {
                _type = -1;
                _converter = new _detail::_converter_impl<T, U>{ parser };
                return *this;
            }

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            rule & operator=(const lexer::token_description & desc)
            {
                _type = desc.type();
                _converter = nullptr;
                return * this;
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            rule & operator=(const lexer::token_definition<T> & def)
            {
                _type = def.type();
                _converter = nullptr;
                return *this;
            }

            template<typename U>
            rule & operator=(const lexer::token_definition<U> &)
            {
                static_assert(std::is_same<T, U>::value, "You cannot create a rule directly from token definition with another match type.");
                return *this;
            }

            value_type match(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
            {
                if (begin == end)
                {
                    return {};
                }

                if (_type != -1)
                {
                    if (begin->type() == _type)
                    {
                        if (!_allowed_values.empty())
                        {
                            if (std::find(_allowed_values.begin(), _allowed_values.end(), begin->as<T>()) != _allowed_values.end())
                            {
                                return { (begin++)->as<T>() };
                            }

                            else
                            {
                                return {};
                            }
                        }

                        return { (begin++)->as<T>() };
                    }

                    else
                    {
                        return {};
                    }
                }

                else if (_converter)
                {
                    return _converter->get(begin, end);
                }

                else
                {
                    throw std::runtime_error{ "Called match on empty rule." };
                }
            }

        private:
            uint64_t _type;
            std::vector<T> _allowed_values; // if empty, any value is allowed - only has effect _type parser,
            // no effect on _parser parser

            std::shared_ptr<_detail::_converter<T>> _converter;
        };

        template<typename T>
        rule<T> token(const lexer::token_definition<T> & def)
        {
            return rule<T>{ def };
        }

        template<typename T>
        class not_parser : public parser
        {
        public:
            using value_type = void;

            not_parser(const T & np) : _negated{ np }
            {
            }

            bool match(std::vector<lexer::token>::iterator begin, std::vector<lexer::token>::iterator end)
            {
                return !_negated.match(begin, end);
            }

        private:
            T _negated;
        };

        template<typename T>
        class and_parser : public parser
        {
        public:
            using value_type = void;

            and_parser(const T & ap) : _and{ ap }
            {
            }

            bool match(std::vector<lexer::token>::iterator begin, std::vector<lexer::token>::iterator end)
            {
                return _and.match(begin, end);
            }

        private:
            T _and;
        };

        template<typename T, typename = typename std::enable_if<!std::is_same<typename T::value_type, void>::value>::type>
        class optional_parser : public parser
        {
        public:
            using value_type = boost::optional<typename T::value_type>;

            optional_parser(const T & opt) : _optional{ opt }
            {
            }

            value_type match(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
            {
                return _optional.match(begin, end);
            }

        private:
            T _optional;
        };

        template<typename T, typename = typename std::enable_if<is_optional<typename T::value_type>::value>::type>
        class kleene_parser : public parser
        {
        public:
            using value_type = std::vector<typename T::value_type::value_type>;

            kleene_parser(const T & other) : _kleene{ other }
            {
            }

            value_type match(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
            {
                value_type ret;

                boost::optional<typename value_type::value_type> val;

                while (val = _kleene.match(begin, end))
                {
                    val = _kleene.match(begin, end);
                    ret.emplace_back(_detail::_constructor<value_type, typename T::value_type>::construct(std::move(*val)));
                }

                return ret;
            }

        private:
            T _kleene;
        };

        template<typename T, typename = typename std::enable_if<is_optional<typename T::value_type>::value>::type>
        class plus_parser : public parser
        {
        public:
            using value_type = boost::optional<std::vector<typename T::value_type::value_type>>;

            plus_parser(const T & other) : _plus{ other }
            {
            }

            value_type match(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
            {
                value_type ret;

                boost::optional<typename value_type::value_type> val = _plus.match(begin, end);

                if (!val)
                {
                    return {};
                }

                return kleene_parser<T>{ _plus }.match(begin, end);
            }

        private:
            T _plus;
        };

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

        template<typename T, typename U, typename = typename std::enable_if<(is_optional<typename T::value_type>::value
            || _detail::_is_variant_parser<T>::value) && (is_optional<typename U::value_type>::value
            || _detail::_is_variant_parser<U>::value)>::type>
        class variant_parser : public parser
        {
        public:
            using value_type = boost::optional<typename make_variant_type<typename T::value_type, typename U::value_type>::type>;

            variant_parser(const T & first, const U & second) : _first{ first }, _second{ second }
            {
            }

            value_type match(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
            {
                auto f = _first.match(begin, end);

                if (f)
                {
                    return { _detail::_constructor<value_type, typename T::value_type>::construct(f) };
                }

                auto s = _second.match(begin, end);

                if (s)
                {
                    return { _detail::_constructor<value_type, typename U::value_type>::construct(s) };
                }

                return {};
            }

        private:
            T _first;
            U _second;
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

        template<typename T, typename U>
        class difference_parser : public parser
        {
        public:
            using value_type = typename T::value_type;

            difference_parser(const T & match, const T & dont) : _match{ match }, _dont{ dont }
            {
            }

            value_type match(std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
            {
                auto b1 = begin, b2 = begin;

                auto m = _match.match(b1, end);
                auto d = _dont.match(b2, end);

                if (m && !d)
                {
                    begin = b1;
                    return { m };
                }

                return {};
            }

        private:
            T _match;
            U _dont;
        };

        template<typename T, typename U>
        class seqor_parser : public parser
        {
            using value_type = typename std::conditional<
                std::is_same<typename T::value_type, void>::value,
                typename std::conditional<
                    std::is_same<typename U::value_type, void>::value,
                    void,
                    boost::optional<typename U::value_type>
                >::type,
                typename std::conditional<
                    std::is_same<typename U::value_type, void>::value,
                    boost::optional<typename T::value_type>,
                    std::tuple<boost::optional<typename T::value_type>, boost::optional<typename U::value_type>>
                >::type
            >::type;
        };

        template<typename T, typename U>
        class list_parser : public parser
        {
            using value_type = std::vector<typename T::value_type>; // no void, sorry - you can hardly have a list of voids
            // and it hardly makes any sense
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
