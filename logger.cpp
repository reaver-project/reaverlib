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

reaver::logger::logger reaver::logger::log;

reaver::logger::logger::logger(reaver::logger::level level) : _level(level), _worker{[=]()
    {
        while (!_quit)
        {
            std::function<void()> f;

            {
                std::lock_guard<std::mutex> lock(_queue_mutex);
                f = _queue.back();
                _queue.pop();
            }

            f();
        }
    }}, _streams{std::cout}
{
}

reaver::logger::logger::~logger()
{
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        _queue.push([=](){ _quit = true; });
    }

    _worker.join();
}

reaver::logger::action::~action()
{
    if (_level < _logger._level)
    {
        return;
    }

    for (auto & stream : _logger._streams)
    {
        std::lock_guard<reaver::logger::stream_wrapper> _lock(stream);

        for (auto & x : _strings)
        {
            stream << x.first;
            stream << x.second;
        }
    }
}

reaver::logger::stream_wrapper::stream_wrapper(std::ostream & stream) : _impl{new _detail::_stream_ref_wrapper{stream}}
{
}

reaver::logger::stream_wrapper::stream_wrapper(std::shared_ptr<std::fstream> & stream) : _impl{new _detail::_stream_shptr_wrapper{stream}}
{
}
