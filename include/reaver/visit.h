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

#include <boost/variant.hpp>

#include "tmp.h"
#include "id.h"
#include "unit.h"

namespace reaver { inline namespace _v1
{
    namespace _detail
    {
        template<typename Void, typename...>
        struct _filtered_common_type;

        template<typename Last>
        struct _filtered_common_type<void, Last>
        {
            using type = Last;
        };

        template<typename Last, typename... Voids>
        struct _filtered_common_type<typename std::enable_if<all<std::is_same<Voids, boost::detail::variant::void_>::value...>::value>::type, Last, Voids...>
        {
            using type = Last;
        };

        template<typename First, typename... Rest>
        struct _filtered_common_type<typename std::enable_if<!all<std::is_same<Rest, boost::detail::variant::void_>::value...>::value>::type, First, Rest...>
            : std::common_type<First, typename _filtered_common_type<void, Rest...>::type>
        {
        };

        template<typename... Rest>
        struct _filtered_common_type<void, boost::detail::variant::void_, Rest...> : _filtered_common_type<void, Rest...>
        {
        };

        template<typename Lambda>
        struct _call_forwarder
        {
            template<typename Arg, typename std::enable_if<!std::is_same<boost::detail::variant::void_, Arg>::value, int>::type = 0>
            auto operator()(Arg && arg) -> decltype(std::declval<Lambda>()(std::forward<Arg>(arg)));
            boost::detail::variant::void_ operator()(const boost::detail::variant::void_ & v);
        };

        template<typename U>
        struct _remove_recursive_wrapper
        {
            using type = U;
        };

        template<typename U>
        struct _remove_recursive_wrapper<boost::recursive_wrapper<U>>
        {
            using type = U;
        };
    }

    template<typename Lambda, typename... Variants>
    struct visitor
    {
        Lambda lambda;

        using result_type = typename _detail::_filtered_common_type<void, decltype(std::declval<_detail::_call_forwarder<Lambda>>()(std::declval<typename _detail::_remove_recursive_wrapper<Variants>::type>()))...>::type;

        template<typename Arg>
        decltype(auto) operator()(Arg && arg)
        {
            return lambda(std::forward<Arg>(arg));
        }
    };

    template<typename Lambda, typename... Variants>
    decltype(auto) visit(Lambda && lambda, const boost::variant<Variants...> & variant)
    {
        auto v = visitor<Lambda, Variants...>{ std::forward<Lambda>(lambda) };
        return boost::apply_visitor(v, variant);
    }

    namespace _detail
    {
        template<typename U, bool IsNotReference, bool IsNotCv>
        struct _match_type_specification_impl
        {
            using type = U;
        };

        template<typename U>
        struct _match_type_specification_impl<U, false, true>
        {
            using type = std::remove_cv_t<U>;
        };

        template<typename U>
        struct _match_type_specification_impl<U, true, false>
        {
            using type = std::remove_reference_t<U>;
        };

        template<typename U>
        struct _match_type_specification_impl<U, true, true>
        {
            using type = std::remove_cv_t<std::remove_reference_t<U>>;
        };

        template<typename T, typename U>
        struct _match_type_specification
        {
            using type = typename _match_type_specification_impl<
                typename _remove_recursive_wrapper<U>::type,
                std::is_same<T, std::remove_reference_t<T>>::value,
                std::is_same<T, std::remove_cv_t<T>>::value
            >::type;
        };

        template<typename T, typename Lambda, typename Default>
        struct _dispatching_visitor_impl
        {
        public:
            _dispatching_visitor_impl(Lambda l, Default d) : _l{ std::move(l) }, _d{ std::move(d) }
            {
            }

            template<typename U, typename std::enable_if<std::is_same<typename _match_type_specification<T, U>::type, T>::value, int>::type = 0>
            decltype(auto) operator()(U && u)
            {
                return _l(std::forward<U>(u));
            }

            template<typename U, typename std::enable_if<!std::is_same<typename _match_type_specification<T, U>::type, T>::value, int>::type = 0>
            decltype(auto) operator()(U && u)
            {
                return _d(std::forward<U>(u));
            }

        private:
            Lambda _l;
            Default _d;
        };

        template<typename T, typename Lambda, typename Default>
        auto _make_dispatching_visitor_impl(Lambda && lambda, Default && def)
        {
            return _dispatching_visitor_impl<T, Lambda, Default>(std::forward<Lambda>(lambda), std::forward<Default>(def));
        }
    }

    struct default_id {};

    inline auto make_visitor()
    {
        return [](){ return unit{}; };
    }

    template<typename Lambda>
    auto make_visitor(default_id, Lambda && lambda)
    {
        return std::forward<Lambda>(lambda);
    }

    template<typename T, typename Lambda, typename... Rest>
    auto make_visitor(id<T>, Lambda && lambda, Rest &&... rest)
    {
        return _detail::_make_dispatching_visitor_impl<T>(std::forward<Lambda>(lambda), make_visitor(std::forward<Rest>(rest)...));
    }
}}
