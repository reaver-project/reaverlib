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

#include <exception>

#include "variant.h"

namespace reaver { inline namespace _v1
{
    constexpr struct error_tag_type {} error_tag{};

    template<typename T, typename Error = std::exception_ptr>
    class expected
    {
        struct _wrap_error
        {
            Error value;
        };

    public:
        explicit expected(T t) : _value{ std::move(t) }
        {
        }

        expected(error_tag_type, Error err) : _value{ _wrap_error{ std::move(err) } }
        {
        }

        T & operator*()
        {
            return get<0>(fmap(_value, make_overload_set(
                [](T & value) -> T & { return value; },
                [](_wrap_error & err) -> T & { throw err.value; }
            )));
        }

        const T & operator*() const
        {
            return get<0>(fmap(_value, make_overload_set(
                [](const T & value) -> const T & { return value; },
                [](const _wrap_error & err) -> const T & { throw err.value; }
            )));
        }

        T * operator->()
        {
            return &get<0>(fmap(_value, make_overload_set(
                [](T & value) -> T & { return value; },
                [](_wrap_error & err) -> T & { throw err.value; }
            )));
        }

        const T * operator->() const
        {
            return &get<0>(fmap(_value, make_overload_set(
                [](const T & value) -> const T & { return value; },
                [](const _wrap_error & err) -> const T & { throw err.value; }
            )));
        }

        explicit operator bool() const
        {
            return _value.index() == 0;
        }

        Error & get_error()
        {
            return get<1>(_value).value;
        }

        const Error & get_error() const
        {
            return get<1>(_value).value;
        }

    private:
        variant<T, _wrap_error> _value;
    };

    template<typename T>
    class expected<T, void> : public expected<T, unit>
    {
    public:
        using expected<T, unit>::expected;
    };

    template<typename T, typename Error = std::exception_ptr>
    expected<T, Error> make_expected(T t)
    {
        return expected<T, Error>{ std::move(t) };
    }

    template<typename Error, typename T>
    expected<T, Error> make_expected_err_type(T t)
    {
        return expected<T, Error>{ std::move(t) };
    }

    template<typename T, typename Error>
    expected<T, Error> make_error(Error err)
    {
        return expected<T, Error>{ error_tag, std::move(err) };
    }

    template<typename T>
    expected<T, void> make_error()
    {
        return expected<T, void>{ error_tag, unit{} };
    }

    template<typename T, typename Error, typename F>
    auto fmap(expected<T, Error> & exp, F && f) -> decltype(make_expected_err_type<Error>(invoke(std::forward<F>(f), *exp)))
    {
        if (exp)
        {
            return make_expected_err_type<Error>(invoke(std::forward<F>(f), *exp));
        }

        return { error_tag, exp.get_error() };
    }

    template<typename T, typename Error, typename F>
    auto fmap(const expected<T, Error> & exp, F && f) -> decltype(make_expected_err_type<Error>(invoke(std::forward<F>(f), *exp)))
    {
        if (exp)
        {
            return make_expected_err_type<Error>(invoke(std::forward<F>(f), *exp));
        }

        return { error_tag, exp.get_error() };
    }

    template<typename T, typename Error, typename F>
    auto fmap(expected<T, Error> && exp, F && f) -> decltype(make_expected_err_type<Error>(invoke(std::forward<F>(f), *exp)))
    {
        if (exp)
        {
            return make_expected_err_type<Error>(invoke(std::forward<F>(f), std::move(*exp)));
        }

        return { error_tag, std::move(exp.get_error()) };
    }

    template<typename T, typename ErrorInner, typename ErrorOuter>
    auto join(expected<expected<T, ErrorInner>, ErrorOuter> & exp) -> expected<T, variant<ErrorInner, ErrorOuter>>
    {
        if (exp && *exp)
        {
            return make_expected_err_type<variant<ErrorInner, ErrorOuter>>(**exp);
        }

        if (exp)
        {
            return { error_tag, exp->get_error() };
        }

        return { error_tag, exp.get_error() };
    }

    template<typename T, typename ErrorInner, typename ErrorOuter>
    auto join(const expected<expected<T, ErrorInner>, ErrorOuter> & exp) -> expected<T, variant<ErrorInner, ErrorOuter>>
    {
        if (exp && *exp)
        {
            return make_expected_err_type<variant<ErrorInner, ErrorOuter>>(**exp);
        }

        if (exp)
        {
            return { error_tag, exp->get_error() };
        }

        return { error_tag, exp.get_error() };
    }

    template<typename T, typename ErrorInner, typename ErrorOuter>
    auto join(expected<expected<T, ErrorInner>, ErrorOuter> && exp) -> expected<T, variant<ErrorInner, ErrorOuter>>
    {
        if (exp && *exp)
        {
            return make_expected_err_type<variant<ErrorInner, ErrorOuter>>(std::move(**exp));
        }

        if (exp)
        {
            return { error_tag, std::move(exp->get_error()) };
        }

        return { error_tag, std::move(exp.get_error()) };
    }
}}

