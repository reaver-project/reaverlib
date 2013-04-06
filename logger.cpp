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

#include "logger.h"

reaver::logger::logger reaver::logger::dlog{std::cout};

reaver::logger::logger::logger(reaver::logger::level level) : _level{level}, _worker{[=]()
    {
        while (!_quit)
        {
            _semaphore.wait();

            std::queue<std::function<void()>> q;

            {
                std::lock_guard<std::mutex> lock{_lock};
                std::swap(q, _queue);
            }

            while (q.size())
            {
                q.front()();
                q.pop();
            }
        }
    }}, _streams{}
{
}

reaver::logger::logger::logger(std::ostream & stream, reaver::logger::level level) : logger{level}
{
    _streams.push_back({stream});
}

reaver::logger::logger::~logger()
{
    _async([=](){ _quit = true; });
    _worker.join();
}

void reaver::logger::logger::add_stream(const reaver::logger::stream_wrapper & stream)
{
    _streams.push_back(stream);
}

void reaver::logger::logger::_async(std::function<void()> f)
{
    std::lock_guard<std::mutex> lock{_lock};

    _queue.push(f);

    _semaphore.notify();
}

reaver::logger::action::action(logger & log, reaver::logger::level level, const std::vector<std::pair<reaver::style::style, std::string>> & init)
    : _logger(log), _level(level), _strings(init)
{
}

reaver::logger::action::~action()
{
    if (_level < _logger._level)
    {
        return;
    }

    std::vector<std::pair<reaver::style::style, std::string>> strings = _strings;

    for (auto & stream : _logger._streams)
    {
        _logger._async([=]() mutable
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

reaver::logger::stream_wrapper::stream_wrapper(std::ostream & stream) : _impl{new _detail::_stream_ref_wrapper{stream}}
{
}

reaver::logger::stream_wrapper::stream_wrapper(const std::shared_ptr<std::fstream> & stream) : _impl{new _detail::_stream_shptr_wrapper{stream}}
{
}

reaver::logger::stream_wrapper::~stream_wrapper()
{
}

reaver::logger::stream_wrapper & reaver::logger::stream_wrapper::operator<<(const std::string & str)
{
    _impl->get() << str;
    return *this;
}

reaver::logger::stream_wrapper & reaver::logger::stream_wrapper::operator<<(const reaver::style::style & style)
{
    _impl->get() << style;
    return *this;
}

reaver::logger::action reaver::logger::logger::operator()(reaver::logger::level level)
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
        case info:
            return action(*this, level, { std::make_pair(style(), "Info: ") });
        case warning:
            return action(*this, level, { std::make_pair(style(colors::bbrown, colors::def, styles::bold), "Warning: "),
                std::make_pair(style(), "") });
        case error:
            return action(*this, level, { std::make_pair(style(colors::bred, colors::def, styles::bold), "Error: "),
                std::make_pair(style(), "") });
        case crash:
            return action(*this, level, { std::make_pair(style(colors::bred, colors::def, styles::bold), "Internal error: "),
                std::make_pair(style(), "") });
        case always:
            return action(*this, level);
    }
}
