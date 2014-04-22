/**
 * Reaver Library Licence
 *
 * Copyright © 2013 Michał "Griwes" Dominiak
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

#include <map>
#include <functional>
#include <utility>

#include "exception.h"

namespace reaver
{
    namespace _detail
    {
        template<typename T, typename Handle>
        struct _return_type
        {
            using type = std::map<Handle, T>;
        };

        template<typename Handle>
        struct _return_type<void, Handle>
        {
            using type = void;
        };

        template<typename Handle, typename Ret, typename... Args>
        struct _call
        {
            static std::map<Handle, Ret> call(std::map<Handle, std::function<Ret (Args...)>> callbacks, Args... args)
            {
                std::map<Handle, Ret> ret;

                for (const auto & x : callbacks)
                {
                    ret.emplace(std::make_pair(x.first, x.second(args...)));
                }

                return std::move(ret);
            }
        };

        template<typename Handle, typename... Args>
        struct _call<Handle, void, Args...>
        {
            static void call(std::map<Handle, std::function<void (Args...)>> callbacks, Args... args)
            {
                for (const auto & x : callbacks)
                {
                    x.second(args...);
                }
            }
        };

        template<typename Ret, typename... Args>
        struct _traits
        {
            using return_type = Ret;
        };

        template<typename Ret, typename Arg>
        struct _traits<Ret, Arg>
        {
            using return_type = Ret;
            using argument_type = Arg;
        };

        template<typename Ret, typename Arg1, typename Arg2>
        struct _traits<Ret, Arg1, Arg2>
        {
            using return_type = Ret;
            using first_argument_type = Arg1;
            using second_argument_type = Arg2;
        };
    }

    class incompatible_handle : public exception
    {
    public:
        incompatible_handle() : exception{ logger::crash }
        {
            *this << "incompatible callback handles used.";
        }
    };

    class handles_exhausted : public exception
    {
    public:
        handles_exhausted() : exception{ logger::crash }
        {
            *this << "callback handles exhausted.";
        }
    };

    class invalid_handle : public exception
    {
    public:
        invalid_handle() : exception{ logger::crash }
        {
            *this << "invalid callback handle used.";
        }
    };

    template<typename>
    class callbacks;

    template<typename Ret, typename... Args>
    class callbacks<Ret (Args...)> : public _detail::_traits<Ret, Args...>
    {
    public:
        class handle
        {
            friend class callbacks;

            handle(callbacks * owner, std::size_t id) : _owner{ owner }, _id{ id }
            {
            }

            callbacks * _owner;
            std::size_t _id;

            handle & operator++()
            {
                ++_id;

                if (_id == ~(std::size_t)0)
                {
                    throw handles_exhausted{};
                }

                return *this;
            }

        public:
            bool operator<(const handle & rhs) const
            {
                if (_owner != rhs._owner)
                {
                    throw incompatible_handle{};
                }

                return _id < rhs._id;
            }
        };

        callbacks() : _handle{ this, ~(std::size_t)0 }
        {
        }

        callbacks(const callbacks & rhs) : _callbacks{ rhs._callbacks }, _handle{ this, rhs._handle._id }
        {
        }

        callbacks(callbacks && rhs) : _callbacks{ std::move(rhs._callbacks) }, _handle{ this, rhs._handle._id }
        {
        }

        callbacks & operator=(const callbacks & rhs)
        {
            _callbacks = rhs._callbacks;
            _handle._id = rhs._handle._id;
        }

        callbacks & operator=(callbacks && rhs)
        {
            _callbacks = std::move(rhs._callbacks);
            _handle._id = rhs._handle._id;
        }

        handle operator+=(std::function<Ret (Args...)> f)
        {
            handle h = ++_handle;
            _callbacks.emplace(h, std::move(f));
            return h;
        }

        void operator-=(handle h)
        {
            if (h._owner != this)
            {
                throw incompatible_handle{};
            }

            if (h._id == ~(std::size_t)0)
            {
                throw invalid_handle{};
            }

            if (_callbacks.count(h))
            {
                _callbacks.erase(h);
            }
        }

        callbacks operator+(std::function<Ret (Args...)> f) const
        {
            callbacks ret{ *this };
            ret += std::move(f);
            return std::move(ret);
        }

        callbacks operator-(handle h)
        {
            callbacks ret{ *this };
            h._owner = &ret;
            ret -= h;
            return std::move(ret);
        }

        typename _detail::_return_type<Ret, handle>::type operator()(Args... args) const
        {
            return _detail::_call<handle, Ret, Args...>::call(_callbacks, args...);
        }

        void clear()
        {
            _callbacks.clear();
        }

        typename std::map<handle, std::function<Ret (Args...)>>::iterator begin()
        {
            return _callbacks.begin();
        }

        typename std::map<handle, std::function<Ret (Args...)>>::const_iterator begin() const
        {
            return _callbacks.begin();
        }

        typename std::map<handle, std::function<Ret (Args...)>>::iterator end()
        {
            return _callbacks.end();
        }

        typename std::map<handle, std::function<Ret (Args...)>>::const_iterator end() const
        {
            return _callbacks.end();
        }

        handle last() const
        {
            return _handle;
        }

        explicit operator bool() const
        {
            return !_callbacks.empty();
        }

    private:
        std::map<handle, std::function<Ret (Args...)>> _callbacks;
        handle _handle;
    };
}
