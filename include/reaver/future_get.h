/**
 * Reaver Library Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "future.h"
#include "semaphore.h"

namespace reaver
{
inline namespace _v1
{
    template<typename T>
    auto get(future<T> fut)
    {
        semaphore sem;

        // ...gcc doesn't work with an unnamed pack here
        auto callback = [&](auto &&... args) {
            swallow{ args... };
            sem.notify();
        };
        fut.then(callback).on_error(callback).detach();

        sem.wait();

        return std::move(fut.try_get().value());
    }
}
}
