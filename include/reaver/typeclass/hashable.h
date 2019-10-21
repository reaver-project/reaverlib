/**
 * Reaver Library Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "sfinae_function.h"
#include "typeclass.h"

namespace reaver
{
inline namespace prelude
{
    inline namespace _v1
    {
        // TODO: use this
        struct hashable_definition
        {
            using hash = std::size_t() const;
        };

        struct hashable
        {
            TYPECLASS_INSTANCE(typename T);
        };

        template<typename T>
        auto hash(const T & t) SFINAE_FUNCTION(tc_instance<hashable, T>::hash(t));

        // clang-format off
        DEFAULT_INSTANCE(hashable, T)
        {
            // prefer std::hash specialization over T::hash_value
            template<typename U = T, typename... Ts>
            static auto hash(const U & t, Ts...) SFINAE_FUNCTION(t.hash_value());

            template<typename U = T>
            static auto hash(const U & t) SFINAE_FUNCTION(std::hash<U>()(t));
        };
    }
}
}
