/**
 * Reaver Library Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <map>
#include <type_traits>

#include <boost/type_index.hpp>
#include <boost/any.hpp>

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<typename T>
        struct _has_identity_construct
        {
        private:
            template<typename U>
            static auto _test(int) -> decltype(U::construct(std::declval<typename U::type>()), void());

            template<typename U>
            static char _test(long);

        public:
            static constexpr bool value = std::is_void<decltype(_test<T>(0))>::value;
        };

        template<typename... Args>
        struct _has_construct
        {
        private:
            template<typename T, typename U>
            static auto _test(int) -> decltype(T::construct(std::declval<U>()), void());

            template<typename T, typename...>
            static char _test(long);

        public:
            static constexpr bool value = std::is_void<decltype(_test<Args...>(0))>::value;
        };

        template<typename... Args>
        struct _has_static_cast
        {
        private:
            template<typename T, typename Arg>
            static auto _test(int) -> decltype(static_cast<typename T::type>(std::declval<Arg>()), void());

            template<typename...>
            static char _test(long);

        public:
            static constexpr bool value = std::is_void<decltype(_test<Args...>(0))>::value;
        };

        template<typename... Args>
        struct _has_exact_match
        {
        private:
            template<typename T, typename... Ts>
            static auto _test(int) -> decltype(static_cast<decltype(T::construct(std::declval<Ts>()...)) (*)(Args...)>(&T::construct), void());

            template<typename...>
            static char _test(long);

        public:
            static constexpr bool value = std::is_void<decltype(_test<typename std::remove_reference<Args>::type...>(0))>::value;
        };

        template<typename... Args>
        struct _is_callable
        {
        private:
            template<typename T, typename... Ts>
            static auto _test(int) -> decltype(T::construct(std::declval<Ts>()...), void());

            template<typename...>
            static char _test(long);

        public:
            static constexpr bool value = std::is_void<decltype(_test<Args...>(0))>::value;
        };

        template<typename... Args>
        struct _is_constructible
        {
        private:
            template<typename T, typename... Ts>
            static auto _test(int) -> decltype(typename T::type{ std::declval<Ts>()... }, void());

            template<typename...>
            static char _test(long);

        public:
            static constexpr bool value = std::is_void<decltype(_test<Args...>(0))>::value;
        };

        template<typename... Args>
        struct _is_same : std::false_type
        {
        };

        template<typename T, typename U>
        struct _is_same<T, U> : public std::is_same<typename std::remove_reference<typename std::remove_cv<T>::type>::type, typename std::remove_reference<typename std::remove_cv<U>::type>::type>
        {
        };

        template<template<typename...> class Trait, typename T, typename TypeList>
        struct _apply_on_type_list;

        template<template<typename...> class Trait, typename T, typename... Types>
        struct _apply_on_type_list<Trait, T, std::tuple<Types...>> : Trait<T, Types...> {};

        template<unsigned I>
        struct _choice : _choice<I + 1> {};

        template<>
        struct _choice<10> {};

        struct _select_overload : _choice<0> {};
    }

    class configuration
    {
    public:
        template<typename T, typename... Args>
        void set(Args &&... args)
        {
            _set<T, std::tuple<Args...>>(_detail::_select_overload{})(std::forward<Args>(args)...);
        }

        template<typename T>
        typename T::type & get()
        {
            return boost::any_cast<typename T::type &>(_map.at(boost::typeindex::type_id<T>()));
        }

        template<typename T>
        const typename T::type & get() const
        {
            return boost::any_cast<const typename T::type &>(_map.at(boost::typeindex::type_id<T>()));
        }

    private:
        // necessary forms:
        // 0 - _map[boost::typeindex::type_id<T>()] = T::construct(std::move(value)), when identity construct exists
        // 1 - _map[boost::typeindex::type_id<T>()] = std::move(value), when the target type is passed and no identity construct exists
        // 2 - _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<Arg>(arg)), with perfect match of argument type (modulo cref-qualifiers)
        // 4 - _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(std::forward<Arg>(arg)), if no perfectly matching construct
        // 5 - _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<Args>(args)...), if matching construct exists
        // 6 - _map[boost::typeindex::type_id<T>()] = typename T::type{ std::forward<Args>(args)... }, if possible

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_same, typename T::type, TypeList>::value
            && _detail::_has_identity_construct<T>::value, int>::type = 0>
        auto _set(_detail::_choice<0>)
        {
            return [&](typename T::type value){ _map[boost::typeindex::type_id<T>()] = T::construct(std::move(value)); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_same, typename T::type, TypeList>::value, int>::type = 0>
        auto _set(_detail::_choice<1>)
        {
            return [&](typename T::type value){ _map[boost::typeindex::type_id<T>()] = std::move(value); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_has_exact_match, T, TypeList>::value, int>::type = 0>
        auto _set(_detail::_choice<2>)
        {
            return [&](auto && arg){ _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<decltype(arg)>(arg)); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_has_static_cast, T, TypeList>::value, int>::type = 0>
        auto _set(_detail::_choice<3>)
        {
            return [&](auto && arg){ _map[boost::typeindex::type_id<T>()] = static_cast<typename T::type>(std::forward<decltype(arg)>(arg)); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_callable, T, TypeList>::value, int>::type = 0>
        auto _set(_detail::_choice<4>)
        {
            return [&](auto &&... args){ _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<decltype(args)>(args)...); };
        }

        template<typename T, typename TypeList, typename std::enable_if<_detail::_apply_on_type_list<_detail::_is_constructible, T, TypeList>::value, int>::type = 0>
        auto _set(_detail::_choice<5>)
        {
            return [&](auto &&... args){ _map[boost::typeindex::type_id<T>()] = typename T::type{ std::forward<decltype(args)>(args)... }; };
        }

        std::map<boost::typeindex::type_index, boost::any> _map;
    };
}}
