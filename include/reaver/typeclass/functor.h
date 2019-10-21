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

#include "typeclass.h"

namespace reaver
{
inline namespace prelude
{
    inline namespace _v1
    {
        template<template<typename...> typename F>
        struct functor
        {
            TYPECLASS_INSTANCE(typename U);
        };

        template<template<typename...> typename F,
            typename A,
            typename... Args,
            typename Fun,
            decltype(tc_instance<functor<F>, F<A, Args...>>::fmap(std::declval<F<A, Args...>>(), std::declval<Fun>()), int())...>
        auto fmap(F<A, Args...> f, Fun && fn)
        {
            return tc_instance<functor<F>, F<A, Args...>>::fmap(std::move(f), std::forward<Fun>(fn));
        }

        // clang-format off
        DEFAULT_INSTANCE_TEMPLATE((template<typename...> typename F), (F), functor, U)
        {
            template<typename Fun, typename... Args>
            static auto fmap(F<Args...>, Fun && fn) = delete;
        };
        // clang-format on
    }
}
}
