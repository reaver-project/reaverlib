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

#include <ostream>
#include <memory>

namespace reaver
{
    namespace logger { inline namespace _v1
    {
        namespace _detail
        {
            class _stream_wrapper_impl
            {
            public:
                virtual ~_stream_wrapper_impl() {}

                virtual std::ostream & get() = 0;
            };

            class _stream_ref_wrapper : public _stream_wrapper_impl
            {
            public:
                _stream_ref_wrapper(std::ostream & stream) : _stream{ stream }
                {
                }

                virtual std::ostream & get()
                {
                    return _stream;
                }

            private:
                std::ostream & _stream;
            };

            class _stream_uptr_wrapper : public _stream_wrapper_impl
            {
            public:
                _stream_uptr_wrapper(std::unique_ptr<std::ostream> stream) : _stream{ std::move(stream) }
                {
                }

                virtual std::ostream & get()
                {
                    return *_stream;
                }

            private:
                std::unique_ptr<std::ostream> _stream;
            };
        }

        class stream_wrapper
        {
        public:
            stream_wrapper(std::ostream & stream) : _impl{ new _detail::_stream_ref_wrapper{ stream } }
            {
            }

            stream_wrapper(std::unique_ptr<std::ostream> & stream) : _impl{ new _detail::_stream_uptr_wrapper{ std::move(stream) } }
            {
            }

            stream_wrapper(const stream_wrapper &) = default;
            stream_wrapper(stream_wrapper &&) = default;

            template<typename T>
            stream_wrapper & operator<<(T && rhs)
            {
                _impl->get() << std::forward<T>(rhs);
                return *this;
            }

        private:
            std::unique_ptr<_detail::_stream_wrapper_impl> _impl;
        };
    }}
}
