/**
 * Reaver Library Licence
 *
 * Copyright © 2015, 2017-2018 Michał "Griwes" Dominiak
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

#include <optional>

#include <boost/functional/hash.hpp>
#include <boost/program_options.hpp>

namespace test
{
#include "configuration/options.h"
}

namespace
{
struct version : test::reaver::options::opt<version, void>
{
    static constexpr const char * name = "version,v";
};

struct help : test::reaver::options::opt<help, void>
{
    static constexpr const char * name = "help,h";
};

struct other_void : test::reaver::options::opt<other_void, void>
{
    static constexpr const char * name = "other-void";
};

struct count : test::reaver::options::opt<count, int>
{
    static constexpr const char * name = "count,c";
    static constexpr const char * description = "this option controls the count of some random thing";
};

struct output : test::reaver::options::opt<output, std::string>
{
    static constexpr const char * name = "output,o";
};

struct command : test::reaver::options::opt<command, std::string>
{
    static constexpr test::reaver::options::option_set options = { test::reaver::options::positional };
};

struct value : test::reaver::options::opt<value, std::size_t>
{
    static constexpr test::reaver::options::option_set options = { test::reaver::options::positional(1) };
};

struct optional : test::reaver::options::opt<optional, std::optional<int>>
{
    static constexpr const char * name = "optional,o";
};

struct path : test::reaver::options::opt<path, std::vector<std::string>>
{
    static constexpr const char * name = "path";
};
}

MAYFLY_BEGIN_SUITE("configuration");
MAYFLY_BEGIN_SUITE("options");

MAYFLY_ADD_TESTCASE("empty", [] {
    const char * argv[] = { "" };
    auto parsed = test::reaver::options::parse_argv<>(1, argv);
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<>>::value);
});

MAYFLY_ADD_TESTCASE("void", [] {
    const char * argv[] = { "", "--version" };
    auto parsed = test::reaver::options::parse_argv(2, argv, test::reaver::id<version>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<version>>::value);
    MAYFLY_REQUIRE(parsed.get<version>());
});

MAYFLY_ADD_TESTCASE("regular", [] {
    const char * argv[] = { "", "--count", "2" };
    auto parsed = test::reaver::options::parse_argv(3, argv, test::reaver::id<count>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<count>>::value);
    MAYFLY_REQUIRE(parsed.get<count>() == 2);
});

MAYFLY_ADD_TESTCASE("multiple regular", [] {
    const char * argv[] = { "", "--count", "2", "--output", "foo" };
    auto parsed = test::reaver::options::parse_argv(5, argv, test::reaver::id<count>{}, test::reaver::id<output>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<count, output>>::value);
    MAYFLY_REQUIRE(parsed.get<count>() == 2);
    MAYFLY_REQUIRE(parsed.get<output>() == "foo");
});

MAYFLY_ADD_TESTCASE("multiple void", [] {
    const char * argv[] = { "", "--version", "--help" };
    auto parsed = test::reaver::options::parse_argv(3, argv, test::reaver::id<version>{}, test::reaver::id<help>{}, test::reaver::id<other_void>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<version, help, other_void>>::value);
    MAYFLY_REQUIRE(parsed.get<version>());
    MAYFLY_REQUIRE(parsed.get<help>());
    MAYFLY_REQUIRE(!parsed.get<other_void>());
});

MAYFLY_ADD_TESTCASE("positional", [] {
    const char * argv[] = { "", "install" };
    auto parsed = test::reaver::options::parse_argv(2, argv, test::reaver::id<command>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<command>>::value);
    MAYFLY_REQUIRE(parsed.get<command>() == "install");
});

MAYFLY_ADD_TESTCASE("multiple positional", [] {
    const char * argv[] = { "", "upgrade", "123" };
    auto parsed = test::reaver::options::parse_argv(3, argv, test::reaver::id<value>{}, test::reaver::id<command>{});
    MAYFLY_REQUIRE(std::is_same<decltype(parsed), test::reaver::bound_configuration<value, command>>::value);
    MAYFLY_REQUIRE(parsed.get<command>() == "upgrade");
    MAYFLY_REQUIRE(parsed.get<value>() == 123);
});

// TODO: need checks for static asserts in positional comparator (checking for
// overlapping "regions"
// of positional arguments)

MAYFLY_ADD_TESTCASE("optional", [] {
    {
        const char * argv[] = { "", "--optional", "5" };
        auto parsed = test::reaver::options::parse_argv(3, argv, test::reaver::id<optional>{});
        MAYFLY_REQUIRE(parsed.get<optional>() == 5);
    }

    {
        const char * argv[] = { "" };
        auto parsed = test::reaver::options::parse_argv(1, argv, test::reaver::id<optional>{});
        MAYFLY_REQUIRE(!parsed.get<optional>());
    }
});

MAYFLY_ADD_TESTCASE("vector", [] {
    const char * argv[] = { "", "--path", "a", "--path", "b" };
    auto parsed = test::reaver::options::parse_argv(5, argv, test::reaver::id<path>{});
    MAYFLY_REQUIRE(parsed.get<path>() == std::vector<std::string>{ "a", "b" });
});

MAYFLY_ADD_TESTCASE("mixed arguments", [] {
    const char * argv[] = { "", "upgrade", "--version", "--count", "5", "456", "--output", "foo", "--help" };
    auto parsed = test::reaver::options::parse_argv(9,
        argv,
        test::reaver::id<command>{},
        test::reaver::id<value>{},
        test::reaver::id<help>{},
        test::reaver::id<version>{},
        test::reaver::id<output>{},
        test::reaver::id<count>{});
    MAYFLY_REQUIRE(parsed.get<command>() == "upgrade");
    MAYFLY_REQUIRE(parsed.get<help>());
    MAYFLY_REQUIRE(parsed.get<version>());
    MAYFLY_REQUIRE(parsed.get<output>() == "foo");
    MAYFLY_REQUIRE(parsed.get<value>() == 456);
    MAYFLY_REQUIRE(parsed.get<count>() == 5);
});

// TODO: tests regarding tag::parsed_type
// TODO: tests regarding tag::default_value

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
