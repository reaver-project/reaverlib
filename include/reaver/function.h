/**
 * Reaver Library Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

namespace reaver
{
inline namespace _v1
{
    template<typename Signature>
    class unique_function;

    template<typename Return, typename... Args>
    class unique_function<Return(Args...)>
    {
    public:
        template<typename T>
        unique_function(T t)
        {
            _data = { new T(std::move(t)), +[](void * ptr) { delete reinterpret_cast<T *>(ptr); } };
            _invoker = +[](void * ptr, Args &&... args) { return std::invoke(*reinterpret_cast<T *>(ptr), std::forward<Args>(args)...); };
        }

        unique_function(const unique_function &) = delete;
        unique_function(unique_function &&) = default;
        unique_function & operator=(const unique_function &) = delete;
        unique_function & operator=(unique_function &&) = default;

        Return operator()(Args... args)
        {
            return _invoker(_data.get(), std::forward<Args>(args)...);
        }

    private:
        using deleter = void (*)(void *);
        using invoker = Return (*)(void *, Args &&...);
        std::unique_ptr<void, deleter> _data = { nullptr, nullptr };
        invoker _invoker = { nullptr };
    };

    template<typename Return, typename... Args>
    class unique_function<Return(Args...) const>
    {
    public:
        template<typename T>
        unique_function(T t)
        {
            _data = { new T(std::move(t)), +[](const void * ptr) { delete reinterpret_cast<const T *>(ptr); } };
            _invoker = +[](const void * ptr, Args &&... args) { return std::invoke(*reinterpret_cast<const T *>(ptr), std::forward<Args>(args)...); };
        }

        unique_function(const unique_function &) = delete;
        unique_function(unique_function &&) = default;
        unique_function & operator=(const unique_function &) = delete;
        unique_function & operator=(unique_function &&) = default;

        Return operator()(Args... args) const
        {
            return _invoker(_data.get(), std::forward<Args>(args)...);
        }

    private:
        using deleter = void (*)(const void *);
        using invoker = Return (*)(const void *, Args &&...);
        std::unique_ptr<const void, deleter> _data = { nullptr, nullptr };
        invoker _invoker = { nullptr };
    };
}
}
