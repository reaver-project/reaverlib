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
#include <unordered_map>
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
        thread_pool_closed() : exception{ crash }
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
                _spawn();
            }
        }

        ~thread_pool()
        {
            _end = true;
            _cond.notify_all();

            for (auto & th : _threads)
            {
                th.second.join();
            }
        }

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
            return _size;
        }

        void resize(std::size_t new_size)
        {
            std::unique_lock<std::mutex> lock{ _lock };

            if (new_size == _size)
            {
                return;
            }

            if (new_size > _size)
            {
                while (_size++ < new_size)
                {
                    _spawn();
                }
            }

            else
            {
                _die_semaphore.notify(_size - new_size);
                _size = new_size;
            }
        }

    private:
        void _loop()
        {
            while (!_end || _threads.size())
            {
                if (_die_semaphore.try_wait())
                {
                    auto this_thread_id = std::this_thread::get_id();
                    push([this_thread_id, this]() mutable {
                        _threads.erase(_threads.find(this_thread_id));
                    });

                    return;
                }

                std::function<void ()> f;

                {
                    std::unique_lock<std::mutex> lock{ _lock };

                    while (!_end && _queue.empty())
                    {
                        _cond.wait(lock);
                    }

                    if (_end && _queue.empty())
                    {
                        return;
                    }

                    f = std::move(_queue.front());
                    _queue.pop();
                }

                f();
            }
        }

        void _spawn()
        {
            std::thread t{ &thread_pool::_loop, this };
            _threads.emplace(std::make_pair(t.get_id(), std::move(t)));
        }

        std::atomic<std::size_t> _size;

        std::unordered_map<std::thread::id, std::thread> _threads;
        std::queue<std::function<void()>> _queue;

        std::condition_variable _cond;
        std::mutex _lock;

        semaphore _die_semaphore;

        std::atomic<bool> _end;
    };
}
