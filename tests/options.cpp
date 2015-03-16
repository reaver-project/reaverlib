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

#include <boost/program_options.hpp>
#include <boost/range/algorithm.hpp>

namespace test
{
#   include "configuration/options.h"
}

namespace
{
    struct version : test::reaver::options::opt<version, void>
    {
        static constexpr const char * name = "version,v";
    };

    struct count : test::reaver::options::opt<count, int>
    {
        static constexpr const char * name = "count,c";
        static constexpr const char * description = "this option controls the count of some random thing";
    };
}

MAYFLY_BEGIN_SUITE("configuration");
MAYFLY_BEGIN_SUITE("options");

MAYFLY_ADD_TESTCASE("empty", []
{
    auto parsed = test::reaver::options::parse_argv<>(0, nullptr);
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<>>::value);
});

MAYFLY_ADD_TESTCASE("void", []
{
    const char * argv[] = { "", "--version" };
    auto parsed = test::reaver::options::parse_argv(2, argv, test::reaver::id<version>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<version>>::value);
    MAYFLY_REQUIRE(parsed.get<version>());
});

MAYFLY_ADD_TESTCASE("regular", []
{
    const char * argv[] = { "", "--count", "2" };
    auto parsed = test::reaver::options::parse_argv(3, argv, test::reaver::id<count>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<count>>::value);
    MAYFLY_REQUIRE(parsed.get<count>() == 2);
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;

