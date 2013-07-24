/**
 * Miranda License
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

#include <reaver/logger.h>

using namespace reaver::logger;

namespace reaver
{
    class exception : public std::exception, public reaver::logger::logger_friend
    {
    public:
        exception(level l = always)
        {
            using reaver::style::style;
            using reaver::style::colors;
            using reaver::style::styles;

            switch (l)
            {
                case trace:
                    _strings = { std::make_pair(style(colors::bgray), "Trace: "), std::make_pair(style(), "") };
                    break;
                case debug:
                    _strings = { std::make_pair(style(colors::gray), "Debug: "), std::make_pair(style(), "") };
                    break;
                case info:
                    _strings = { std::make_pair(style(), "Info: ") };
                    break;
                case warning:
                    _strings = { std::make_pair(style(colors::bbrown, colors::def, styles::bold), "Warning: "),
                        std::make_pair(style(), "") };
                    break;
                case error:
                    _strings = { std::make_pair(style(colors::bred, colors::def, styles::bold), "Error: "),
                        std::make_pair(style(), "") };
                    break;
                case crash:
                    _strings = { std::make_pair(style(colors::bred, colors::def, styles::bold), "Internal error: "),
                        std::make_pair(style(), "") };
                    break;
                case always:
                    ;
            }
        }

        exception() : exception{ always }
        {
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
            print(dlog);

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

        level level() const
        {
            return _level;
        }

    private:
        ::level _level;

        std::vector<std::pair<reaver::style::style, std::string>> _strings;
    };
}
