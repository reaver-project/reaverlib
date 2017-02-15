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

#include "kwargs.h"

namespace
{
using namespace reaver::kwargs;

struct argx : kwarg<int>
{
    using kwarg::kwarg;
};

struct argy : kwarg<std::string>
{
    using kwarg::kwarg;
};

struct argz : kwarg<int>
{
    using kwarg::kwarg;
};
}

MAYFLY_BEGIN_SUITE("keyword arguments");

MAYFLY_ADD_TESTCASE("get", [] {
    MAYFLY_REQUIRE(reaver::kwargs::get<argx>(argx{ 1 }, argy{ "foo" }, argz{ 2 }) == 1);
    MAYFLY_REQUIRE(reaver::kwargs::get<argy>(argx{ 1 }, argy{ "foo" }, argz{ 2 }) == "foo");
    MAYFLY_REQUIRE(reaver::kwargs::get<argz>(argx{ 1 }, argy{ "foo" }, argz{ 2 }) == 2);
});

MAYFLY_ADD_TESTCASE("get_or", [] {
    MAYFLY_REQUIRE(reaver::kwargs::get_or<argx>(0, argx{ 1 }, argy{ "foo" }, argz{ 2 }) == 1);
    MAYFLY_REQUIRE(reaver::kwargs::get_or<argy>("", argx{ 1 }, argy{ "foo" }, argz{ 2 }) == "foo");
    MAYFLY_REQUIRE(reaver::kwargs::get_or<argz>(0, argx{ 1 }, argy{ "foo" }, argz{ 2 }) == 2);
    MAYFLY_REQUIRE(reaver::kwargs::get_or<argx>(0, argy{ "foo" }, argz{ 2 }) == 0);
});

MAYFLY_ADD_TESTCASE("get_nth", [] {
    MAYFLY_REQUIRE(reaver::kwargs::get_nth<argx>(0, argx{ 1 }, argx{ 2 }, argy{ "foo" }, argx{ 3 }) == 1);
    MAYFLY_REQUIRE(reaver::kwargs::get_nth<argx>(1, argx{ 1 }, argx{ 2 }, argy{ "foo" }, argx{ 3 }) == 2);
    MAYFLY_REQUIRE(reaver::kwargs::get_nth<argx>(2, argx{ 1 }, argx{ 2 }, argy{ "foo" }, argx{ 3 }) == 3);
    MAYFLY_REQUIRE_THROWS_TYPE(std::out_of_range, reaver::kwargs::get_nth<argx>(1, argx{ 1 }));
});

MAYFLY_ADD_TESTCASE("get_all", [] {
    MAYFLY_REQUIRE(reaver::kwargs::get_all<argx>(argx{ 1 }, argx{ 2 }, argz{ 3 }, argy{ "foo" }, argx{ 4 }) == std::array<int, 3>{ { 1, 2, 4 } });
});

MAYFLY_END_SUITE;
