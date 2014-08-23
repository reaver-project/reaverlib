/**
 * Reaver Library License
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

#include <set>

#include "logger.h"

namespace reaver
{
inline namespace __v1
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
        template<typename Level = logger::always_type>
        exception(Level = {}) : _level { Level{} }
        {
            static std::set<logger::base_level> allowed_levels = {
                logger::always,
                logger::note,
                logger::info,
                logger::warning,
                logger::error,
                logger::fatal,
                logger::crash
            };

            if (allowed_levels.find(Level{}) == allowed_levels.end())
            {
                throw invalid_exception_level{};
            }

            _streamables = logger::default_level_registry()[Level{}];
        }

        ~exception()
        {
        }

        template<typename T>
        exception & operator<<(T && rhs)
        {
            _streamables.emplace_back(std::forward<T>(rhs));
            return *this;
        }

        virtual const char * what() const noexcept
        {
            return "reaver::exception\ncall e.print(<logger object>) to get more detailed output";
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
}
}
