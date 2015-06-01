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

namespace reaver
{
    inline namespace prelude { inline namespace functor { inline namespace _v1
    {
        template<typename F>
        auto fmap(boost::none_t, F &&)
        {
            return boost::none;
        }

        template<typename T, typename F>
        auto fmap(const boost::optional<T> & o, F && f)
        {
            return o ? boost::make_optional(std::forward<F>(f)(*o)) : boost::none;
        }

        template<typename T, typename F>
        auto fmap(boost::optional<T> && o, F && f)
        {
            return o ? boost::make_optional(std::forward<F>(f)(std::move(*o))) : boost::none;
        }

        template<typename T, typename F>
        auto fmap(const std::vector<T> & vec, F && f)
        {
            std::vector<decltype(std::forward<F>(f)(std::declval<const T &>()))> ret;
            ret.reserve(vec.size());
            for (auto && elem : vec) { ret.push_back(std::forward<F>(f)(elem)); }
            return ret;
        }

        template<typename T, typename F>
        auto fmap(std::vector<T> && vec, F && f)
        {
            std::vector<decltype(std::forward<F>(f)(std::declval<T>()))> ret;
            ret.reserve(vec.size());
            for (auto elem : vec) { ret.push_back(std::forward<F>(f)(std::move(elem))); }
            return ret;
        }

        template<typename T, typename F>
        auto fmap(const std::unique_ptr<T> & ptr, F && f)
        {
            return ptr ? std::make_unique<decltype(std::forward<F>(f)(*ptr))>(std::forward<F>(f)(*ptr)) : nullptr;
        }

        template<typename T, typename F>
        auto fmap(std::unique_ptr<T> && ptr, F && f)
        {
            return ptr ? std::make_unique<decltype(std::forward<F>(f)(std::move(*ptr)))>(std::forward<F>(f)(std::move(*ptr))) : nullptr;
        }

        template<typename T, typename F>
        auto fmap(const std::shared_ptr<T> & ptr, F && f)
        {
            return ptr ? std::make_unique<decltype(std::forward<F>(f)(*ptr))>(std::forward<F>(f)(*ptr)) : nullptr;
        }
    }}}
}

