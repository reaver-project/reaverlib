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

namespace reaver
{
inline namespace _v1
{
    enum class ref_qualifier
    {
        none,
        lvalue,
        rvalue
    };

    template<typename, bool is_const = false, bool is_volatile = false, ref_qualifier = ref_qualifier::none>
    struct function_traits;

    template<typename Ret, typename... Args, bool IsConst, bool IsVolatile, ref_qualifier RefQualifier>
    struct function_traits<Ret(Args...), IsConst, IsVolatile, RefQualifier>
    {
        using return_type = Ret;
        constexpr static bool is_const = IsConst;
        constexpr static bool is_volatile = IsVolatile;
        constexpr static ref_qualifier reference_qualifier = RefQualifier;

        template<template<typename...> typename Template, typename... AdditionalArguments>
        using explode = Template<AdditionalArguments..., Ret, Args...>;
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) const> : function_traits<Ret(Args...), true, false>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) volatile> : function_traits<Ret(Args...), false, true>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) const volatile> : function_traits<Ret(Args...), true, true>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) &> : function_traits<Ret(Args...), true, false, ref_qualifier::lvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) const &> : function_traits<Ret(Args...), true, false, ref_qualifier::lvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) volatile &> : function_traits<Ret(Args...), false, true, ref_qualifier::lvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) const volatile &> : function_traits<Ret(Args...), true, true, ref_qualifier::lvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) &&> : function_traits<Ret(Args...), true, false, ref_qualifier::rvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) const &&> : function_traits<Ret(Args...), true, false, ref_qualifier::rvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) volatile &&> : function_traits<Ret(Args...), false, true, ref_qualifier::rvalue>
    {
    };

    template<typename Ret, typename... Args>
    struct function_traits<Ret(Args...) const volatile &&> : function_traits<Ret(Args...), true, true, ref_qualifier::rvalue>
    {
    };

    template<typename F>
    using return_type = typename function_traits<F>::return_type;

    template<typename F, template<typename...> typename Template, typename... AdditionalArguments>
    using explode = typename function_traits<F>::template explode<Template, AdditionalArguments...>;
}
}
