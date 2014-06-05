/**
 * Reaver Library Licence
 *
 * Copyright © 2012-2013 Michał "Griwes" Dominiak
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

#include "logger.h"

reaver::logger::__v1::logger reaver::logger::__v1::dlog{ std::cout };

reaver::logger::__v1::logger::logger(reaver::logger::__v1::level level) : _level{ level }, _worker{ [=]()
    {
        while (!_quit)
        {
            _semaphore.wait();

            std::queue<std::function<void()>> q;

            {
                std::lock_guard<std::mutex> lock{ _lock };
                std::swap(q, _queue);
            }

            while (q.size())
            {
                q.front()();
                q.pop();
            }
        }
    } }, _streams{}
{
}

reaver::logger::__v1::logger::logger(std::ostream & stream, reaver::logger::__v1::level level) : logger{ level }
{
    _streams.push_back({ stream });
}

reaver::logger::__v1::logger::~logger()
{
    _async({ [=](){ _quit = true; } });
    _worker.join();
}

void reaver::logger::__v1::logger::add_stream(const reaver::logger::__v1::stream_wrapper & stream)
{
    _streams.push_back(stream);
}

void reaver::logger::__v1::logger::_async(std::function<void()> f)
{
    std::lock_guard<std::mutex> lock{ _lock };

    _queue.push(f);

    _semaphore.notify();
}

reaver::logger::__v1::action::action(logger & log, reaver::logger::__v1::level level, const std::vector<std::pair<reaver::style::style, std::string>> & init)
    : _logger{ log }, _level{ level }, _strings{ init }
{
}

reaver::logger::__v1::action::~action()
{
    if (_level < logger_friend::_level(_logger))
    {
        return;
    }

    std::vector<std::pair<reaver::style::style, std::string>> strings = _strings;

    for (auto & stream : _streams(_logger))
    {
        _async(_logger, [=]() mutable
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

reaver::logger::__v1::stream_wrapper::stream_wrapper(std::ostream & stream) : _impl{ new _detail::_stream_ref_wrapper{ stream } }
{
}

reaver::logger::__v1::stream_wrapper::stream_wrapper(const std::shared_ptr<std::fstream> & stream) : _impl{ new _detail::_stream_shptr_wrapper{ stream } }
{
}

reaver::logger::__v1::stream_wrapper::~stream_wrapper()
{
}

reaver::logger::__v1::stream_wrapper & reaver::logger::__v1::stream_wrapper::operator<<(const std::string & str)
{
    _impl->get() << str;
    return *this;
}

reaver::logger::__v1::stream_wrapper & reaver::logger::__v1::stream_wrapper::operator<<(const reaver::style::style & style)
{
    _impl->get() << style;
    return *this;
}

reaver::logger::__v1::action reaver::logger::__v1::logger::operator()(reaver::logger::__v1::level level)
{
    using reaver::style::style;
    using reaver::style::colors;
    using reaver::style::styles;

    switch (level)
    {
        case trace:
            return action(*this, level, { std::make_pair(style(colors::bgray), "Trace: "), std::make_pair(style(), "") });
        case debug:
            return action(*this, level, { std::make_pair(style(colors::gray), "Debug: "), std::make_pair(style(), "") });
        case note:
            return action(*this, level, { std::make_pair(style(colors::gray, colors::def, styles::bold), "Note: ") });
        case info:
            return action(*this, level, { std::make_pair(style(colors::gray, colors::def, styles::bold), "Info: ") });
        case success:
            return action(*this, level, { std::make_pair(style(colors::green, colors::def, styles::bold), "Success: "),
                std::make_pair(style(colors::bgray, colors::def, styles::bold), "") });
        case warning:
            return action(*this, level, { std::make_pair(style(colors::bbrown, colors::def, styles::bold), "Warning: "),
                std::make_pair(style(colors::bgray, colors::def, styles::bold), "") });
        case syntax:
            return action(*this, level, { std::make_pair(style(colors::bred, colors::def, styles::bold), "Syntax error: "),
                std::make_pair(style(colors::bgray, colors::def, styles::bold), "") });
        case error:
            return action(*this, level, { std::make_pair(style(colors::bred, colors::def, styles::bold), "Error: "),
                std::make_pair(style(colors::bgray, colors::def, styles::bold), "") });
        case fatal:
            return action(*this, level, { std::make_pair(style(colors::bred, colors::def, styles::bold), "Fatal error: "),
                std::make_pair(style(colors::bgray, colors::def, styles::bold), "") });
        case crash:
            return action(*this, level, { std::make_pair(style(colors::bred, colors::def, styles::bold), "Internal error: "),
                std::make_pair(style(colors::bgray, colors::def, styles::bold), "") });
        case always:
            return action(*this, level);
    }
}

std::vector<reaver::logger::__v1::stream_wrapper> & reaver::logger::__v1::logger_friend::_streams(logger & l)
{
    return l._streams;
}

void reaver::logger::__v1::logger_friend::_async(logger & l, std::function<void()> f)
{
    l._async(f);
}

reaver::logger::__v1::level reaver::logger::__v1::logger_friend::_level(logger & l)
{
    return l._level;
}
