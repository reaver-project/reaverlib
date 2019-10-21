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

#include "typeclass/hashable.h"

namespace
{
struct test_with_specialization
{
    std::size_t hash_value() const
    {
        MAYFLY_REQUIRE(!"should've preferred the std::hash specialization!");
        __builtin_unreachable();
    }
};

struct test_without_specialization
{
    std::size_t value;

    std::size_t hash_value() const
    {
        return value;
    }
};
}

namespace std
{
template<>
struct hash<test_with_specialization>
{
    std::size_t operator()(const test_with_specialization &) const
    {
        return 0;
    }
};
}

MAYFLY_BEGIN_SUITE("typeclass");
MAYFLY_BEGIN_SUITE("hashable");

MAYFLY_ADD_TESTCASE("string", [] {
    std::string s1 = "foo";
    std::string s2 = "foo";
    std::string s3 = "bar";

    MAYFLY_REQUIRE(reaver::hash(s1) == reaver::hash(s2));
    MAYFLY_REQUIRE(reaver::hash(s1) != reaver::hash(s3));
});

MAYFLY_ADD_TESTCASE("std::hash should win", [] {
    test_with_specialization t;
    MAYFLY_REQUIRE(reaver::hash(t) == 0);
});

MAYFLY_ADD_TESTCASE("hash_value", [] {
    test_without_specialization t{ 123 };
    MAYFLY_REQUIRE(reaver::hash(t) == 123);
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
