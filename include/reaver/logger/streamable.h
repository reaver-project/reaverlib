/**
 * Reaver Library Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include "stream_wrapper.h"

namespace reaver
{
namespace logger
{
    inline namespace _v1
    {
        class streamable
        {
        public:
            streamable() = delete;

            template<typename T>
            streamable(T && t) : _value{ std::make_unique<_streamable_impl<std::decay_t<std::remove_reference_t<T>>>>(std::forward<T>(t)) }
            {
            }

            template<typename T>
            streamable & operator=(T && t)
            {
                _value = std::make_unique<_streamable_impl<std::decay_t<std::remove_reference_t<T>>>>(std::forward<T>(t));
                return *this;
            }

            streamable(const streamable & other) : _value{ other._value->clone() }
            {
            }

            streamable & operator=(const streamable & other)
            {
                _value = other._value->clone();
                return *this;
            }

            streamable(streamable & other) : _value{ other._value->clone() }
            {
            }

            streamable(streamable &&) = default;
            streamable & operator=(streamable &&) = default;

            void stream(stream_wrapper & wr) const
            {
                _value->stream(wr);
            }

        private:
            class _streamable_base
            {
            public:
                virtual ~_streamable_base()
                {
                }

                virtual void stream(stream_wrapper & os) const = 0;
                virtual std::unique_ptr<_streamable_base> clone() const = 0;
            };

            template<typename T>
            class _streamable_impl : public _streamable_base
            {
            public:
                _streamable_impl(T value) : _value{ std::move(value) }
                {
                }

                virtual void stream(stream_wrapper & os) const override
                {
                    os << _value;
                }

                virtual std::unique_ptr<_streamable_base> clone() const override
                {
                    return std::make_unique<_streamable_impl>(_value);
                }

            private:
                T _value;
            };

            std::unique_ptr<_streamable_base> _value;
        };
    }
}
}
