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
#include <type_traits>
#include <vector>
#include <exception>

#include <boost/optional.hpp>

#include "skip.h"
#include "helpers.h"
#include "converter.h"
#include "../lexer.h"

namespace reaver
{
    namespace parser
    {
        template<typename T>
        class limited_rule;

        template<typename T>
        class rule : public parser
        {
        public:
            using value_type = boost::optional<T>;

            friend class limited_rule<T>;

            rule() : _type{ -1 }, _converter{}
            {
            }

            rule(const rule & rhs) : _type{ rhs._type }, _converter{ rhs._converter }
            {
            }

            template<typename U, typename = typename std::enable_if<std::is_base_of<parser, U>::value>::type>
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

            template<typename U, typename = typename std::enable_if<std::is_base_of<parser, U>::value>::type>
            rule & operator=(const U & parser)
            {
                _type = -1;
                _converter.reset(new _detail::_converter_impl<T, U>{ parser });
                return *this;
            }

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            rule & operator=(const lexer::token_description & desc)
            {
                _type = desc.type();
                _converter.reset();
                return * this;
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            rule & operator=(const lexer::token_definition<T> & def)
            {
                _type = def.type();
                _converter.reset();
                return *this;
            }

            template<typename U>
            rule & operator=(const lexer::token_definition<U> &)
            {
                static_assert(std::is_same<T, U>::value, "You cannot create a rule directly from token definition with another match type.");
                return *this;
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>
                ::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                if (begin == end)
                {
                    return {};
                }

                if (_type != -1)
                {
                    if (begin->type() == _type)
                    {
                        return { (begin++)->as<T>() };
                    }

                    else
                    {
                        return {};
                    }
                }

                else if (_converter)
                {
                    _converter->set_skip(skip);

                    if (_converter->match(begin, end))
                    {
                        return _converter->get();
                    }

                    else
                    {
                        return {};
                    }
                }

                else
                {
                    throw std::runtime_error{ "called match on empty rule." };
                }
            }

            limited_rule<T> operator()(const std::vector<T> & add_allowed)
            {
                limited_rule<T> ret{ *this };
                return ret(add_allowed);
            }

        private:
            uint64_t _type;
            std::shared_ptr<_detail::_converter<T>> _converter;
        };

        template<typename T>
        rule<T> token(const lexer::token_definition<T> & def)
        {
            return rule<T>{ def };
        }

        template<typename T>
        rule<T> token(const lexer::token_description & desc)
        {
            return rule<T>{ desc };
        }

                template<typename T>
        class limited_rule : public parser
        {
        public:
            using value_type = boost::optional<T>;

            limited_rule() : _type{ -1 }
            {
            }

            limited_rule(const rule<T> & rhs) : _type{ rhs._type }
            {
            }

            limited_rule(const limited_rule & rhs) = default;

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            limited_rule(const lexer::token_description & desc) : _type{ desc.type() }
            {
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            limited_rule(const lexer::token_definition<T> & def) : _type{ def.type() }
            {
            }

            template<typename U>
            limited_rule(const lexer::token_definition<U> &)
            {
                static_assert(std::is_same<T, U>::value, "You cannot create a rule directly from token definition with another match type.");
            }

            // directly use lexer token description as a parser
            // in this case, the type check is done at runtime - therefore, produces less helpful error messages
            limited_rule & operator=(const lexer::token_description & desc)
            {
                _type = desc.type();
                return *this;
            }

            // directly use lexer token *definition* as a parser
            // in *this* case, the type check is done at compile time - error messages are compile time
            limited_rule & operator=(const lexer::token_definition<T> & def)
            {
                _type = def.type();
                return *this;
            }

            template<typename U>
            limited_rule & operator=(const lexer::token_definition<U> &)
            {
                static_assert(std::is_same<T, U>::value, "You cannot create a rule directly from token definition with another match type.");
                return *this;
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>
                ::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                if (!_allowed.size())
                {
                    return {};
                }

                while (skip.match(begin, end)) {}

                value_type ret;

                if (begin == end)
                {
                    return {};
                }

                if (_type != -1)
                {
                    if (begin->type() == _type)
                    {
                        ret = { (begin++)->as<T>() };
                    }

                    else
                    {
                        return {};
                    }
                }

                else
                {
                    throw std::runtime_error{ "called match on empty limited rule." };
                }

                if (!ret || std::find(_allowed.begin(), _allowed.end(), *ret) == _allowed.end())
                {
                    return {};
                }

                return ret;
            }

            limited_rule & operator()(const std::vector<T> & add_allowed)
            {
                std::copy(add_allowed.begin(), add_allowed.end(), std::back_inserter(_allowed));

                return *this;
            }

        private:
            uint64_t _type;
            std::vector<T> _allowed;
        };
    }
}
