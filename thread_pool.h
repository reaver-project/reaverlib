/**
 * Reaver Library License
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

#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <future>
#include <type_traits>

#include "exception.h"

namespace reaver
{
    class thread_pool_closed : public exception
    {
    public:
        thread_pool_closed() : exception{ error }
        {
            *this << "tried to insert a task into an already closed thread pool.";
        }
    };

    class thread_pool
    {
    public:
        thread_pool(std::size_t size)
        {
            while (size--)
            {
                _threads.emplace_back([this](){
                    while (_end || _threads.size())
                    {
                        std::function<void ()> f;

                        {
                            std::unique_lock<std::mutex> lock{ _lock };

                            while (_end && _queue.empty())
                            {
                                _cond.wait(lock);
                            }

                            f = std::move(_queue.front());
                            _queue.pop();
                        }

                        f();
                    }
                });
            }
        }

        ~thread_pool();

        template<typename F, typename... Args>
        std::future<typename std::result_of<F (Args...)>::type> push(F && f, Args &&... args)
        {
            if (_end)
            {
                throw thread_pool_closed{};
            }

            auto task = std::make_shared<std::packaged_task<typename std::result_of<F (Args...)>::type ()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            auto future = task->get_future();

            {
                std::unique_lock<std::mutex> lock{ _lock };
                _queue.emplace([task]{ (*task)(); });
            }

            _cond.notify_one();

            return future;
        }

        std::size_t size() const
        {
            return _threads.size();
        }

        void resize(std::size_t);

    private:
        std::vector<std::thread> _threads;
        std::queue<std::function<void()>> _queue;

        std::condition_variable _cond;
        std::mutex _lock;

        std::atomic<bool> _end;
    };
}
