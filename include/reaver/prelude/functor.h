/**
 * Reaver Library Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

#include <vector>
#include <memory>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "../tmp.h"
#include "../tpl/vector.h"
#include "fold.h"
#include "../logic.h"
#include "../tpl/unique.h"
#include "../tpl/rebind.h"

namespace reaver
{
    inline namespace prelude { inline namespace functor { inline namespace _v1
    {
        template<typename F>
        auto fmap(boost::none_t, F &&)
        {
            return boost::none;
        }

        template<typename T, typename F, typename std::enable_if<is_optional<std::remove_cv_t<std::remove_reference_t<T>>>::value, int>::type = 0>
        auto fmap(T && o, F && f)
        {
            return o ? boost::make_optional(std::invoke(std::forward<F>(f), *o)) : boost::none;
        }

        template<typename T, typename F, typename std::enable_if<is_vector<std::remove_cv_t<std::remove_reference_t<T>>>::value, int>::type = 0>
        auto fmap(T & vec, F && f)
        {
            std::vector<decltype(std::invoke(f, *vec.begin()))> ret;
            ret.reserve(vec.size());
            for (auto && elem : vec) { ret.push_back(std::invoke(f, elem)); }
            return ret;
        }

        template<typename T, typename F>
        auto fmap(std::vector<T> && vec, F && f)
        {
            std::vector<decltype(std::invoke(f, *vec.begin()))> ret;
            ret.reserve(vec.size());
            for (auto elem : vec) { ret.push_back(std::invoke(f, std::move(elem))); }
            return ret;
        }

        template<typename T, typename F>
        auto fmap(const std::unique_ptr<T> & ptr, F && f)
        {
            return ptr ? std::make_unique<decltype(std::invoke(f, *ptr))>(std::invoke(f, *ptr)) : nullptr;
        }

        template<typename T, typename F>
        auto fmap(std::unique_ptr<T> && ptr, F && f)
        {
            return ptr ? std::make_unique<decltype(std::invoke(f, std::move(*ptr)))>(std::invoke(f, std::move(*ptr))) : nullptr;
        }

        template<typename T, typename F>
        auto fmap(const std::shared_ptr<T> & ptr, F && f)
        {
            return ptr ? std::make_unique<decltype(std::invoke(f, *ptr))>(std::invoke(f, *ptr)) : nullptr;
        }

        namespace _detail
        {
            template<typename T, typename F>
            struct _visitor : boost::static_visitor<T>
            {
                _visitor(F && f) : _f(std::forward<F>(f))
                {
                }

                template<typename U>
                T operator()(U && u)
                {
                    return T{ std::invoke(std::forward<F>(_f), std::forward<U>(u)) };
                }

                F && _f;
            };
        }

        template<typename... Ts, typename F>
        auto fmap(const boost::variant<Ts...> & variant, F && f)
        {
            _detail::_visitor<tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), std::declval<Ts>()))...>, boost::variant>, F> visitor(std::forward<F>(f));
            return boost::apply_visitor(visitor, variant);
        }
    }}}
}
