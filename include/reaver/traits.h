/**
 * Reaver Library Licence
 *
 * Copyright © 2015-2017 Michał "Griwes" Dominiak
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

#include <optional>
#include <type_traits>
#include <vector>

namespace reaver
{
inline namespace _v1
{
    template<typename T, typename _ = void>
    struct is_container : std::false_type
    {
    };

    template<typename T>
    struct is_container<T,
        std::void_t<typename T::value_type,
            typename T::size_type,
            typename T::allocator_type,
            typename T::iterator,
            typename T::const_iterator,
            decltype(std::declval<T>().size()),
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end()),
            decltype(std::declval<T>().cbegin()),
            decltype(std::declval<T>().cend())>> : public std::true_type
    {
    };

    namespace _detail
    {
        template<typename... Args>
        struct _is_callable_impl : std::false_type
        {
        };

        template<typename T, typename... Args>
        struct _is_callable_impl<std::void_t<decltype(std::declval<T>()(std::declval<Args>()...))>, T, Args...> : std::true_type
        {
        };
    };

    template<typename T, typename... Args>
    struct is_callable : _detail::_is_callable_impl<void, T, Args...>
    {
    };

    template<typename>
    struct is_vector : public std::false_type
    {
    };

    template<typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type
    {
    };

    template<typename>
    struct is_optional : public std::false_type
    {
    };

    template<typename T>
    struct is_optional<std::optional<T>> : std::true_type
    {
    };
}
}
