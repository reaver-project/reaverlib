/**
 * Reaver Library Licence
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

#include <reaver/mayfly.h>

//#include <thread>
//#include <functional>
//#include <unordered_map>
//#include <map>
#include <queue>
//#include <atomic>
//#include <mutex>
#include <future>
//#include <type_traits>

#include <boost/functional/hash.hpp>

namespace test
{
#include "thread_pool.h"
}

MAYFLY_BEGIN_SUITE("thread pool");

MAYFLY_ADD_TESTCASE("running tasks", [] {
    {
        test::reaver::thread_pool pool{ 1 };
        auto future = pool.push([] { return 1; });
        MAYFLY_REQUIRE(future.get() == 1);
    }

    {
        test::reaver::thread_pool pool{ 1 };
        auto future1 = pool.push([] { return 1; });
        auto future2 = pool.push([] { return 2; });
        MAYFLY_REQUIRE(future1.get() == 1);
        MAYFLY_REQUIRE(future2.get() == 2);
    }

    {
        test::reaver::thread_pool pool{ 2 };
        auto future1 = pool.push([] { return 1; });
        auto future2 = pool.push([] { return 2; });
        MAYFLY_REQUIRE(future1.get() == 1);
        MAYFLY_REQUIRE(future2.get() == 2);
    }
});

MAYFLY_ADD_TESTCASE("handling aborted pools", [] {
    test::reaver::thread_pool pool{ 1 };
    pool.abort();
    MAYFLY_REQUIRE_THROWS_TYPE(test::reaver::thread_pool_closed, pool.push([] {}));
});

MAYFLY_ADD_TESTCASE("resizing", [] {
    {
        test::reaver::thread_pool pool{ 4 };
        pool.resize(2);
        while (pool.size() != 2)
        {
            std::this_thread::yield();
        }
        pool.resize(6);
        while (pool.size() != 6)
        {
            std::this_thread::yield();
        }
    }
});

MAYFLY_END_SUITE;
