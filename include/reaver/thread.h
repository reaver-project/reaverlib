/**
 * Reaver Library License
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <thread>

#include "exception.h"

namespace reaver { inline namespace _v1
{
    enum class joinable_thread_policies
    {
        join_on_destruction,
        detach_on_destruction,
        throw_on_destruction
    };

    class joinable_thread_destructed : public exception
    {
    public:
        joinable_thread_destructed(std::thread::id id) : exception{ logger::crash }
        {
            *this << "attempted to destroy a thread object holding a joinable thread with id " << id;
        }
    };

    template<joinable_thread_policies Policy>
    class thread : public std::thread
    {
    public:
        using std::thread::thread;
        thread() = default;
        thread(thread &&) = default;
        thread & operator=(thread &&) = default;

        ~thread();
    };

    template<>
    inline thread<joinable_thread_policies::join_on_destruction>::~thread()
    {
        if (joinable())
        {
            join();
        }
    }

    template<>
    inline thread<joinable_thread_policies::detach_on_destruction>::~thread()
    {
        if (joinable())
        {
            detach();
        }
    }

    template<>
    inline thread<joinable_thread_policies::throw_on_destruction>::~thread()
    {
        if (joinable())
        {
            throw joinable_thread_destructed{ get_id() };
        }
    }

    using joining_thread = thread<joinable_thread_policies::join_on_destruction>;
    using detaching_thread = thread<joinable_thread_policies::detach_on_destruction>;
    using throwing_thread = thread<joinable_thread_policies::throw_on_destruction>;
}}
