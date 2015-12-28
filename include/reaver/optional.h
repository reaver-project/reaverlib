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

#include "variant.h"
#include "invoke.h"

namespace reaver { inline namespace _v1
{
    constexpr struct none_t {} none{};

    template<typename T>
    class optional : public variant<T, none_t>
    {
        using _base = variant<T, none_t>;
    public:
        optional() : _base{ none }
        {
        }

        using _base::_base;

        operator bool() const
        {
            return _base::index() == 0;
        }

        T & operator*()
        {
            return get();
        }

        const T & operator*() const
        {
            return get();
        }

        T & get()
        {
            return reaver::get<0>(*this);
        }

        const T & get() const
        {
            return reaver::get<0>(*this);
        }
    };

    template<typename T>
    auto make_optional(T && t)
    {
        return optional<T>{ std::forward<T>(t) };
    }

    template<typename T, typename U>
    bool operator==(const optional<T> & t, const U & u)
    {
        return t && *t == u;
    }

    template<typename T, typename U>
    bool operator==(const T & t, const optional<U> & u)
    {
        return u && t == *u;
    }

    template<typename T, typename U>
    bool operator==(const optional<T> & t, const optional<U> & u)
    {
        return (!t && !u) || (t && u && *t == *u);
    }

    template<typename T, typename U>
    bool operator!=(const optional<T> & t, const U & u)
    {
        return !(t == u);
    }

    template<typename T, typename U>
    bool operator!=(const T & t, const optional<U> & u)
    {
        return !(t == u);
    }

    template<typename T, typename U>
    bool operator!=(const optional<T> & t, const optional<U> & u)
    {
        return !(t == u);
    }

    template<typename T, typename F>
    decltype(auto) fmap(optional<T> & t, F && f)
    {
        return t ? make_optional(invoke(std::forward<F>(f), t.get())) : none;
    }

    template<typename T, typename F>
    decltype(auto) fmap(const optional<T> & t, F && f)
    {
        return t ? make_optional(invoke(std::forward<F>(f), t.get())) : none;
    }

    template<typename T, typename F>
    decltype(auto) fmap(optional<T> && t, F && f)
    {
        return t ? make_optional(invoke(std::forward<F>(f), std::move(t.get()))) : none;
    }
}}

