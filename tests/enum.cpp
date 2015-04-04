/**
 * Reaver Library Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

#include "enum.h"

MAYFLY_BEGIN_SUITE("reflected enum");

namespace
{
    reflected_enum(test,
        foo,
        bar,
        baz = 5,
        buzz = 1,
        fizz
    );

    reflected_enum(another,
        foo,
        bar,
        baz
    );
}

MAYFLY_ADD_TESTCASE("enum_values", []
{
    MAYFLY_REQUIRE(enum_values<test>() == std::vector<test>{ test::foo, test::bar, test::baz, test::buzz, test::fizz });
});

MAYFLY_ADD_TESTCASE("enum_strings", []
{
    MAYFLY_REQUIRE(enum_strings<test>() == std::vector<std::string>{ "foo", "bar", "baz", "buzz", "fizz" });
});

MAYFLY_ADD_TESTCASE("to_string", []
{
    MAYFLY_CHECK(to_string(test::foo) == "foo");
    MAYFLY_CHECK(to_string(test::bar) == "bar");
    MAYFLY_CHECK(to_string(test::baz) == "baz");
    MAYFLY_CHECK(to_string(test::buzz) == "bar");
    MAYFLY_CHECK(to_string(test::fizz) == "fizz");
});

MAYFLY_ADD_TESTCASE("from_string", []
{
    MAYFLY_CHECK(from_string<test>("foo") == test::foo);
    MAYFLY_CHECK(from_string<test>("bar") == test::bar);
    MAYFLY_CHECK(from_string<test>("baz") == test::baz);
    MAYFLY_CHECK(from_string<test>("buzz") == test::buzz);
    MAYFLY_CHECK(from_string<test>("fizz") == test::fizz);
});

// this is a sanity test to check for potential problems with inlineness and multiple definitions and such
MAYFLY_ADD_TESTCASE("another enum", []
{
    MAYFLY_REQUIRE(to_string(another::bar) == "bar");
    MAYFLY_REQUIRE(from_string<another>("foo") == another::foo);
});

MAYFLY_END_SUITE;

