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

#include <memory>
#include <string>
#include <utility>

#include "exception.h"

namespace reaver
{
inline namespace _v1
{
    namespace _posix
    {
        extern "C" {
#include <dlfcn.h>
        }
    }

    class library_not_found : public exception
    {
    public:
        library_not_found(std::string n) : exception{ logger::error }, name{ std::move(n) }
        {
            *this << "couldn't find library `" << name << "`.";
        }

        std::string name;
    };

    class symbol_not_found : public exception
    {
    public:
        symbol_not_found(std::string n, std::string l) : exception{ logger::error }, name{ std::move(n) }, library{ std::move(l) }
        {
            *this << "couldn't find symbol `" << name << "` in library `" << library << "`.";
        }

        std::string name;
        std::string library;
    };

    class plugin
    {
    public:
        plugin(std::string name) : _handle{ _posix::dlopen(("lib" + name + ".so").c_str(), RTLD_LAZY) }, _name{ std::move(name) }
        {
            if (!_handle)
            {
                throw library_not_found{ std::move(_name) };
            }
        }

        ~plugin() noexcept
        {
            _posix::dlclose(_handle);
        }

        template<typename T>
        T * get_symbol(const std::string & name) const
        {
            auto ret = reinterpret_cast<T *>(_posix::dlsym(_handle, name.c_str()));
            if (!ret)
            {
                throw symbol_not_found{ name, _name };
            }
            return ret;
        }

    private:
        void * _handle;
        std::string _name;
    };

    inline std::shared_ptr<plugin> open_library(const std::string & name)
    {
        return std::make_shared<plugin>(name);
    }

    template<typename Signature>
    class plugin_function;

    template<typename Ret, typename... Args>
    class plugin_function<Ret(Args...)>
    {
    public:
        plugin_function(std::shared_ptr<plugin> plugin, const std::string & name)
            : _plugin{ std::move(plugin) }, _function{ _plugin->get_symbol<signature>(name) }
        {
        }

        plugin_function(const plugin_function &) = default;
        plugin_function(plugin_function &&) = default;
        plugin_function & operator=(const plugin_function &) = default;
        plugin_function & operator=(plugin_function &&) = default;

        Ret operator()(Args... args) const
        {
            return _function(std::forward<Args>(args)...);
        }

        friend bool operator==(const plugin_function & lhs, const plugin_function & rhs)
        {
            return lhs._function == rhs._function;
        }

        using signature = Ret(Args...);

        template<typename T>
        friend struct std::hash;

    private:
        std::shared_ptr<plugin> _plugin;
        signature * _function;
    };
}
}

namespace std
{
template<typename T>
struct hash<reaver::plugin_function<T>>
{
    std::size_t operator()(const reaver::plugin_function<T> & t) const
    {
        return std::hash<T *>()(t._function);
    }
};
};
