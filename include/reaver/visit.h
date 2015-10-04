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
#include "overloads.h"
#include "logic.h"
#include "swallow.h"

namespace reaver { inline namespace _v1
{
    template<typename Lambda, typename... Variants>
    decltype(auto) visit(Lambda && lambda, boost::variant<Variants...> & variant)
    {
        return boost::apply_visitor(lambda, variant);
    }

    template<typename Lambda, typename... Variants>
    decltype(auto) visit(Lambda && lambda, const boost::variant<Variants...> & variant)
    {
        return boost::apply_visitor(lambda, variant);
    }

    namespace _detail
    {
        template<typename T>
        struct _remove_recursive_wrapper
        {
            using type = T;
        };

        template<typename T>
        struct _remove_recursive_wrapper<boost::recursive_wrapper<T>>
        {
            using type = T;
        };

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

        template<typename T, typename U>
        struct _is_in_variant : std::false_type {};

        template<typename... Args, typename U>
        struct _is_in_variant<boost::variant<Args...>, U> : any_of<std::is_same<Args, U>::value...> {};

        template<typename T, typename Lambda, typename Default>
        struct _dispatching_visitor_impl
        {
        public:
            _dispatching_visitor_impl(Lambda l, Default d) : _l{ std::move(l) }, _d{ std::move(d) }
            {
            }

            template<typename U>
            decltype(auto) operator()(U && u)
            {
                return _call<U>(select_overload{})(std::forward<U>(u));
            }

        private:
            template<typename U, typename std::enable_if<std::is_same<typename _match_type_specification<T, U>::type, T>::value
                || _is_in_variant<T, U>::value, int>::type = 0>
            auto _call(choice<0>)
            {
                return [&](U && u){ return _l(std::forward<U>(u)); };
            }

            template<typename U>
            auto _call(choice<1>)
            {
                return [&](U && u){ return _d(std::forward<U>(u)); };
            }

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
        return [](auto &&... args)
        {
            static_assert((swallow{args...}, false), "invalid call to dispatching visitor");
            return unit{};
        };
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
