/**
 * Reaver Library Licence
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

#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace reaver
{
    class semaphore
    {
    public:
        semaphore() : _count()
        {
        }

        void notify()
        {
            std::unique_lock<std::mutex> lock{_mutex};

            ++_count;

            _condition.notify_one();
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock{_mutex};

            while (!_count)
            {
                _condition.wait(lock);
            }

            --_count;
        }

    private:
        std::mutex _mutex;
        std::condition_variable _condition;
        std::atomic<uint64_t> _count;
    };
}
