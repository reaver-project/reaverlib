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

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <istream>
#include <regex>
#include <functional>

#include <boost/lexical_cast.hpp>

namespace reaver
{
    namespace lexer
    {
        template<typename Out, typename In>
        Out convert(const In & in)
        {
            return boost::lexical_cast<Out>(in);
        }

        namespace _detail
        {
            struct _token
            {
                virtual ~_token() {}
            };

            template<typename T>
            struct _token_impl : public _token
            {
                _token_impl(const T & t) : match{ t }
                {
                }

                T match;
            };
        }

        class token
        {
        public:
            token(uint64_t type) : _type{ type }
            {
            }

            template<typename T>
            token(uint64_t type, const T & t) : _type{ type }, _token{ new _detail::_token_impl<T>{ t } }
            {
            }

            template<typename T>
            T as() const
            {
                if (dynamic_cast<_detail::_token_impl<T> *>(&*_token))
                {
                    return dynamic_cast<_detail::_token_impl<T> *>(&*_token)->match;
                }

                else
                {
                    throw std::bad_cast{};
                }
            }

            uint64_t type() const
            {
                return _type;
            }

        private:
            uint64_t _type;
            std::shared_ptr<_detail::_token> _token;
        };

        namespace _detail
        {
            template<typename CharType>
            class _token_description
            {
            public:
                virtual ~_token_description() {}

                virtual token match(typename std::basic_string<CharType>::const_iterator &, typename std::basic_string<CharType>::const_iterator) = 0;
                virtual uint64_t type() = 0;
            };

            template<typename CharType, typename T>
            class _token_description_impl : public _token_description<CharType>
            {
            public:
                template<typename F>
                _token_description_impl(uint64_t type, std::basic_regex<CharType> regex, F converter) : _type{ type }, _regex{ regex },
                    _converter{ converter }
                {
                }

                virtual token match(typename std::basic_string<CharType>::const_iterator & begin, typename std::basic_string<CharType>::const_iterator end)
                {
                    std::match_results<typename std::basic_string<CharType>::const_iterator> match;

                    if (std::regex_search(begin, end, match, _regex))
                    {
                        if (match[0].first - begin == 0)
                        {
                            begin += match[0].str().length();
                            return token{ _type, _converter(match[0].str()) };
                        }
                    }

                    return token{ -1 };
                }

                virtual uint64_t type()
                {
                    return _type;
                }

            private:
                uint64_t _type;
                std::basic_regex<CharType> _regex;
                std::function<T (const std::basic_string<CharType> &)> _converter;
            };
        }

        template<typename>
        class basic_token_description;

        template<typename CharType, typename T = std::basic_string<CharType>>
        class basic_token_definition
        {
        public:
            friend class basic_token_description<CharType>;

            using value_type = T;

            basic_token_definition(uint64_t type, std::basic_string<CharType> regex) : _desc{ new _detail::_token_description_impl<CharType, T>
                { type, std::basic_regex<CharType>{ regex, std::regex::extended }, [](const std::basic_string<CharType> & str) { return convert<T>(str); } } }
            {
            }

            template<typename F>
            basic_token_definition(uint64_t type, std::basic_string<CharType> regex, F converter) : _desc
                { new _detail::_token_description_impl<CharType, T>{ type, std::basic_regex<CharType>{ regex, std::regex::extended }, converter } }
            {
            }

            token match(typename std::basic_string<CharType>::const_iterator & begin, typename std::basic_string<CharType>::const_iterator end) const
            {
                return _desc->match(begin, end);
            }

            uint64_t type() const
            {
                return _desc->type();
            }

        private:
            std::shared_ptr<_detail::_token_description_impl<CharType, T>> _desc;
        };

        template<typename T>
        struct match_type
        {
        };

        template<typename CharType>
        class basic_token_description
        {
        public:
            template<typename T>
            basic_token_description(const basic_token_definition<CharType, T> & def) : _desc{ def._desc }
            {
            }

            basic_token_description(uint64_t type, std::basic_string<CharType> regex) : _desc{ new _detail::_token_description_impl<CharType, std::basic_string<CharType>>
                { type, std::basic_regex<CharType>{ regex, std::regex::extended }, [](const std::basic_string<CharType> & str) { return str; } } }
            {
            }

            template<typename T>
            basic_token_description(uint64_t type, std::basic_string<CharType> regex, match_type<T>) : _desc{ new _detail::_token_description_impl<CharType, T>
                { type, std::basic_regex<CharType>{ regex, std::regex::extended }, [](const std::basic_string<CharType> & str) { return convert<T>(str); } } }
            {
            }

            template<typename T, typename F>
            basic_token_description(uint64_t type, std::basic_string<CharType> regex, match_type<T>, F converter) : _desc
                { new _detail::_token_description_impl<CharType, T>{ type, std::basic_regex<CharType>{ regex, std::regex::extended }, converter } }
            {
            }

            token match(typename std::basic_string<CharType>::const_iterator & begin, typename std::basic_string<CharType>::const_iterator end) const
            {
                return _desc->match(begin, end);
            }

            uint64_t type() const
            {
                return _desc->type();
            }

        private:
            std::shared_ptr<_detail::_token_description<CharType>> _desc;
        };

        template<typename CharType>
        class basic_tokens_description
        {
        public:
            basic_tokens_description() {}

            struct _inserter
            {
                _inserter(basic_tokens_description<CharType> & parent) : _parent{ parent }
                {
                }

                _inserter & operator()(const basic_token_description<CharType> & desc)
                {
                    _parent._descs.emplace(std::make_pair(desc.type(), desc));
                    return *this;
                }

                template<typename... Args>
                _inserter & operator()(Args &&... args)
                {
                    (*this)({ std::forward<Args>(args)... });
                    return *this;
                }

                _inserter & operator()(const std::string & alias, uint64_t type)
                {
                    _parent._aliases[alias] = type;
                    return *this;
                }

            private:
                basic_tokens_description<CharType> & _parent;
            };

            friend struct _inserter;

            _inserter add(const basic_token_description<CharType> & desc)
            {
                _descs.emplace(std::make_pair(desc.type(), desc));
                return { *this };
            }

            template<typename... Args>
            _inserter add(Args &&... args)
            {
                add({ std::forward<Args>(args)... });
                return { *this };
            }

            _inserter add(const std::string & alias, uint64_t type)
            {
                _aliases[alias] = type;
            }

            typename std::map<uint64_t, basic_token_description<CharType>>::const_iterator begin() const
            {
                return _descs.begin();
            }

            typename std::map<uint64_t, basic_token_description<CharType>>::const_iterator end() const
            {
                return _descs.end();
            }

            basic_token_description<CharType> operator[](uint64_t type)
            {
                return _descs.at(type);
            }

            basic_token_description<CharType> operator[](const std::string & alias)
            {
                return _descs.at(_aliases.at(alias));
            }

        private:
            std::map<uint64_t, basic_token_description<CharType>> _descs;
            std::map<std::string, uint64_t> _aliases;
        };

        class unexpected_characters : public std::runtime_error
        {
        public:
            unexpected_characters() : std::runtime_error{ "Unexpected characters in tokenized string; tokenization failed." }
            {
            }
        };

        template<typename CharType>
        std::vector<token> tokenize(const std::basic_string<CharType> & str, const basic_tokens_description<CharType> & def)
        {
            std::vector<token> ret;

            auto begin = str.begin(), end = str.end();

            while (begin != end)
            {
                auto b = def.begin(), e = def.end();

                for (; b != e; ++b)
                {
                    token matched = b->second.match(begin, end);

                    if (matched.type() != -1)
                    {
                        ret.push_back(matched);

                        break;
                    }
                }

                if (b == e)
                {
                    throw unexpected_characters{};
                }
            }

            return ret;
        }

        template<typename CharType, typename InputIterator>
        std::vector<token> tokenize(InputIterator begin, InputIterator end, const basic_tokens_description<CharType> & def)
        {
            return tokenize({ begin, end }, def);
        }

        template<typename CharType>
        std::vector<token> tokenize(std::basic_istream<CharType> & str, const basic_tokens_description<CharType> & def)
        {
            return tokenize(std::istreambuf_iterator<CharType>(str.rdbuf()), std::istreambuf_iterator<CharType>(), def);
        }

        template<typename CharType, uint64_t N>
        std::vector<token> tokenize(const CharType (&str)[N], const basic_tokens_description<CharType> & def)
        {
            return tokenize({ str, str + N}, def);
        }

        using token_description = basic_token_description<char>;
        template<typename T>
        using token_definition = basic_token_definition<char, T>;
        using tokens_description = basic_tokens_description<char>;
    }
}
