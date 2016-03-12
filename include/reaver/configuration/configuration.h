/**
 * Reaver Library Licence
 *
 * Copyright © 2014-2015 Michał "Griwes" Dominiak
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
 **/

#pragma once

#include <unordered_map>
#include <type_traits>

#include <boost/type_index.hpp>
#include <boost/functional/hash.hpp>
#include <boost/any.hpp>

#include "../unit.h"
#include "../swallow.h"
#include "../overloads.h"
#include "../logic.h"
#include "../void_t.h"

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<typename Tag, typename = void>
        struct _has_identity_construct : public std::false_type {};

        template<typename T>
        struct _has_identity_construct<T, void_t<decltype(T::construct(std::declval<typename T::type>()))>> : public std::true_type {};

        template<typename Tag, typename From, typename = void>
        struct _has_construct : public std::false_type {};

        template<typename T, typename U>
        struct _has_construct<T, U, void_t<decltype(T::construct(std::declval<U>()))>> : public std::true_type {};

        template<typename... Args>
        struct _has_static_cast_impl : public std::false_type {};

        template<typename T, typename U>
        struct _has_static_cast_impl<void_t<decltype(static_cast<typename T::type>(std::declval<U>()))>, T, U> : public std::true_type {};

        template<typename Tag, typename... From>
        struct _has_static_cast : _has_static_cast_impl<void, Tag, From...> {};

        template<typename... Args>
        struct _has_exact_match_impl : public std::false_type {};

        template<typename T, typename... Args>
        struct _has_exact_match_impl<void_t<decltype(static_cast<decltype(T::construct(std::declval<Args>()...)) (*)(Args...)>(&T::construct))>, T, Args...> : public std::true_type {};

        template<typename Tag, typename... From>
        struct _has_exact_match : _has_exact_match_impl<void, Tag, std::remove_reference_t<From>...> {};

        template<typename... Args>
        struct _is_construct_callable_impl : public std::false_type {};

        template<typename T, typename... Args>
        struct _is_construct_callable_impl<void_t<decltype(T::construct(std::declval<Args>()...))>, T, Args...> : public std::true_type {};

        template<typename Tag, typename... Args>
        struct _is_construct_callable : public _is_construct_callable_impl<void, Tag, Args...> {};

        template<typename... Args>
        struct _is_constructible_impl : public std::false_type {};

        template<typename T, typename... Args>
        struct _is_constructible_impl<void_t<decltype(typename T::type{ std::declval<Args>()... })>, T, Args...> : public std::true_type {};

        template<typename Tag, typename... Args>
        struct _is_constructible : public _is_constructible_impl<void, Tag, Args...> {};

        template<typename... Args>
        struct _is_same : std::false_type {};

        template<typename T, typename U>
        struct _is_same<T, U> : public std::is_same<
            typename std::remove_reference<typename std::remove_cv<T>::type>::type,
            typename std::remove_reference<typename std::remove_cv<U>::type>::type
        > {};

        template<template<typename...> class Trait, typename T, typename TypeList>
        struct _apply_on_type_list;

        template<template<typename...> class Trait, typename T, typename... Types>
        struct _apply_on_type_list<Trait, T, std::tuple<Types...>> : Trait<T, Types...> {};
    }

    class configuration
    {
    public:
        template<typename T, typename... Args>
        unit set(Args &&... args)
        {
            _set<T, std::tuple<Args...>>(select_overload{})(std::forward<Args>(args)...);
            return {};
        }

        template<typename T, typename... Args>
        unit set(T, Args &&... args)
        {
            set<T>(std::forward<Args>(args)...);
            return {};
        }

        template<typename T>
        auto & get(T = {})
        {
            return _get<T>(select_overload{})();
        }

        template<typename T>
        auto & get(T = {}) const
        {
            return boost::any_cast<const typename T::type &>(_map.at(boost::typeindex::type_id<T>()));
        }

    private:
        template<typename T, typename std::enable_if<std::is_void<decltype(T::default_value, void())>::value, int>::type = 0>
        auto _get(choice<0>)
        {
            return [&]() -> decltype(auto)
            {
                if (_map.find(boost::typeindex::type_id<T>()) == _map.end())
                {
                    set<T>(typename T::type{ T::default_value });
                }
                return boost::any_cast<typename T::type &>(_map.at(boost::typeindex::type_id<T>()));
            };
        }

        template<typename T>
        auto _get(choice<1>)
        {
            return [&]() -> decltype(auto) { return boost::any_cast<typename T::type &>(_map.at(boost::typeindex::type_id<T>())); };
        }

        // necessary forms:
        // 0 - _map[boost::typeindex::type_id<T>()] = T::construct(std::move(value)), when identity construct exists
        // 1 - _map[boost::typeindex::type_id<T>()] = std::move(value), when the target type is passed and no identity construct exists
        // 2 - _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<Arg>(arg)), with perfect match of argument type (modulo cref-qualifiers)
        // 4 - _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(std::forward<Arg>(arg)), if no perfectly matching construct
        // 5 - _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<Args>(args)...), if matching construct exists
        // 6 - _map[boost::typeindex::type_id<T>()] = typename T::type{ std::forward<Args>(args)... }, if possible

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_same, typename T::type, TypeList>::value
            && _detail::_has_identity_construct<T>::value, int>::type = 0>
        auto _set(choice<0>)
        {
            return [&](typename T::type value){ _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(T::construct(std::move(value))); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_same, typename T::type, TypeList>::value, int>::type = 0>
        auto _set(choice<1>)
        {
            return [&](typename T::type value){ _map[boost::typeindex::type_id<T>()] = std::move(value); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_has_exact_match, T, TypeList>::value, int>::type = 0>
        auto _set(choice<2>)
        {
            return [&](auto &&... arg){ _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(T::construct(std::forward<decltype(arg)>(arg)...)); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_has_static_cast, T, TypeList>::value, int>::type = 0>
        auto _set(choice<3>)
        {
            return [&](auto && arg){ _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(std::forward<decltype(arg)>(arg)); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_construct_callable, T, TypeList>::value, int>::type = 0>
        auto _set(choice<4>)
        {
            return [&](auto &&... args){ _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(T::construct(std::forward<decltype(args)>(args)...)); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_constructible, T, TypeList>::value, int>::type = 0>
        auto _set(choice<5>)
        {
            return [&](auto &&... args){ _map[boost::typeindex::type_id<T>()] = typename T::type{ std::forward<decltype(args)>(args)... }; };
        }

        std::unordered_map<boost::typeindex::type_index, boost::any, boost::hash<boost::typeindex::type_index>> _map;
    };

    namespace _detail
    {
        template<typename Checked, typename Set>
        struct _is_a_subset : public std::false_type
        {
        };

        template<typename... Set>
        struct _is_a_subset<std::tuple<>, std::tuple<Set...>> : public std::true_type
        {
        };

        template<typename... Checked>
        struct _is_a_subset<std::tuple<Checked...>, std::tuple<>> : public std::false_type
        {
        };

        template<typename T>
        struct _is_a_subset<std::tuple<T>, std::tuple<T>> : public std::true_type
        {
        };

        template<typename T, typename... Others>
        struct _is_a_subset<std::tuple<T>, std::tuple<T, Others...>> : public std::true_type
        {
        };

        template<typename T, typename Head, typename... Tail>
        struct _is_a_subset<std::tuple<T>, std::tuple<Head, Tail...>> : public _is_a_subset<std::tuple<T>, std::tuple<Tail...>>
        {
        };

        template<typename... Checked, typename... Set>
        struct _is_a_subset<std::tuple<Checked...>, std::tuple<Set...>> : public all_of<_is_a_subset<std::tuple<Checked>, std::tuple<Set...>>::value...>
        {
        };
    }

    template<typename... Allowed>
    class bound_configuration : public configuration
    {
    private:
        template<typename... Other, typename std::enable_if<_detail::_is_a_subset<std::tuple<Other...>, std::tuple<Allowed...>>::value, int>::type = 0>
        bound_configuration(const bound_configuration<Other...> & other) : configuration{ other }
        {
        }

        template<typename... Other, typename std::enable_if<_detail::_is_a_subset<std::tuple<Other...>, std::tuple<Allowed...>>::value, int>::type = 0>
        bound_configuration(bound_configuration<Other...> && other) : configuration{ std::move(other) }
        {
        }

    public:
        template<typename... Other, typename std::enable_if<
            !_detail::_is_a_subset<std::tuple<Allowed...>, std::tuple<Other...>>::value &&
            !_detail::_is_a_subset<std::tuple<Other...>, std::tuple<Allowed...>>::value,
        int>::type = 0>
        bound_configuration(const bound_configuration<Other...> & other) = delete;

        template<typename... Other, typename std::enable_if<
            !_detail::_is_a_subset<std::tuple<Allowed...>, std::tuple<Other...>>::value &&
            !_detail::_is_a_subset<std::tuple<Other...>, std::tuple<Allowed...>>::value,
        int>::type = 0>
        bound_configuration(bound_configuration<Other...> && other) = delete;

        template<typename... Other, typename std::enable_if<_detail::_is_a_subset<std::tuple<Allowed...>, std::tuple<Other...>>::value, int>::type = 0>
        bound_configuration(const bound_configuration<Other...> & other)
        {
            swallow{ set<Allowed>(other.template get<Allowed>())... };
        }

        template<typename... Other, typename std::enable_if<_detail::_is_a_subset<std::tuple<Allowed...>, std::tuple<Other...>>::value, int>::type = 0>
        bound_configuration(bound_configuration<Other...> && other)
        {
            swallow{ set<Allowed>(std::move(other.template get<Allowed>()))... };
        }

        bound_configuration(const configuration & config)
        {
            swallow{ set<Allowed>(config.get<Allowed>())... };
        }

        bound_configuration(configuration && config)
        {
            swallow{ set<Allowed>(std::move(config.get<Allowed>()))... };
        }

        template<typename T, typename... Args, typename std::enable_if<any_of<std::is_same<T, Allowed>::value...>::value, int>::type = 0>
        unit set(Args &&... args)
        {
            return configuration::set<T>(std::forward<Args>(args)...);
        }

        template<typename T, typename... Args, typename std::enable_if<any_of<std::is_same<T, Allowed>::value...>::value, int>::type = 0>
        unit set(T, Args &&... args)
        {
            return configuration::set(T{}, std::forward<Args>(args)...);
        }

        template<typename T, typename std::enable_if<any_of<std::is_same<T, Allowed>::value...>::value, int>::type = 0>
        auto & get(T = {})
        {
            return configuration::get<T>();
        }

        template<typename T, typename std::enable_if<any_of<std::is_same<T, Allowed>::value...>::value, int>::type = 0>
        auto & get(T = {}) const
        {
            return configuration::get<T>();
        }

        template<typename T, typename... Args, typename std::enable_if<!any_of<std::is_same<T, Allowed>::value...>::value, int>::type = 0>
        auto add(Args &&... args) const &
        {
            bound_configuration<Allowed..., T> ret = *this;
            ret.template set<T>(std::forward<Args>(args)...);
            return ret;
        }

        template<typename T, typename... Args, typename std::enable_if<!any_of<std::is_same<T, Allowed>::value...>::value, int>::type = 0>
        auto add(Args &&... args) &&
        {
            bound_configuration<Allowed..., T> ret = std::move(*this);
            ret.template set<T>(std::forward<Args>(args)...);
            return ret;
        }

        template<typename...>
        friend class bound_configuration;
    };

    template<>
    class bound_configuration<> : public configuration
    {
    public:
        bound_configuration() = default;
        bound_configuration(const bound_configuration &) = default;
        bound_configuration(bound_configuration &&) = default;

        bound_configuration(const configuration &)
        {
        }

        bound_configuration(configuration &&)
        {
        }

        template<typename T, typename... Args>
        auto add(Args &&... args) const
        {
            bound_configuration<T> ret = *this;
            ret.template set<T>(std::forward<Args>(args)...);
            return ret;
        }

    private:
        template<typename... Args>
        unit set(Args &&...);
        template<typename... Args>
        void get(Args &&...) const;
    };
}}

