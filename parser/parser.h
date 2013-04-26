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
 * ones. And run-time too. I *did* write `all hope abandon`, didn't I?
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
        class parser
        {
        };

        namespace _detail
        {
            template<typename Ret, typename... Args>
            struct _constructor
            {
                static Ret construct(const Args &... args)
                {
                    return Ret{ args... };
                }
            };

            template<typename T>
            struct _constructor<T, T>
            {
                static const T & construct(const T & t)
                {
                    return t;
                }
            };

            template<typename...>
            struct _unpacker;

            // C++11 variadic templates are dumb
            // you cannot even do struct _constructor<Args1..., std::tuple<TupleTypes...>, Args2...> :F

            // 0 args before tuple
            template<int... I, typename Ret, typename... TupleTypes, typename... Args>
            struct _unpacker<sequence<I...>, Ret, std::tuple<TupleTypes...>, Args...>
            {
                static Ret unpack(const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return _constructor<Ret, TupleTypes..., Args...>::construct(std::get<I>(t)..., args...);
                }
            };

            template<typename Ret, typename... TupleTypes, typename... Args>
            struct _constructor<Ret, std::tuple<TupleTypes...>, Args...>
            {
                static Ret construct(const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return _unpacker<typename generator<sizeof...(TupleTypes)>::type, Ret, std::tuple<TupleTypes...>,
                        Args...>::unpack(t, args...);
                }
            };

            template<typename... TupleTypes>
            struct _constructor<std::tuple<TupleTypes...>, std::tuple<TupleTypes...>>
            {
                static const std::tuple<TupleTypes...> & construct(const std::tuple<TupleTypes...> & t)
                {
                    return t;
                }
            };

            template<typename Ret, typename... TupleTypes, typename... Args>
            struct _constructor<boost::optional<Ret>, std::tuple<TupleTypes...>, Args...>
            {
                static boost::optional<Ret> construct(const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return { _unpacker<typename generator<sizeof...(TupleTypes)>::type, Ret, std::tuple<TupleTypes...>,
                        Args...>::unpack(t, args...) };
                }
            };

            // 1 arg before tuple
            template<int... I, typename Ret, typename First, typename... TupleTypes, typename... Args>
            struct _unpacker<sequence<I...>, Ret, First, std::tuple<TupleTypes...>, Args...>
            {
                static Ret unpack(const First & f, const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return _constructor<Ret, First, TupleTypes..., Args...>::construct(f, std::get<I>(t)..., args...);
                }
            };

            template<typename Ret, typename First, typename... TupleTypes, typename... Args>
            struct _constructor<Ret, First, std::tuple<TupleTypes...>, Args...>
            {
                static Ret construct(const First & f, const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return _unpacker<typename generator<sizeof...(TupleTypes)>::type, First, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, t, args...);
                }
            };

            template<typename Ret, typename First, typename... TupleTypes, typename... Args>
            struct _constructor<boost::optional<Ret>, First, std::tuple<TupleTypes...>, Args...>
            {
                static boost::optional<Ret> construct(const First & f, const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return { _unpacker<typename generator<sizeof...(TupleTypes)>::type, First, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, t, args...) };
                }
            };

            // 2 args before tuple
            template<int... I, typename Ret, typename First, typename Second, typename... TupleTypes, typename... Args>
            struct _unpacker<sequence<I...>, Ret, First, Second, std::tuple<TupleTypes...>, Args...>
            {
                static Ret unpack(const First & f, const Second & s, const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return _constructor<Ret, First, Second, TupleTypes..., Args...>::construct(f, s, std::get<I>(t)..., args...);
                }
            };

            template<typename Ret, typename First, typename Second, typename... TupleTypes, typename... Args>
            struct _constructor<Ret, First, Second, std::tuple<TupleTypes...>, Args...>
            {
                static Ret construct(const First & f, const Second & s, const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return _unpacker<typename generator<sizeof...(TupleTypes)>::type, First, Second, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, s, t, args...);
                }
            };

            template<typename Ret, typename First, typename Second, typename... TupleTypes, typename... Args>
            struct _constructor<boost::optional<Ret>, First, Second, std::tuple<TupleTypes...>, Args...>
            {
                static boost::optional<Ret> construct(const First & f, const Second & s, const std::tuple<TupleTypes...> & t,
                    const Args &... args)
                {
                    return { _unpacker<typename generator<sizeof...(TupleTypes)>::type, First, Second, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, s, t, args...) };
                }
            };

            template<typename Ret, typename... Args>
            struct _constructor<boost::optional<Ret>, Args...>
            {
                static boost::optional<Ret> construct(const Args &... args)
                {
                    return { _constructor<Ret, Args...>::construct(args...) };
                }
            };

            template<typename V>
            struct _true_type
            {
                using type = typename V::value_type;
            };

            template<>
            struct _true_type<void>
            {
                using type = void;
            };

            template<>
            struct _true_type<bool>
            {
                using type = bool;
            };

            template<typename V>
            struct _true_type<std::vector<V>>
            {
                using type = std::vector<V>;
            };

            template<typename V>
            typename _true_type<V>::type _pass_true_type(const V & t)
            {
                return *t;
            }

            template<typename V>
            const std::vector<V> & _pass_true_type(const std::vector<V> & v)
            {
                return v;
            }

            bool _pass_true_type(bool b)
            {
                return b;
            }

            template<typename V>
            bool _is_matched(const V & t)
            {
                return t;
            }

            template<typename V>
            bool _is_matched(const std::vector<V> & v)
            {
                return true;
            }

            bool _is_matched(bool b)
            {
                return b;
            }

            template<typename T>
            struct _variant_converter : public boost::static_visitor<T>
            {
                template<typename Arg>
                T operator()(const Arg & a) const
                {
                    return _constructor<T, Arg>::construct(a);
                }
            };

            template<typename T, typename... Ts>
            T _get_variant_as(const boost::variant<Ts...> & v)
            {
                return boost::apply_visitor(_variant_converter<T>{}, v);
            }

            template<typename Ret, typename... Ts>
            struct _constructor<Ret, boost::variant<Ts...>>
            {
                static Ret construct(const boost::variant<Ts...> & v)
                {
                    return _get_variant_as<Ret>(v);
                }
            };

            template<typename Ret, typename... Ts>
            struct _constructor<boost::optional<Ret>, boost::variant<Ts...>>
            {
                static boost::optional<Ret> construct(const boost::variant<Ts...> & v)
                {
                    return { _get_variant_as<Ret>(v) };
                }
            };

            template<typename... Ts, typename... Us>
            struct _constructor<boost::variant<Ts...>, boost::variant<Us...>>
            {
                static boost::variant<Ts...> construct(const boost::variant<Us...> & v)
                {
                    return { v };
                }
            };

            template<typename... Ts>
            struct _constructor<boost::variant<Ts...>, boost::variant<Ts...>>
            {
                static const boost::variant<Ts...> & construct(const boost::variant<Ts...> & v)
                {
                    return v;
                }
            };

            template<typename Ret, typename T>
            struct _constructor<std::vector<Ret>, std::vector<T>>
            {
                // construct vector of objects from a vector of values
                static std::vector<Ret> construct(const std::vector<T> & vec)
                {
                    std::vector<Ret> ret;
                    ret.reserve(vec.size());

                    for (auto it = vec.begin(); it != vec.end(); ++it)
                    {
                        ret.emplace_back(_constructor<Ret, typename _true_type<T>::type>::construct(_pass_true_type(*it)));
                    }

                    return ret;
                }
            };

            template<typename T>
            struct _constructor<std::vector<T>, std::vector<T>>
            {
                static const std::vector<T> & construct(const std::vector<T> & vec)
                {
                    return vec;
                }
            };

            template<typename Ret, typename... TupleTypes>
            struct _constructor<boost::optional<Ret>, std::vector<std::tuple<TupleTypes...>>>
            {
                static boost::optional<Ret> construct(const std::vector<std::tuple<TupleTypes...>> & vt)
                {
                    return { _constructor<Ret, std::vector<std::tuple<TupleTypes...>>>::construct(vt) };
                }
            };

            template<typename Ret, typename First, typename... TupleTypes>
            struct _constructor<boost::optional<Ret>, First, std::vector<std::tuple<TupleTypes...>>>
            {
                static boost::optional<Ret> construct(const First & f, const std::vector<std::tuple<TupleTypes...>> & vt)
                {
                    return { _constructor<Ret, First, std::vector<std::tuple<TupleTypes...>>>::construct(f, vt) };
                }
            };

            template<typename Ret, typename First, typename Second, typename... TupleTypes>
            struct _constructor<boost::optional<Ret>, First, Second, std::vector<std::tuple<TupleTypes...>>>
            {
                static boost::optional<Ret> construct(const First & f, const Second & s, const std::vector<std::tuple<TupleTypes...>> & vt)
                {
                    return { _constructor<Ret, First, Second, std::vector<std::tuple<TupleTypes...>>>::construct(f, s, vt) };
                }
            };

            template<typename Ret, typename First, typename Second, typename Third, typename... TupleTypes>
            struct _constructor<boost::optional<Ret>, First, Second, Third, std::vector<std::tuple<TupleTypes...>>>
            {
                static boost::optional<Ret> construct(const First & f, const Second & s, const Third & t,
                    const std::vector<std::tuple<TupleTypes...>> & vt)
                {
                    return { _constructor<Ret, First, Second, Third, std::vector<std::tuple<TupleTypes...>>>::construct(f, s, t, vt) };
                }
            };

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
                    throw std::runtime_error{ "Called match on empty rule." };
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
                    throw std::runtime_error{ "Called match on empty rule." };
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

        template<typename Tref>
        class not_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = void;

            not_parser(const T & np) : _negated{ np }
            {
            }

            bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            bool match(std::vector<lexer::token>::const_iterator begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                return !_negated.match(begin, end, skip);
            }

        private:
            Tref _negated;
        };

        template<typename Tref>
        class and_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = void;

            and_parser(const T & ap) : _and{ ap }
            {
            }

            bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            bool match(std::vector<lexer::token>::const_iterator begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                return _and.match(begin, end, skip);
            }

        private:
            Tref _and;
        };

        template<typename Tref>
        class discard_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = void;

            discard_parser(const T & discarded) : _discarded{ discarded }
            {
            }

            bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            bool match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                return _detail::_is_matched(_discarded.match(begin, end, skip));
            }

        private:
            T _discarded;
        };

        template<typename Tref, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<Tref>
            ::type::value_type, void>::value>::type>
        class optional_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = typename std::conditional<is_optional<typename T::value_type>::value, boost::optional<typename T::value_type>,
                boost::optional<boost::optional<typename T::value_type>>>::type;

            optional_parser(const T & opt) : _optional{ opt }
            {
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                return _optional.match(begin, end, skip);
            }

        private:
            Tref _optional;
        };

        template<typename Tref, typename = typename std::enable_if<is_optional<typename std::remove_reference<Tref>::type
            ::value_type>::value>::type>
        class kleene_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;

            using value_type = std::vector<typename T::value_type::value_type>;

            kleene_parser(const T & other) : _kleene{ other }
            {
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                value_type ret;

                boost::optional<typename value_type::value_type> val;

                while (val = _kleene.match(begin, end, skip))
                {
                    ret.emplace_back(_detail::_constructor<typename value_type::value_type, typename T::value_type::value_type>
                        ::construct(*val));

                    while (skip.match(begin, end)) {}
                }

                return ret;
            }

        private:
            Tref _kleene;
        };

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

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                value_type ret{ typename value_type::value_type{} };

                auto b = begin;
                while (skip.match(b, end)) {}
                auto val = _plus.match(b, end, skip);

                if (!val)
                {
                    return {};
                }

                do
                {
                    ret->emplace_back(_detail::_constructor<typename value_type::value_type::value_type, typename T
                        ::value_type::value_type>::construct(*val));

                    while (skip.match(b, end)) {}
                } while (val = _plus.match(b, end, skip));

                begin = b;

                return ret;
            }

        private:
            Tref _plus;
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

        template<typename Tref, typename Uref, typename = typename std::enable_if<(is_optional<typename
            std::remove_reference<Tref>::type::value_type>::value || _detail::_is_variant_parser<Tref>::value)
            && (is_optional<typename std::remove_reference<Uref>::type::value_type>::value || _detail::_is_variant_parser<Uref>
            ::value)>::type>
        class variant_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = boost::optional<typename remove_optional<typename make_variant_type<typename T::value_type,
                typename U::value_type>::type>::type>;

            variant_parser(const T & first, const U & second) : _first{ first }, _second{ second }
            {
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                auto b = begin;

                while (skip.match(b, end)) {}

                auto f = _first.match(b, end, skip);

                if (f)
                {
                    begin = b;
                    return { _detail::_constructor<value_type, typename T::value_type::value_type>::construct(*f) };
                }

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
            >::type>; // this is not even my final form! ...and probably some bugs on the way

            sequence_parser(const T & first, const U & second) : _first{ first }, _second{ second }
            {
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
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

        template<typename Tref, typename Uref, typename = typename std::enable_if<is_optional<typename
            std::remove_reference<Tref>::type::value_type>::value && is_optional<typename std::remove_reference<Uref>
            ::type::value_type>::value>::type>
        class difference_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = typename T::value_type;

            difference_parser(const T & match, const U & dont) : _match{ match }, _dont{ dont }
            {
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip, typename = typename std::enable_if<std::is_base_of<parser, Skip>::value>::type>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                auto b = begin;
                auto m = _match.match(b, end, skip);

                while (skip.match(b, end)) {}

                auto b2 = b;
                auto d = _dont.match(b, end, skip);

                if (m && !d)
                {
                    begin = b;
                    return { m };
                }

                return {};
            }

        private:
            Tref _match;
            Uref _dont;
        };

        template<typename Tref, typename Uref>
        class seqor_parser : public parser
        {
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

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

        template<typename Tref, typename Uref, typename = typename std::enable_if<is_optional<typename std::remove_reference<Tref>
            ::type::value_type>::value && is_optional<typename std::remove_reference<Uref>::type::value_type>::value>::type>
        class list_parser : public parser
        {
        public:
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = boost::optional<std::vector<typename remove_optional<typename T::value_type>::type>>;

            list_parser(const T & element, const U & separator) : _element{ element }, _separator{ separator }
            {
            }

            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end) const
            {
                return match(begin, end, _detail::_def_skip{});
            }

            template<typename Skip>
            value_type match(std::vector<lexer::token>::const_iterator & begin, std::vector<lexer::token>::const_iterator end,
                Skip skip) const
            {
                while (skip.match(begin, end)) {}

                auto b = begin;
                auto elem = _element.match(b, end, skip);

                if (!elem)
                {
                    return {};
                }

                begin = b;

                value_type ret{ { *elem } };

                while (skip.match(begin, end)) {}
                typename U::value_type sep;

                b = begin;
                while ((sep = _separator.match(b, end, skip)) && (elem = _element.match(b, end, skip)))
                {
                    ret->emplace_back(*elem);
                    begin = b;
                }

                return ret;
            }

        private:
            Tref _element;
            Uref _separator;
        };

        template<typename T, typename U>
        class expect_parser;

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        not_parser<T> operator!(T && parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        not_parser<const T &> operator!(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        and_parser<T> operator&(T && parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        and_parser<const T &> operator&(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        optional_parser<T> operator-(T && parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        optional_parser<const T &> operator-(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        plus_parser<T> operator+(T && parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        plus_parser<const T &> operator+(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        discard_parser<T> operator~(T && parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        discard_parser<const T &> operator~(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        kleene_parser<T> operator*(T && parser)
        {
            return { parser };
        }

        template<typename T, typename = typename std::enable_if<std::is_base_of<parser, T>::value>::type>
        kleene_parser<const T &> operator*(const T & parser)
        {
            return { parser };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        variant_parser<T, U> operator|(T && lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        variant_parser<const T &, const U &> operator|(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        variant_parser<const T &, U> operator|(const T & lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        variant_parser<T, const U &> operator|(T && lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        sequence_parser<T, U> operator>>(T && lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        sequence_parser<const T &, const U &> operator>>(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        sequence_parser<const T &, U> operator>>(const T & lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        sequence_parser<T, const U &> operator>>(T && lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        difference_parser<T, U> operator-(T && lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        difference_parser<const T &, const U &> operator-(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        difference_parser<const T &, U> operator-(const T & lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        difference_parser<T, const U &> operator-(T && lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        seqor_parser<T, U> operator||(T && lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        seqor_parser<const T &, const U &> operator||(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        seqor_parser<const T &, U> operator||(const T & lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        seqor_parser<T, const U &> operator||(T && lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        list_parser<T, U> operator%(T && lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        list_parser<const T &, const U &> operator%(const T & lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        list_parser<const T &, U> operator%(const T & lhs, U && rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename U, typename = typename std::enable_if<std::is_base_of<parser, T>::value &&
            std::is_base_of<parser, U>::value>::type>
        list_parser<T, const U &> operator%(T && lhs, const U & rhs)
        {
            return { lhs, rhs };
        }

        template<typename T, typename Skip>
        typename std::conditional<std::is_same<typename T::value_type, void>::value, bool, typename T::value_type>::type
        parse(const T & parser, std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end,
            Skip skip)
        {
            auto ret = parser.match(begin, end, skip);
            while (skip.match(begin, end)) {}
            return ret;
        }

        template<typename T>
        typename std::conditional<std::is_same<typename T::value_type, void>::value, bool, typename T::value_type>::type
        parse(const T & parser, std::vector<lexer::token>::iterator & begin, std::vector<lexer::token>::iterator end)
        {
            parse(parser, begin, end, _detail::_def_skip{});
        }
    }
}
