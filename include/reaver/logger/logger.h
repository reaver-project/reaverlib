/**
 * Reaver Library Licence
 *
 * Copyright © 2014-2015 Michał "Griwes" Dominiak
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

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#include "action.h"
#include "level_registry.h"
#include "../style.h"

namespace reaver
{
    namespace logger { inline namespace _v1
    {
        class logger
        {
        public:
            logger(base_level level = info) : _level{ level }, _worker{ [=]()
                {
                    while (!_quit)
                    {
                        std::vector<std::function<void()>> functions;

                        {
                            std::unique_lock<std::mutex> lock{ _lock };
                            if (_queue.empty())
                            {
                                _cv.wait(lock);
                            }
                            std::swap(functions, _queue);
                        }

                        for (auto && f : functions)
                        {
                            f();
                        }
                    }

                    for (auto & stream : _streams)
                    {
                        stream << style::style();
                    }
                } }, _streams{}
            {
            }

            logger(std::ostream & stream, base_level level = info) : logger{ level }
            {
                _streams.emplace_back(stream);
            }

            ~logger()
            {
                _async([=](){ _quit = true; });
                _worker.join();
            }

            void sync()
            {
                std::mutex mtx;
                std::condition_variable cond;

                _async([&](){ cond.notify_one(); });

                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock);
            }

            void add_stream(stream_wrapper stream)
            {
                _streams.push_back(std::move(stream));
            }

            void set_level(base_level l = info)
            {
                _level = l;
            }

            template<typename T = always_type>
            action<T> operator()(T = {})
            {
                return { *this, default_level_registry()[T{}] };
            }

            friend class logger_friend;

        private:
            void _async(std::function<void()> f)
            {
                std::lock_guard<std::mutex> lock{ _lock };
                _queue.push_back(std::move(f));

                _cv.notify_one();
            }

            std::atomic<bool> _quit{ false };

            base_level _level;

            std::condition_variable _cv;
            std::mutex _lock;
            std::thread _worker;
            std::vector<std::function<void()>> _queue;

            std::vector<stream_wrapper> _streams;
        };

        base_level logger_friend::_level(logger & l)
        {
            return l._level;
        }

        void logger_friend::_write(logger & l, std::vector<streamable> vec)
        {
            l._async([vec = std::move(vec), &l]()
            {
                for (auto & stream : l._streams)
                {
                    for (const auto & x : vec)
                    {
                        x.stream(stream);
                    }
                }
            });
        }

        inline logger & default_logger()
        {
            static logger def{ std::cout };
            return def;
        }

        template<typename Level = always_type>
        inline auto dlog(Level = {})
        {
            return default_logger()(Level{});
        }
    }}
}
