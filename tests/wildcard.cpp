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

#include "wildcard.h"

MAYFLY_BEGIN_SUITE("wildcard");

MAYFLY_ADD_TESTCASE("star", [] {
    MAYFLY_REQUIRE(reaver::wildcard::match("foo*", "foo"));
    MAYFLY_REQUIRE(reaver::wildcard::match("f*oo", "foo"));
    MAYFLY_REQUIRE(reaver::wildcard::match("*o", "foo"));
    MAYFLY_REQUIRE(reaver::wildcard::match("a*b*c", "a  b xfdsc"));

    MAYFLY_REQUIRE(!reaver::wildcard::match("*f", "foo"));
});

MAYFLY_ADD_TESTCASE("question mark", [] {
    MAYFLY_REQUIRE(reaver::wildcard::match("fo?", "foo"));
    MAYFLY_REQUIRE(reaver::wildcard::match("?oo", "foo"));
    MAYFLY_REQUIRE(reaver::wildcard::match("f?o", "foo"));

    MAYFLY_REQUIRE(!reaver::wildcard::match("foo?", "foo"));
    MAYFLY_REQUIRE(!reaver::wildcard::match("?foo", "foo"));
});

MAYFLY_ADD_TESTCASE("star and question mark", [] {
    MAYFLY_REQUIRE(reaver::wildcard::match("foo?bar*", "foo bar"));
    MAYFLY_REQUIRE(reaver::wildcard::match("foo*?ar", "foo bar"));
    MAYFLY_REQUIRE(reaver::wildcard::match("fo???ar", "foo bar"));
    MAYFLY_REQUIRE(reaver::wildcard::match("fo?*?bar", "foo bar"));
    MAYFLY_REQUIRE(reaver::wildcard::match("fo?*?ar", "foo bar"));
});

MAYFLY_END_SUITE;
