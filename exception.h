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

#include "logger.h"

namespace reaver
{
inline namespace __v1
{
    class exception : public std::exception, public reaver::logger::logger_friend
    {
    public:
        exception(logger::level l = logger::always) : _level { l }
        {
            using reaver::style::style;
            using reaver::style::colors;
            using reaver::style::styles;

            switch (l)
            {
                case logger::note:
                    _strings = { std::make_pair(style(colors::gray, colors::def, styles::bold), "Note: ") };
                    break;
                case logger::info:
                    _strings = { std::make_pair(style(colors::gray, colors::def, styles::bold), "Info: ") };
                    break;
                case logger::warning:
                    _strings = { std::make_pair(style(colors::bbrown, colors::def, styles::bold), "Warning: "),
                        std::make_pair(style(colors::bgray, colors::def, styles::bold), "") };
                    break;
                case logger::syntax:
                    _strings = { std::make_pair(style(colors::bred, colors::def, styles::bold), "Syntax error: "),
                        std::make_pair(style(colors::bgray, colors::def, styles::bold), "") };
                case logger::error:
                    _strings = { std::make_pair(style(colors::bred, colors::def, styles::bold), "Error: "),
                        std::make_pair(style(colors::bgray, colors::def, styles::bold), "") };
                    break;
                case logger::fatal:
                    _strings = { std::make_pair(style(colors::bred, colors::def, styles::bold), "Fatal error: "),
                        std::make_pair(style(colors::bgray, colors::def, styles::bold), "") };
                    break;
                case logger::crash:
                    _strings = { std::make_pair(style(colors::bred, colors::def, styles::bold), "Internal error: "),
                        std::make_pair(style(colors::bgray, colors::def, styles::bold), "") };
                    break;
                case logger::always:
                    _strings = { std::make_pair(style(), "") };
                    break;

                default:
                    throw invalid_exception_level{};
            }
        }

        ~exception()
        {
        }

        exception & operator<<(const char * str)
        {
            _strings.back().second.append(str);
            return *this;
        }

        exception & operator<<(const std::string & str)
        {
            _strings.back().second.append(str);
            return *this;
        }

        template<typename T>
        exception & operator<<(const T & rhs)
        {
            std::stringstream ss;
            ss << rhs;
            _strings.back().second.append(std::move(ss.str()));
            return *this;
        }

        exception & operator<<(const reaver::style::style & style)
        {
            _strings.push_back(std::make_pair(style, ""));
            return *this;
        }

        virtual const char * what() const noexcept
        {
            return "reaver::exception\ncall e.print(<logger object>) to get more detailed output";
        }

        virtual void print(reaver::logger::logger & l) const noexcept
        {
            auto strings = _strings;

            for (auto & stream : _streams(l))
            {
                _async(l, [=]() mutable
                {
                    for (auto x : strings)
                    {
                        stream << x.first;
                        stream << x.second;
                    }

                    stream << "\n";
                });
            }
        }

        friend reaver::logger::logger & operator<<(reaver::logger::logger &, const exception &);

        logger::level level() const
        {
            return _level;
        }

    private:
        logger::level _level;

        std::vector<std::pair<reaver::style::style, std::string>> _strings;
    };

    class file_is_directory : public exception
    {
    public:
        file_is_directory(const std::string & filename) : exception{ logger::crash }
        {
            using reaver::style::style;
            using reaver::style::colors;
            using reaver::style::styles;

            *this << "file " << style::style(colors::bgray, colors::def, styles::bold) << filename << style::style()
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

            *this << "couldn't find file " << style::style(colors::bgray, colors::def, styles::bold) << filename
                << style::style() << ".";
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

            *this << "couldn't open file " << style::style(colors::bgray, colors::def, styles::bold) << filename
                << style::style() << ".";
        }
    };
}
}
