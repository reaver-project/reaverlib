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

#include <functional>
#include <memory>
#include <vector>

#include "../logic.h"
#include "../tpl/rebind.h"
#include "../tpl/unique.h"
#include "../tpl/vector.h"
#include "../traits.h"

namespace reaver
{
inline namespace prelude
{
    inline namespace functor
    {
        inline namespace _v1
        {
            template<typename T,
                typename F,
                typename std::enable_if<is_vector<std::remove_cv_t<std::remove_reference_t<T>>>::value, int>::type = 0,
                typename = decltype(std::invoke(std::declval<F>(), *std::declval<T>().begin()))>
            auto fmap(T & vec, F && f)
            {
                std::vector<decltype(std::invoke(f, *vec.begin()))> ret;
                ret.reserve(vec.size());
                for (auto && elem : vec)
                {
                    ret.push_back(std::invoke(f, elem));
                }
                return ret;
            }

            template<typename T,
                typename F,
                typename std::enable_if<is_vector<std::remove_cv_t<std::remove_reference_t<T>>>::value, int>::type = 0,
                typename = decltype(std::invoke(std::declval<F>(), std::move(*std::declval<T>().begin())))>
            auto fmap(T && vec, F && f)
            {
                std::vector<decltype(std::invoke(f, std::move(*vec.begin())))> ret;
                ret.reserve(vec.size());
                for (auto && elem : vec)
                {
                    ret.push_back(std::invoke(f, std::move(elem)));
                }
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
        }
    }
}
}
