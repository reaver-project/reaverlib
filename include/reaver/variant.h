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
#include <type_traits>
#include <variant>

#include "exception.h"
#include "overloads.h"
#include "tpl/concat.h"
#include "tpl/index_of.h"
#include "tpl/map.h"
#include "tpl/nth.h"
#include "tpl/rebind.h"
#include "tpl/replace.h"
#include "tpl/unique.h"
#include "tpl/vector.h"
#include "traits.h"

namespace reaver
{
inline namespace _v1
{
    template<typename T>
    struct recursive_wrapper
    {
        recursive_wrapper() : _storage{ std::make_shared<T>() }
        {
        }

        recursive_wrapper(const recursive_wrapper & other) : _storage{ std::make_shared<T>(*other._storage) }
        {
        }
        recursive_wrapper(recursive_wrapper &&) noexcept = default;

        recursive_wrapper & operator=(const recursive_wrapper & rhs)
        {
            _storage = std::make_shared<T>(*rhs._storage);
            return *this;
        }
        recursive_wrapper & operator=(recursive_wrapper &&) noexcept = default;

        recursive_wrapper & operator=(const T & t)
        {
            _storage = std::make_shared<T>(t);
            return *this;
        }

        recursive_wrapper & operator=(T && t)
        {
            _storage = std::make_shared<T>(std::move(t));
            return *this;
        }

        recursive_wrapper(T t) : _storage{ std::make_shared<T>(std::move(t)) }
        {
        }

        operator T &()
        {
            return *_storage;
        }

        operator const T &() const
        {
            return *_storage;
        }

        T & operator*()
        {
            return *_storage;
        }

        const T & operator*() const
        {
            return *_storage;
        }

        T * operator->()
        {
            return &*_storage;
        }

        const T * operator->() const
        {
            return &*_storage;
        }

        template<typename U = T, typename = decltype(std::declval<U>().index())>
        auto index() const
        {
            return _storage->index();
        }

        template<typename U = T, typename = decltype(std::declval<U>() == std::declval<U>())>
        bool operator==(const recursive_wrapper & rhs) const
        {
            return *_storage == *rhs;
        }

        template<typename U = T, typename = decltype(std::declval<U>() != std::declval<U>())>
        bool operator!=(const recursive_wrapper & rhs) const
        {
            return *_storage != *rhs;
        }

        template<typename U = T, typename = decltype(std::declval<U>() < std::declval<U>())>
        bool operator<(const recursive_wrapper & rhs) const
        {
            return *_storage < *rhs;
        }

    private:
        std::shared_ptr<T> _storage;
    };

    namespace _detail
    {
        template<typename T>
        decltype(auto) _dereference_wrapper(T && t)
        {
            return std::forward<T>(t);
        }

        template<typename T>
        auto _dereference_wrapper(recursive_wrapper<T> & t)
        {
            return *t;
        }

        template<typename T>
        auto _dereference_wrapper(const recursive_wrapper<T> & t)
        {
            return *t;
        }

        template<typename T>
        auto _dereference_wrapper(recursive_wrapper<T> && t)
        {
            return std::move(*t);
        }
    }

    template<typename... Ts, typename F>
    auto fmap(const std::variant<Ts...> & var, F && f)
    {
        using result_type =
            tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), _detail::_dereference_wrapper(std::declval<const Ts &>())))...>, std::variant>;
        return std::visit([&](auto && val) -> result_type { return std::forward<F>(f)(_detail::_dereference_wrapper(std::forward<decltype(val)>(val))); }, var);
    }

    template<typename... Ts, typename F>
    auto fmap(std::variant<Ts...> & var, F && f)
    {
        using result_type =
            tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), _detail::_dereference_wrapper(std::declval<Ts &>())))...>, std::variant>;
        return std::visit([&](auto && val) -> result_type { return std::forward<F>(f)(_detail::_dereference_wrapper(std::forward<decltype(val)>(val))); }, var);
    }
    template<typename... Ts, typename F>
    auto fmap(std::variant<Ts...> && var, F && f)
    {
        using result_type =
            tpl::rebind<tpl::unique<decltype(std::invoke(std::forward<F>(f), _detail::_dereference_wrapper(std::declval<Ts &&>())))...>, std::variant>;
        return std::visit(
            [&](auto && val) -> result_type { return std::forward<F>(f)(_detail::_dereference_wrapper(std::forward<decltype(val)>(val))); }, std::move(var));
    }

    template<typename F, typename... Ts, typename = decltype(std::get<0>(fmap(std::declval<const std::variant<Ts...> &>(), std::declval<F>())))>
    auto mbind(const std::variant<Ts...> & v, F && f)
    {
        using return_type =
            tpl::rebind<tpl::rebind<tpl::rebind<tpl::map<tpl::unbind<decltype(fmap(v, std::forward<F>(f)))>, tpl::unbind>, tpl::concat>, tpl::unique>,
                std::variant>;

        return std::get<0>(fmap(v, [&](auto && value) -> return_type { return std::invoke(std::forward<F>(f), std::forward<decltype(value)>(value)); }));
    }

    template<typename F, typename... Ts, typename = decltype(std::get<0>(fmap(std::declval<std::variant<Ts...> &>(), std::declval<F>())))>
    auto mbind(std::variant<Ts...> & v, F && f)
    {
        using return_type =
            tpl::rebind<tpl::rebind<tpl::rebind<tpl::map<tpl::unbind<decltype(fmap(v, std::forward<F>(f)))>, tpl::unbind>, tpl::concat>, tpl::unique>,
                std::variant>;

        return std::get<0>(fmap(v, [&](auto && value) -> return_type { return std::invoke(std::forward<F>(f), std::forward<decltype(value)>(value)); }));
    }

    template<typename F, typename... Ts, typename = decltype(std::get<0>(fmap(std::declval<std::variant<Ts...>>(), std::declval<F>())))>
    auto mbind(std::variant<Ts...> && v, F && f)
    {
        using return_type = tpl::rebind<
            tpl::rebind<tpl::rebind<tpl::map<tpl::unbind<decltype(fmap(std::move(v), std::forward<F>(f)))>, tpl::unbind>, tpl::concat>, tpl::unique>,
            std::variant>;

        return std::get<0>(
            fmap(std::move(v), [&](auto && value) -> return_type { return std::invoke(std::forward<F>(f), std::forward<decltype(value)>(value)); }));
    }

    template<typename T, typename U>
    bool operator==(const recursive_wrapper<T> & lhs, const U & rhs)
    {
        return *lhs == rhs;
    }

    template<typename T, typename U>
    bool operator==(const T & lhs, const recursive_wrapper<U> & rhs)
    {
        return lhs == *rhs;
    }

    template<typename T, typename U>
    bool operator!=(const recursive_wrapper<T> & lhs, const U & rhs)
    {
        return *lhs != rhs;
    }

    template<typename T, typename U>
    bool operator!=(const T & lhs, const recursive_wrapper<U> & rhs)
    {
        return lhs != *rhs;
    }

    template<typename T, typename U>
    bool operator<(const recursive_wrapper<T> & lhs, const U & rhs)
    {
        return *lhs < rhs;
    }

    template<typename T, typename U>
    bool operator<(const T & lhs, const recursive_wrapper<U> & rhs)
    {
        return lhs < *rhs;
    }
}
}
