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

#include <reaver/mayfly.h>

#include <future>
#include <optional>
#include <queue>
#include <variant>

namespace test
{
#include "future_get.h"
}

MAYFLY_BEGIN_SUITE("future get");

MAYFLY_ADD_TESTCASE("ready get", [] {
    auto ready_future = test::reaver::make_ready_future(1);
    MAYFLY_REQUIRE(test::reaver::get(ready_future) == 1);
});

MAYFLY_ADD_TESTCASE("exceptional get", [] {
    auto exceptional_future = test::reaver::make_exceptional_future<int>(5);
    MAYFLY_REQUIRE_THROWS_TYPE(int, test::reaver::get(exceptional_future));
});

MAYFLY_ADD_TESTCASE("packaged get", [] {
    auto pair = test::reaver::package([] { return 123; });
    std::thread{ [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pair.packaged_task();
    } }
        .detach();
    MAYFLY_REQUIRE(test::reaver::get(pair.future) == 123);
});

MAYFLY_ADD_TESTCASE("exceptional packaged get", [] {
    auto exceptional_pair = test::reaver::package([] { throw 654; });
    std::thread{ [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        exceptional_pair.packaged_task();
    } }
        .detach();
    MAYFLY_REQUIRE_THROWS_TYPE(int, test::reaver::get(exceptional_pair.future));
});

MAYFLY_END_SUITE;
