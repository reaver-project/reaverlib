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
        template<typename...>
        struct _is_same_with_pack : std::false_type {};

        template<typename T>
        struct _is_same_with_pack<T, T> : std::true_type {};

        template<typename T>
        auto _identity_existence_test(int) -> decltype(T::construct(typename T::type{}), void());

        template<typename T>
        char _identity_existence_test(long);

        template<typename T>
        using _has_identity_construct = std::is_void<decltype(_identity_existence_test<T>(0))>;

        template<typename T, typename U>
        auto _existence_test(int) -> decltype(T::construct(U{}), void());

        template<typename T, typename... Args>
        char _existence_test(long);

        template<typename... Args>
        using _has_construct = std::is_void<decltype(_existence_test<Args...>(0))>;
    }

    class configuration
    {
    public:
        template<typename T>
        auto set(typename T::type value) -> typename std::enable_if<!_detail::_has_identity_construct<T>::value>::type
        {
            _map[boost::typeindex::type_id<T>()] = std::move(value);
        }

        template<typename T, typename... Args>
        auto set(Args &&... args) -> typename std::enable_if<std::is_same<decltype(typename T::type{ std::forward<Args>(args)... }), typename T::type>::value
            && !_detail::_is_same_with_pack<T, Args...>::value && !_detail::_has_construct<T, Args...>::value>::type
        {
            _map[boost::typeindex::type_id<T>()] = typename T::type{ std::forward<Args>(args)... };
        }

        template<typename T, typename... Args>
        auto set(Args &&... args) -> typename std::enable_if<std::is_same<decltype(T::construct(std::forward<Args>(args)...)), typename T::type>::value>::type
        {
            _map[boost::typeindex::type_id<T>()] = T::construct(std::forward<Args>(args)...);
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
        std::map<boost::typeindex::type_index, boost::any> _map;
    };
}}
