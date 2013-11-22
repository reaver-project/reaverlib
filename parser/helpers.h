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
#define BOOST_NO_CXX11_RVALUE_REFERENCES
#include <boost/variant.hpp>
#undef BOOST_NO_CXX11_RVALUE_REFERENCES

#include "../tmp.h"
#include "../callbacks.h"
#include "../lexer/lexer.h"

namespace reaver
{
    namespace parser
    {
        class parser
        {
        };

        namespace _detail
        {
            template<typename ValueType>
            class _parser_callbacks
            {
            public:
                _parser_callbacks & operator()(std::function<void (std::size_t, std::size_t)> f)
                {
                    _callbacks_positions += std::move(f);
                    return *this;
                }

                _parser_callbacks & operator()(std::function<void (std::vector<lexer::token>::const_iterator, std::vector<
                    lexer::token>::const_iterator)> f)
                {
                    _callbacks_iterators += std::move(f);
                    return *this;
                }

                _parser_callbacks & operator()(std::function<void (std::size_t, std::size_t, const ValueType &)> f)
                {
                    _callbacks_positions_value += std::move(f);
                    return *this;
                }

                _parser_callbacks & operator()(std::function<void (std::vector<lexer::token>::const_iterator, std::vector<
                    lexer::token>::const_iterator, const ValueType &)> f)
                {
                    _callbacks_iterators_value += std::move(f);
                    return *this;
                }

                void operator()(std::size_t begin, std::size_t end) const
                {
                    _callbacks_positions(begin, end);
                }

                void operator()(std::vector<lexer::token>::const_iterator begin, std::vector<lexer::token>::const_iterator
                    end) const
                {
                    _callbacks_iterators(begin, end);
                }

                void operator()(std::size_t begin, std::size_t end, const ValueType & val) const
                {
                    _callbacks_positions_value(begin, end, val);
                }

                void operator()(std::vector<lexer::token>::const_iterator begin, std::vector<lexer::token>::const_iterator
                    end, const ValueType & val) const
                {
                    _callbacks_iterators_value(begin, end, val);
                }

            private:
                callbacks<void (std::size_t, std::size_t)> _callbacks_positions;
                callbacks<void (std::vector<lexer::token>::const_iterator, std::vector<lexer::token>::const_iterator)>
                    _callbacks_iterators;
                callbacks<void (std::size_t, std::size_t, const ValueType &)> _callbacks_positions_value;
                callbacks<void (std::vector<lexer::token>::const_iterator, std::vector<lexer::token>::const_iterator,
                    const ValueType &)> _callbacks_iterators_value;
            };

            template<>
            class _parser_callbacks<bool>
            {
            public:
                _parser_callbacks & operator()(std::function<void (std::size_t, std::size_t)> f)
                {
                    _callbacks_positions += std::move(f);
                    return *this;
                }

                _parser_callbacks & operator()(std::function<void (std::vector<lexer::token>::const_iterator, std::vector<
                    lexer::token>::const_iterator)> f)
                {
                    _callbacks_iterators += std::move(f);
                    return *this;
                }

                void operator()(std::size_t begin, std::size_t end) const
                {
                    _callbacks_positions(begin, end);
                }

                void operator()(std::vector<lexer::token>::const_iterator begin, std::vector<lexer::token>::const_iterator
                    end) const
                {
                    _callbacks_iterators(begin, end);
                }

            private:
                callbacks<void (std::size_t, std::size_t)> _callbacks_positions;
                callbacks<void (std::vector<lexer::token>::const_iterator, std::vector<lexer::token>::const_iterator)>
                    _callbacks_iterators;
            };

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

            template<typename... Tuple1Types, typename... Tuple2Types>
            struct _constructor<std::tuple<Tuple1Types..., Tuple2Types...>, std::tuple<Tuple1Types...>, std::tuple<Tuple2Types...>>
            {
                static std::tuple<Tuple1Types..., Tuple2Types...> construct(const std::tuple<Tuple1Types...> & t1,
                    const std::tuple<Tuple2Types...> & t2)
                {
                    return std::tuple_cat(t1, t2);
                }
            };

            template<typename... Tuple1Types, typename... Tuple2Types>
            struct _constructor<boost::optional<std::tuple<Tuple1Types..., Tuple2Types...>>, std::tuple<Tuple1Types...>,
                std::tuple<Tuple2Types...>>
            {
                static const std::tuple<Tuple1Types..., Tuple2Types...> construct(const std::tuple<Tuple1Types...> & t1,
                    const std::tuple<Tuple2Types...> & t2)
                {
                    return { std::tuple_cat(t1, t2) };
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
                    return _unpacker<typename generator<sizeof...(TupleTypes)>::type, Ret, First, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, t, args...);
                }
            };

            template<typename Ret, typename First, typename... TupleTypes, typename... Args>
            struct _constructor<boost::optional<Ret>, First, std::tuple<TupleTypes...>, Args...>
            {
                static boost::optional<Ret> construct(const First & f, const std::tuple<TupleTypes...> & t, const Args &... args)
                {
                    return { _unpacker<typename generator<sizeof...(TupleTypes)>::type, Ret, First, std::tuple<TupleTypes...>,
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
                    return _unpacker<typename generator<sizeof...(TupleTypes)>::type, Ret, First, Second, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, s, t, args...);
                }
            };

            template<typename Ret, typename First, typename Second, typename... TupleTypes, typename... Args>
            struct _constructor<boost::optional<Ret>, First, Second, std::tuple<TupleTypes...>, Args...>
            {
                static boost::optional<Ret> construct(const First & f, const Second & s, const std::tuple<TupleTypes...> & t,
                    const Args &... args)
                {
                    return { _unpacker<typename generator<sizeof...(TupleTypes)>::type, Ret, First, Second, std::tuple<TupleTypes...>,
                        Args...>::unpack(f, s, t, args...) };
                }
            };

            template<int... I, typename... Rets, typename... Ts>
            struct _unpacker<sequence<I...>, boost::variant<Rets...>, std::tuple<Ts...>>
            {
                static boost::variant<Rets...> unpack(const std::tuple<Ts...> & t)
                {
                    return { t };
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

            template<typename Ret>
            struct _constructor<boost::optional<Ret>, boost::optional<Ret>>
            {
                static const boost::optional<Ret> & construct(const boost::optional<Ret> & r)
                {
                    return r;
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

            template<typename... Types>
            struct _true_type<std::tuple<Types...>>
            {
                using type = std::tuple<Types...>;
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

            inline bool _pass_true_type(bool b)
            {
                return b;
            }

            template<typename... Types>
            const std::tuple<Types...> & _pass_true_type(const std::tuple<Types...> & t)
            {
                return t;
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

            inline bool _is_matched(bool b)
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

            template<typename CharType>
            struct _constructor<std::basic_string<CharType>, std::vector<std::basic_string<CharType>>>
            {
                static const std::basic_string<CharType> construct(const std::vector<std::basic_string<CharType>> & vec)
                {
                    std::string ret;

                    for (auto & x : vec)
                    {
                        ret.append(x);
                    }

                    return ret;
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

            // TODO: similar for different combinations
            template<typename Ret, typename... Tuple1Types, typename... Tuple2Types>
            struct _constructor<boost::optional<Ret>, std::tuple<Tuple1Types...>, std::vector<std::tuple<Tuple2Types...>>>
            {
                static boost::optional<Ret> construct(const std::tuple<Tuple1Types...> & f, const std::vector<std::tuple
                    <Tuple2Types...>> & vt)
                {
                    return { _unpacker<typename generator<sizeof...(Tuple1Types)>::type, Ret, std::tuple<Tuple1Types...>,
                        std::vector<std::tuple<Tuple2Types...>>>::unpack(f, vt) };
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
        }
    }
}
