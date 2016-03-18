/**
 * Reaver Library License
 *
 * Copyright © 2013, 2015 Michał "Griwes" Dominiak
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

#include <unordered_set>

#include "logger.h"
#include "prelude/functor.h"

namespace reaver { inline namespace _v1
{
    class invalid_exception_level : public std::runtime_error
    {
    public:
        invalid_exception_level() : std::runtime_error{ "a logger level below `warning` used as an exception level" }
        {
        }
    };

    class exception : public std::exception, public reaver::logger::logger_friend
    {
    public:
        template<typename Level = logger::always_type, typename std::enable_if<logger::is_logger_level<Level>::value, int>::type = 0>
        exception(Level l = {}) : _level{ l }
        {
            static std::unordered_set<logger::base_level, logger::hasher> allowed_levels = {
                logger::always,
                logger::note,
                logger::info,
                logger::warning,
                logger::error,
                logger::fatal,
                logger::crash
            };

            if (allowed_levels.find(l) == allowed_levels.end())
            {
                throw invalid_exception_level{};
            }

            _streamables = logger::default_level_registry()[l];
        }

        exception(const exception &) = default;
        exception(exception &&) = default;

        virtual ~exception() = default;

        template<typename T>
        exception & operator<<(T && rhs)
        {
            _streamables.emplace_back(std::forward<T>(rhs));
            return *this;
        }

        virtual const char * what() const noexcept
        {
            if (_what.empty())
            {
                std::ostringstream stream;

                {
                    logger::logger log;
                    log.add_stream(stream);

                    print(log);
                }

                stream.flush();
                _what = stream.str();
            }

            return _what.c_str();
        }

        virtual void print(reaver::logger::logger & l) const noexcept
        {
            auto streamables = _streamables;
            streamables.emplace_back('\n');

            logger_friend::_write(l, std::move(streamables));
        }

        friend reaver::logger::logger & operator<<(reaver::logger::logger &, const exception &);

        logger::base_level level() const
        {
            return _level;
        }

    private:
        logger::base_level _level;

        std::vector<logger::streamable> _streamables;
        mutable std::string _what;
    };

    class exception_list : public exception
    {
    public:
        exception_list() : exception{ logger::always }
        {
        }

        exception_list(const exception_list & other)
        {
            std::lock_guard<std::mutex> lock{ other._mutex };
            _exceptions = other._exceptions;
        }

        exception_list(exception_list && other) : _exceptions{ std::move(other._exceptions) }
        {
        }

        exception_list & operator=(const exception_list & other)
        {
            std::lock_guard<std::mutex> lock{ other._mutex };
            std::lock_guard<std::mutex> self_lock{ _mutex };

            _exceptions = other._exceptions;

            return *this;
        }

        exception_list & operator=(exception_list && other)
        {
            std::lock_guard<std::mutex> lock{ _mutex };

            _exceptions = std::move(other._exceptions);

            return *this;
        }

        void push_back(std::exception_ptr ptr)
        {
            std::lock_guard<std::mutex> lock{ _mutex };
            _exceptions.push_back(ptr);
        }

        std::size_t size() const
        {
            std::lock_guard<std::mutex> lock{ _mutex };
            return _exceptions.size();
        }

        template<typename F>
        auto map(F && f) const
        {
            std::lock_guard<std::mutex> lock{ _mutex };
            return fmap(_exceptions, std::forward<F>(f));
        }

        virtual void print(reaver::logger::logger & l) const noexcept override
        {
            map([&](auto ptr) {
                try
                {
                    std::rethrow_exception(ptr);
                }
                catch (reaver::exception & ex)
                {
                    ex.print(l);
                }
                catch (std::exception & ex)
                {
                    l(logger::fatal) << ex.what();
                }

                return unit{};
            });
        }

    private:
        mutable std::mutex _mutex;
        std::vector<std::exception_ptr> _exceptions;
    };

    class file_is_directory : public exception
    {
    public:
        file_is_directory(const std::string & filename) : exception{ logger::crash }
        {
            using reaver::style::style;
            using reaver::style::colors;
            using reaver::style::styles;

            *this << "file " << style(colors::bgray, colors::def, styles::bold) << filename << style()
                << " is a directory.";
        }
    };

    class file_not_found : public exception
    {
    public:
        file_not_found(const std::string & filename) : exception{ logger::crash }
        {
            using reaver::style::style;
            using reaver::style::colors;
            using reaver::style::styles;

            *this << "couldn't find file " << style(colors::bgray, colors::def, styles::bold) << filename
                << style() << ".";
        }
    };

    class file_failed_to_open : public exception
    {
    public:
        file_failed_to_open(const std::string & filename) : exception{ logger::crash }
        {
            using reaver::style::style;
            using reaver::style::colors;
            using reaver::style::styles;

            *this << "couldn't open file " << style(colors::bgray, colors::def, styles::bold) << filename
                << style() << ".";
        }
    };
}}
