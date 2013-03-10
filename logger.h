/**
 * Reaver Library
 *
 * Copyright (C) 2012-2013 Reaver Project Team:
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

#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <fstream>

#include "style.h"

namespace reaver
{
    namespace logger
    {
        enum class level
        {
            trace,
            debug,
            info,
            warning,
            error,
            crash,
            always
        };

        static constexpr auto trace = level::trace;
        static constexpr auto debug = level::debug;
        static constexpr auto info = level::info;
        static constexpr auto warning = level::warning;
        static constexpr auto error = level::error;
        static constexpr auto crash = level::crash;
        static constexpr auto always = level::always;

        class logger;
        extern logger log;

        class action
        {
        public:
            action(logger & log = reaver::logger::log, level lev = always) : _logger(log), _level(lev), _strings()
            {
                _strings.push_back({{}, ""});
            }

            ~action();

            action & operator<<(const char * str)
            {
                _strings.back().second.append(str);
                return *this;
            }

            action & operator<<(const std::string & str)
            {
                _strings.back().second.append(str);
                return *this;
            }

            template<typename T>
            action & operator<<(const T & rhs)
            {
                _strings.back().second.append(std::to_string(rhs));
                return *this;
            }

            action & operator<<(const reaver::style::style & style)
            {
                _strings.push_back(std::make_pair(style, ""));
                return *this;
            }

            friend class logger;

        private:
            action(logger &, level, const std::vector<std::pair<reaver::style::style, std::string>> &);

            logger & _logger;
            level _level;

            std::vector<std::pair<reaver::style::style, std::string>> _strings;
        };

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
                _stream_ref_wrapper(std::ostream & stream) : _stream(stream)
                {
                }

                virtual std::ostream & get()
                {
                    return _stream;
                }

            private:
                std::ostream & _stream;
            };

            class _stream_shptr_wrapper : public _stream_wrapper_impl
            {
            public:
                _stream_shptr_wrapper(std::shared_ptr<std::fstream> stream) : _stream{stream}
                {
                }

                virtual std::ostream & get()
                {
                    return *_stream;
                }

            private:
                std::shared_ptr<std::fstream> _stream;
            };
        }

        class stream_wrapper
        {
        public:
            stream_wrapper(std::ostream &);
            stream_wrapper(std::shared_ptr<std::fstream> &);

            stream_wrapper(const stream_wrapper &) = default;

            ~stream_wrapper();

            stream_wrapper & operator<<(const std::string &);
            stream_wrapper & operator<<(const style::style &);

        private:
            std::shared_ptr<_detail::_stream_wrapper_impl> _impl;
        };

        extern class logger
        {
        public:
            logger(level = info);
            logger(std::ostream &, level = info);
            ~logger();

            void add_stream(const stream_wrapper &);

            action operator()(level = always);

            friend class action;

        private:
            void _async(std::function<void()>);

            bool _quit = false;

            level _level;

            std::thread _worker;
            std::queue<std::function<void()>> _queue;
            std::mutex _queue_mutex; // I don't like this

            std::vector<stream_wrapper> _streams;
        } log;
    }
}
