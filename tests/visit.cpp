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

#include <boost/variant.hpp>

#include <reaver/mayfly.h>

namespace test
{
#   include "visit.h"
}

MAYFLY_BEGIN_SUITE("visit");

MAYFLY_ADD_TESTCASE("default id", []()
{
    MAYFLY_REQUIRE(test::reaver::make_visitor(test::reaver::default_id(), [](auto &&){ return "foo"; })(0) == std::string("foo"));
});

MAYFLY_ADD_TESTCASE("basic type specifications", []()
{
    auto visitor = test::reaver::make_visitor(
        test::reaver::id<int>(), [](auto &&){ return "foo"; },
        test::reaver::id<bool>(), [](auto &&){ return "bar"; },
        test::reaver::default_id(), [](auto &&){ return "baz"; }
    );

    MAYFLY_REQUIRE(visitor(1) == std::string("foo"));
    MAYFLY_REQUIRE(visitor(false) == std::string("bar"));
    MAYFLY_REQUIRE(visitor("abc") == std::string("baz"));
});

MAYFLY_ADD_TESTCASE("reference type specifications", []()
{
    auto visitor = test::reaver::make_visitor(
        test::reaver::id<int &>(), [](auto &&){ return 1; },
        test::reaver::id<int>(), [](auto &&){ return 2; }
    );

    int i;
    MAYFLY_REQUIRE(visitor(i) == 1);
    MAYFLY_REQUIRE(visitor(1) == 2);
});

MAYFLY_ADD_TESTCASE("cv type specifications", []()
{
    auto visitor = test::reaver::make_visitor(
        test::reaver::id<const volatile int>(), [](auto &&){ return 1; },
        test::reaver::id<const int>(), [](auto &&){ return 2; },
        test::reaver::id<volatile int>(), [](auto &&){ return 3; },
        test::reaver::id<int>(), [](auto &&){ return 4; }
    );

    const volatile int i = 0;
    const int j = 0;
    volatile int k = 0;
    int l = 0;

    MAYFLY_REQUIRE(visitor(i) == 1);
    MAYFLY_REQUIRE(visitor(j) == 2);
    MAYFLY_REQUIRE(visitor(k) == 3);
    MAYFLY_REQUIRE(visitor(l) == 4);
});

MAYFLY_ADD_TESTCASE("cvref type specifications", []()
{
    auto visitor = test::reaver::make_visitor(
        test::reaver::id<const int &>(), [](auto &&){ return 1; },
        test::reaver::id<int &>(), [](auto &&){ return 2; },
        test::reaver::id<int>(), [](auto &&){ return 3; }
    );

    const int i = 0;
    int j = 0;

    MAYFLY_REQUIRE(visitor(i) == 1);
    MAYFLY_REQUIRE(visitor(j) == 2);
    MAYFLY_REQUIRE(visitor(0) == 3);
});

MAYFLY_END_SUITE;
