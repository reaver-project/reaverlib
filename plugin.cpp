/**
 * Reaver Library Licence
 *
 * Copyright (C) 2013 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
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
 * Michał "Griwes" Dominiak
 *
 **/

#include "plugin.h"

#include <boost/filesystem.hpp>

std::unique_ptr<reaver::plugin> reaver::plugin::load(std::string name)
{
    if (boost::filesystem::exists(boost::filesystem::current_path().string() + name))
    {
        return std::unique_ptr<reaver::plugin>{ new plugin(name) };
    }

    return {};
}

#ifdef __unix__

#include <unistd.h>
#include <dlfcn.h>

reaver::plugin::plugin(std::string name)
{
    _handle = dlopen(name.c_str(), RTLD_LAZY);
}

reaver::plugin::~plugin()
{
    _imported.clear();
    dlclose(_handle);
}

template<typename T>
std::weak_ptr<std::function<T>> reaver::plugin::load_symbol(std::string name)
{
    if (!_handle)
    {
        _detail::_function_wrapper_impl<T> f{ (T *)dlsym(_handle, name.c_str()) };
        auto ret = f.get();
        _imported.insert(f);

        return ret;
    }

    return {};
}

#else
#error "not yet supported target"
#endif

