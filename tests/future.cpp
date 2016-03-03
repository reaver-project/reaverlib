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

#include <reaver/mayfly.h>

#include <reaver/optional.h>

namespace test
{
#   include "future.h"
}

namespace
{
    struct trivial_executor : public test::reaver::executor
    {
        // TODO: reaver::function
        virtual void push(std::function<void ()> f) override
        {
            f();
        }
    };
}

MAYFLY_BEGIN_SUITE("future");

MAYFLY_ADD_TESTCASE("try_get", []()
{
    auto f = test::reaver::make_ready_future(1);

    MAYFLY_REQUIRE(f.try_get() == 1);
    MAYFLY_REQUIRE(!f.try_get());
});

MAYFLY_ADD_TESTCASE("shared state", []()
{
    auto pair = test::reaver::package([](){ return 1; });
    pair.packaged_task();

    MAYFLY_REQUIRE(pair.future.try_get() == 1);
});

MAYFLY_ADD_TESTCASE("void future", []()
{
    auto pair = test::reaver::package([](){});
    MAYFLY_REQUIRE(!pair.future.try_get());

    pair.packaged_task();
    MAYFLY_REQUIRE(pair.future.try_get());
});

MAYFLY_ADD_TESTCASE("exceptional future", []()
{
    {
        auto pair = test::reaver::package([](){ throw 1; });

        MAYFLY_REQUIRE_NOTHROW(pair.packaged_task());
        MAYFLY_REQUIRE_THROWS_TYPE(int, pair.future.try_get());
    }

    {
        auto pair = test::reaver::package([]() -> int { throw 1; });

        MAYFLY_REQUIRE_NOTHROW(pair.packaged_task());
        MAYFLY_REQUIRE_THROWS_TYPE(int, pair.future.try_get());
    }
});

MAYFLY_ADD_TESTCASE("broken promise", []()
{
    {
        reaver::optional<test::reaver::future<>> future;

        {
            auto pair = test::reaver::package([](){});
            future = std::move(pair.future);
        }

        MAYFLY_REQUIRE_THROWS_TYPE(test::reaver::broken_promise, future->try_get());
        MAYFLY_REQUIRE_NOTHROW(future->try_get());
    }

    {
        reaver::optional<test::reaver::future<int>> future;

        {
            auto pair = test::reaver::package([]() -> int { return 1; });
            future = std::move(pair.future);
        }

        MAYFLY_REQUIRE_THROWS_TYPE(test::reaver::broken_promise, future->try_get());
        MAYFLY_REQUIRE_NOTHROW(future->try_get());
    }
});

MAYFLY_ADD_TESTCASE("then", []()
{
    {
        auto pair = test::reaver::package([](){ return; });
        auto future = pair.future.then([](){ return 2; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE(future.try_get() == 2);
    }

    {
        auto pair = test::reaver::package([](){ return 1; });
        auto future = pair.future.then([](auto i){ return i * 2; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE(future.try_get() == 2);
    }
});

MAYFLY_ADD_TESTCASE("exception propagation", []()
{
    {
        auto pair = test::reaver::package([](){ throw 1; });
        auto future = pair.future.then([](){ return 2; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE_THROWS_TYPE(int, future.try_get());
    }

    {
        auto pair = test::reaver::package([]() -> int { throw 1; });
        auto future = pair.future.then([](auto i){ return 2; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE_THROWS_TYPE(int, future.try_get());
    }
});

MAYFLY_END_SUITE;

