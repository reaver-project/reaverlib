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

// due to prelude/functor.h; can probably purge the boost::variant fmap from there at some point
#include <boost/variant.hpp>

namespace test
{
#   include "future.h"
}

namespace
{
    struct trivial_executor : public test::reaver::executor
    {
        // TODO: reaver::function
        virtual void push(test::reaver::function<void ()> f) override
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
        MAYFLY_REQUIRE_THROWS_TYPE(int, pair.future.try_get());
    }

    {
        auto pair = test::reaver::package([]() -> int { throw 1; });

        MAYFLY_REQUIRE_NOTHROW(pair.packaged_task());
        MAYFLY_REQUIRE_THROWS_TYPE(int, pair.future.try_get());
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
    }

    {
        reaver::optional<test::reaver::future<int>> future;

        {
            auto pair = test::reaver::package([]() -> int { return 1; });
            future = std::move(pair.future);
        }

        MAYFLY_REQUIRE_THROWS_TYPE(test::reaver::broken_promise, future->try_get());
    }
});

MAYFLY_BEGIN_SUITE("continuations");

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

MAYFLY_ADD_TESTCASE("exceptional continuation", []()
{
    {
        auto pair = test::reaver::package([](){ throw 1; });
        auto future = pair.future.then([](){ return false; });
        auto exceptional = pair.future.on_error([](std::exception_ptr ex){ return true; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE_THROWS_TYPE(int, future.try_get());
        MAYFLY_REQUIRE(exceptional.try_get() == true);
    }

    {
        auto pair = test::reaver::package([]() -> int { throw 1; });
        auto future = pair.future.then([](int){ return false; });
        auto exceptional = pair.future.on_error([](std::exception_ptr ex){ return true; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE_THROWS_TYPE(int, future.try_get());
        MAYFLY_REQUIRE(exceptional.try_get() == true);
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

MAYFLY_ADD_TESTCASE("then keeps the state alive", []()
{
    {
        bool invoked = false;

        auto pair = test::reaver::package([&](){ invoked = true; });
        auto future = pair.future.then([](){});

        {
            auto moved = std::move(pair.future);
        }

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(invoked == true);
    }

    {
        bool invoked = false;

        auto pair = test::reaver::package([&](){ invoked = true; return 1; });
        auto future = pair.future.then([](auto){});

        {
            auto moved = std::move(pair.future);
        }

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(invoked == true);
    }
});

MAYFLY_ADD_TESTCASE("on_error keeps the state alive", []()
{
    {
        bool invoked = false;

        auto pair = test::reaver::package([&](){ invoked = true; });
        auto future = pair.future.on_error([](auto){});

        {
            auto moved = std::move(pair.future);
        }

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE(invoked == true);
    }

    {
        bool invoked = false;

        auto pair = test::reaver::package([&](){ invoked = true; return 1; });
        auto future = pair.future.on_error([](auto){});

        {
            auto moved = std::move(pair.future);
        }

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_REQUIRE(invoked == true);
    }
});

MAYFLY_ADD_TESTCASE("shared future", []()
{
    {
        auto future = test::reaver::make_ready_future(1);
        auto copy1 = future;
        auto copy2 = copy1;

        MAYFLY_CHECK(future.try_get() == 1);
        MAYFLY_CHECK(copy1.try_get() == 1);
        MAYFLY_CHECK(copy2.try_get() == 1);
    }

    {
        auto pair = test::reaver::package([](){ return 1; });
        auto future = pair.future.then([](auto i){ return i * 2; });
        auto other = pair.future.then([](auto i){ return i * 3; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(pair.future.try_get() == 1);
        MAYFLY_CHECK(future.try_get() == 2);
        MAYFLY_CHECK(other.try_get() == 3);
    }

    {
        auto future = test::reaver::make_ready_future(1);

        {
            auto copy = future;
        }

        MAYFLY_CHECK(future.try_get() == 1);
        MAYFLY_CHECK(!future.try_get());
    }

    {
        auto future1 = test::reaver::make_ready_future(1);
        auto future2 = test::reaver::make_ready_future(2);

        auto copy = future1;
        copy = future2;

        MAYFLY_CHECK(future1.try_get() == 1);
        MAYFLY_CHECK(!future1.try_get());

        MAYFLY_CHECK(future2.try_get() == 2);
        MAYFLY_CHECK(copy.try_get() == 2);
    }
});

MAYFLY_ADD_TESTCASE("shared void future", []()
{
    {
        auto future = test::reaver::make_ready_future();
        auto copy1 = future;
        auto copy2 = copy1;

        MAYFLY_CHECK(future.try_get());
        MAYFLY_CHECK(copy1.try_get());
        MAYFLY_CHECK(copy2.try_get());
    }

    {
        auto pair = test::reaver::package([](){});
        auto future = pair.future.then([](){ return 2; });
        auto other = pair.future.then([](){ return 3; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(pair.future.try_get());
        MAYFLY_CHECK(future.try_get() == 2);
        MAYFLY_CHECK(other.try_get() == 3);
    }

    {
        auto future = test::reaver::make_ready_future();

        {
            auto copy = future;
        }

        MAYFLY_CHECK(future.try_get());
        MAYFLY_CHECK(!future.try_get());
    }

    {
        auto future1 = test::reaver::make_ready_future();
        auto future2 = test::reaver::make_ready_future();

        auto copy = future1;
        copy = future2;

        MAYFLY_CHECK(future1.try_get());
        MAYFLY_CHECK(!future1.try_get());

        MAYFLY_CHECK(future2.try_get());
        MAYFLY_CHECK(copy.try_get());
    }
});

MAYFLY_END_SUITE;

MAYFLY_ADD_TESTCASE("noncopyable value", []()
{
    struct noncopyable
    {
        int i = 0;

        noncopyable(int i) : i{ i }
        {
        }

        noncopyable(noncopyable &&) = default;
        noncopyable & operator=(noncopyable &&) = default;

        noncopyable(const noncopyable &) = delete;
        noncopyable & operator=(const noncopyable &) = delete;
    };

    {
        auto ready = test::reaver::make_ready_future(noncopyable{ 1 });

        MAYFLY_CHECK(ready.try_get()->i == 1);
        MAYFLY_CHECK(!ready.try_get());
    }

    {
        auto pair = test::reaver::package([](){ return noncopyable{ 3 }; });
        auto future = pair.future.then([](noncopyable n){ return n.i; });

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(future.try_get() == 3);
        MAYFLY_CHECK(!pair.future.try_get());
    }

    {
        auto pair = test::reaver::package([](){ return noncopyable{ 3 }; });
        pair.future.then([](auto){});
        MAYFLY_CHECK_THROWS_TYPE(test::reaver::multiple_noncopyable_continuations, pair.future.then([](auto){}));
    }
});

MAYFLY_BEGIN_SUITE("when_all");

MAYFLY_ADD_TESTCASE("when_all", []()
{
    {
        //auto ready = test::reaver::make_ready_future();
        //auto ready_int = test::reaver::make_ready_future(1);

        auto pair = test::reaver::package([](){});
        auto pair_int = test::reaver::package([](){ return 2; });

        auto final = test::reaver::when_all(/*ready, ready_int,*/ pair.future, pair_int.future);

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });
        exec->push([exec, task = std::move(pair_int.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(final.try_get() == std::make_tuple(/*1,*/ 2));
    }

    {
        auto pair1 = test::reaver::package([](){ return 1; });
        auto pair2 = test::reaver::package([](){ return 2; });

        auto final = test::reaver::when_all(pair1.future, pair2.future);

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair1.packaged_task)](){ task(exec); });
        exec->push([exec, task = std::move(pair2.packaged_task)](){ task(exec); });

        MAYFLY_CHECK(final.try_get() == std::make_tuple(1, 2));
    }

    {
        auto f = test::reaver::when_all();
        MAYFLY_CHECK(f.try_get());
    }
});

MAYFLY_ADD_TESTCASE("exception propagation", []()
{
    {
        auto pair = test::reaver::package([](){ throw 1; });

        auto final = test::reaver::when_all(std::move(pair.future));

        auto exec = test::reaver::make_executor<trivial_executor>();
        exec->push([exec, task = std::move(pair.packaged_task)](){ task(exec); });

        MAYFLY_CHECK_THROWS_TYPE(test::reaver::exception_list, final.try_get());
    }
});

MAYFLY_END_SUITE;

MAYFLY_END_SUITE;

