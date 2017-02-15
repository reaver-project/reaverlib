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

#include "traits.h"
#include "variant.h"

namespace reaver
{
inline namespace _v1
{
    class const_call_operator_not_available : public exception
    {
    public:
        const_call_operator_not_available() : exception{ logger::fatal }
        {
            *this << "tried to invoke non-const call operator of the "
                     "underlying function object "
                     "via calling the const call operator of reaver::function";
        }
    };

    template<typename Signature>
    class function;

    namespace _detail
    {
        template<typename Result>
        struct _invoker
        {
            template<typename F, typename V>
            decltype(auto) operator()(V && value, F && f) const
            {
                return get<0>(fmap(std::forward<V>(value), std::forward<F>(f)));
            }
        };

        template<>
        struct _invoker<void>
        {
            template<typename F, typename V>
            void operator()(V && value, F && f) const
            {
                fmap(std::forward<V>(value), [&](auto && arg) {
                    std::forward<F>(f)(std::forward<decltype(arg)>(arg));
                    return unit{};
                });
            }
        };
    }

    template<typename Result, typename... Args>
    class function<Result(Args...)>
    {
        using _free_function = Result (*)(Args...);

    public:
        using result_type = Result;

        function() = delete;

        function(_free_function fptr) : _fptr{ fptr }
        {
        }

        function(const function &) = delete;
        function(function &&) = default;

        function & operator=(const function &) = delete;
        function & operator=(function &&) = default;

        template<typename T,
            typename std::enable_if<std::is_convertible<decltype(std::declval<T>()(std::declval<Args>()...)), Result>::value,
                //            && std::is_convertible<decltype(std::declval<T
                //            &>()(std::declval<Args>()...)), Result>::value
                //            &&
                //            std::is_convertible<decltype(std::declval<const
                //            T
                //            &>()(std::declval<Args>()...)),
                //            Result>::value,
                int>::type = 0>
        function(T t)
            : _context{ reinterpret_cast<void *>(new _invoker<T>{ std::move(t) }) }, _fptr{
                  _erased_invoker{ _context, &_invoker<T>::call_lvalue_ref, &_invoker<T>::call_rvalue_ref, &_invoker<T>::call_const_ref, &_invoker<T>::dtor }
              }
        {
        }

        Result operator()(Args... args) &
        {
            return _detail::_invoker<Result>()(_fptr, [&](auto & value) { return value(std::forward<Args>(args)...); });
        }

        Result operator()(Args... args) &&
        {
            return _detail::_invoker<Result>()(std::move(_fptr), [&](auto value) { return std::move(value)(std::forward<Args>(args)...); });
        }

        Result operator()(Args... args) const &
        {
            return _detail::_invoker<Result>()(_fptr, [&](auto & value) { return value(std::forward<Args>(args)...); });
        }

    private:
        template<typename T>
        class _invoker
        {
        public:
            _invoker(T value) : _value{ std::move(value) }
            {
            }

            static Result call_lvalue_ref(void * context, Args... args)
            {
                T & value = reinterpret_cast<_invoker *>(context)->_value;
                return value(std::forward<Args>(args)...);
            }

            static Result call_rvalue_ref(void * context, Args... args)
            {
                T & value = reinterpret_cast<_invoker *>(context)->_value;
                return std::move(value)(std::forward<Args>(args)...);
            }

            template<typename U = T, typename std::enable_if<is_callable<const U, Args...>::value, int>::type = 0>
            static Result call_const_ref(void * context, Args... args)
            {
                const T & value = reinterpret_cast<_invoker *>(context)->_value;
                return value(std::forward<Args>(args)...);
            }

            template<typename U = T, typename std::enable_if<!is_callable<const U, Args...>::value, int>::type = 0>
            static Result call_const_ref(void *, Args...)
            {
                throw const_call_operator_not_available{};
            }

            static void dtor(void * context)
            {
                delete reinterpret_cast<_invoker *>(context);
            }

        private:
            T _value;
        };

        class _erased_invoker
        {
        public:
            using dtor_type = void (*)(void *);
            using erased_function = Result (*)(void *, Args...);

            _erased_invoker(void * context, erased_function lvalue_ref, erased_function rvalue_ref, erased_function const_ref, dtor_type dtor)
                : _lvalue_ref{ lvalue_ref }, _rvalue_ref{ rvalue_ref }, _const_ref{ const_ref }, _dtor{ dtor }, _context{ context }
            {
            }

            _erased_invoker(const _erased_invoker &) = delete;

            _erased_invoker(_erased_invoker && other) noexcept
                : _lvalue_ref{ other._lvalue_ref }, _rvalue_ref{ other._rvalue_ref }, _const_ref{ other._const_ref }, _dtor{ other._dtor }, _context{
                      other._context
                  }
            {
                other._context = nullptr;
            }

            ~_erased_invoker()
            {
                _dtor(_context);
            }

            Result operator()(Args... args) &
            {
                assert(_context);
                return _lvalue_ref(_context, std::forward<Args>(args)...);
            }

            Result operator()(Args... args) &&
            {
                assert(_context);
                return _rvalue_ref(_context, std::forward<Args>(args)...);
            }

            Result operator()(Args... args) const &
            {
                assert(_context);
                return _const_ref(_context, std::forward<Args>(args)...);
            }

        private:
            erased_function _lvalue_ref;
            erased_function _rvalue_ref;
            erased_function _const_ref;

            dtor_type _dtor;

            void * _context;
        };

        void * _context = nullptr;

        variant<_free_function, _erased_invoker> _fptr;
    };
}
}
