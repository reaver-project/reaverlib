/**
 * Reaver Library Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "functor.h"

namespace reaver
{
inline namespace prelude
{
    inline namespace monad
    {
        inline namespace _v1
        {
            template<typename Vector,
                typename std::enable_if<is_vector<std::remove_cv_t<std::remove_reference_t<Vector>>>::value
                        && is_vector<typename std::remove_cv_t<std::remove_reference_t<Vector>>::value_type>::value,
                    int>::type = 0>
            auto join(Vector & vector)
            {
                std::size_t total_size = 0;
                for (auto && elem : vector)
                {
                    total_size += elem.size();
                }

                std::vector<typename std::remove_cv_t<std::remove_reference_t<Vector>>::value_type::value_type> ret;
                ret.reserve(total_size);
                for (auto && elem : vector)
                {
                    std::copy(elem.begin(), elem.end(), std::back_inserter(ret));
                }

                return ret;
            }

            template<typename Vector,
                typename std::enable_if<is_vector<std::remove_cv_t<std::remove_reference_t<Vector>>>::value
                        && is_vector<typename std::remove_cv_t<std::remove_reference_t<Vector>>::value_type>::value,
                    int>::type = 0>
            auto join(Vector && vector)
            {
                std::size_t total_size = 0;
                for (auto && elem : vector)
                {
                    total_size += elem.size();
                }

                std::vector<typename std::remove_cv_t<std::remove_reference_t<Vector>>::value_type::value_type> ret;
                ret.reserve(total_size);
                for (auto && elem : vector)
                {
                    std::copy(std::make_move_iterator(elem.begin()), std::make_move_iterator(elem.end()), std::back_inserter(ret));
                }

                return ret;
            }

            namespace _prelude_helpers
            {
                template<typename T>
                T _id(T && t);
            }

            template<typename T, typename = decltype(mbind(std::declval<T>(), &_prelude_helpers::_id<T>))>
            auto join(T && t)
            {
                return mbind(std::forward<T>(t), [](auto && element) { return std::forward<decltype(element)>(element); });
            }

            template<typename T, typename F, typename = decltype(join(fmap(std::declval<T>(), std::declval<F>())))>
            auto mbind(T && t, F && f)
            {
                return join(fmap(std::forward<T>(t), std::forward<F>(f)));
            }
        }
    }
}
}
