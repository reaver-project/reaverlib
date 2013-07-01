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

#pragma once

#include <string>
#include <memory>
#include <functional>
#include <set>

namespace reaver
{
    namespace _detail
    {
        class _function_wrapper
        {
        public:
            virtual ~_function_wrapper() {}
        };

        template<typename T>
        class _function_wrapper_impl
        {
        public:
            _function_wrapper_impl(const T * f) : _f{ std::make_shared<std::function<T>>(f) }
            {
            }

            virtual ~_function_wrapper_impl() {}

            std::weak_ptr<std::function<T>> get()
            {
                return _f;
            }

        private:
            std::shared_ptr<std::function<T>> _f;
        };
    }

    class plugin
    {
    public:
        static std::unique_ptr<plugin> load(std::string);

        plugin(std::string);
        ~plugin();

        void close();

        template<typename T>
        std::weak_ptr<std::function<T>> load_symbol(std::string);

    private:
        std::set<_detail::_function_wrapper> _imported;
        void * _handle;
    };
}
