/**
 * Reaver Library Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "thread_pool.h"

namespace reaver { inline namespace _v1
{
    // defaults to a thread pool with thread count of std::thread::hardware_concurrency()
    // the recommended way to make it be a thread pool with a different thread count is to create it manually and pass into this function
    // after the first call to default_executor, the default executor cannot be changed
    inline std::shared_ptr<executor> default_executor(std::shared_ptr<executor> custom = nullptr)
    {
        static auto de = [&]() -> std::shared_ptr<executor> {
            if (custom)
            {
                return std::move(custom);
            }

            return std::make_shared<thread_pool>(std::thread::hardware_concurrency());
        }();

        return de;
    }
}}

