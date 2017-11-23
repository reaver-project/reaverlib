/**
 * Reaver Library Licence
 *
 * Copyright © 2015, 2017 Michał "Griwes" Dominiak
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

#include <unordered_set>
#include <reaver/error.h>
#include <reaver/exception.h>
#include <reaver/optional.h>

namespace test
{
#include "error.h"
}

#include <reaver/mayfly.h>

MAYFLY_BEGIN_SUITE("error engine");

MAYFLY_ADD_TESTCASE("reaching limit", []() {
    test::reaver::error_engine engine{ 2 };
    engine.push(test::reaver::exception{ test::reaver::logger::error });
    MAYFLY_CHECK_NOTHROW(engine.push(test::reaver::exception{ test::reaver::logger::warning }));
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::error_engine_exception, engine.push(test::reaver::exception{ test::reaver::logger::error }));
});

MAYFLY_ADD_TESTCASE("without reaching limit", []() {
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::error_engine_exception, []() {
        test::reaver::error_engine engine{ 2 };
        engine.push(test::reaver::exception{ test::reaver::logger::warning });
        MAYFLY_CHECK_NOTHROW(engine.push(test::reaver::exception{ test::reaver::logger::error }));

        engine.validate();
    }());
});

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpessimizing-move"

MAYFLY_ADD_TESTCASE("moved-from should not throw", []() {
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::error_engine_exception, []() {
        std::optional<test::reaver::error_engine> engine;
        MAYFLY_CHECK_NOTHROW(engine.emplace([]() {
            test::reaver::error_engine engine{ 2 };
            engine.push(test::reaver::exception{ test::reaver::logger::error });

            return std::move(engine);
        }()));

        engine->validate();
    }());
});

#pragma GCC diagnostic pop

MAYFLY_ADD_TESTCASE("reducing limit below current count", []() {
    test::reaver::error_engine engine{ 2 };
    engine.push(test::reaver::exception{ test::reaver::logger::error });
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::error_engine_exception, engine.set_error_limit(1));
});

MAYFLY_END_SUITE;
