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

#include <boost/functional/hash.hpp>

namespace test
{
#include "configuration.h"
}

namespace
{
// storing data
struct simple_tag
{
    using type = int;
};

struct another_tag
{
    using type = int;
};

// automatic conversions
struct automatically_constructing_tag
{
    using type = bool;
};

// object construction
struct explicitly_constructing_tag
{
    using type = std::string;

    static type construct(int i)
    {
        return std::to_string(i);
    }

    static type construct(bool b)
    {
        return b ? "true" : "false";
    }
};

struct identity_constructing_tag
{
    using type = std::string;

    static type construct(std::string s)
    {
        s += " buzz";
        return s;
    }
};

struct naughty_tag
{
    using type = bool;

    template<typename... Args>
    static int construct(Args &&...)
    {
        return 0;
    }
};

struct tag_with_default
{
    using type = int;
    static constexpr int default_value = 1;
};
}

MAYFLY_BEGIN_SUITE("configuration");

MAYFLY_ADD_TESTCASE("storing data", [] {
    test::reaver::configuration config;

    config.set<simple_tag>(1);
    MAYFLY_CHECK(config.get<simple_tag>() == 1);

    config.set<simple_tag>(2);
    MAYFLY_CHECK(config.get<simple_tag>() == 2);

    config.set<another_tag>(3);
    MAYFLY_CHECK(config.get<simple_tag>() == 2);
    MAYFLY_CHECK(config.get<another_tag>() == 3);
});

MAYFLY_ADD_TESTCASE("automatic conversions", [] {
    test::reaver::configuration config;

    config.set<automatically_constructing_tag>(1);
    MAYFLY_CHECK(config.get<automatically_constructing_tag>() == true);

    config.set<automatically_constructing_tag>(0.0);
    MAYFLY_CHECK(config.get<automatically_constructing_tag>() == false);

    config.set<automatically_constructing_tag>(nullptr);
    MAYFLY_CHECK(config.get<automatically_constructing_tag>() == false);
});

MAYFLY_ADD_TESTCASE("object construction", [] {
    test::reaver::configuration config;

    config.set<explicitly_constructing_tag>(std::string{ "foobar" });
    MAYFLY_CHECK(config.get<explicitly_constructing_tag>() == "foobar");

    config.set<explicitly_constructing_tag>("bazbuzz");
    MAYFLY_CHECK(config.get<explicitly_constructing_tag>() == "bazbuzz");

    config.set<explicitly_constructing_tag>(1234);
    MAYFLY_CHECK(config.get<explicitly_constructing_tag>() == "1234");

    config.set<explicitly_constructing_tag>(false);
    MAYFLY_CHECK(config.get<explicitly_constructing_tag>() == "false");

    config.set<identity_constructing_tag>(std::string{ "fizz" });
    MAYFLY_CHECK(config.get<identity_constructing_tag>() == "fizz buzz");

    config.set(identity_constructing_tag{}, std::string{ "fizz" });
    MAYFLY_CHECK(config.get(identity_constructing_tag{}) == "fizz buzz");
});

MAYFLY_ADD_TESTCASE("naughty tag", [] {
    test::reaver::configuration config;

    config.set<naughty_tag>(false);
    MAYFLY_REQUIRE_NOTHROW(config.get<naughty_tag>());

    config.set<naughty_tag>(1);
    MAYFLY_REQUIRE_NOTHROW(config.get<naughty_tag>());

    config.set<naughty_tag>();
    MAYFLY_REQUIRE_NOTHROW(config.get<naughty_tag>());
});

MAYFLY_ADD_TESTCASE("tag with default value", [] {
    test::reaver::configuration config;

    MAYFLY_REQUIRE(config.get<tag_with_default>() == tag_with_default::default_value);

    config.set<tag_with_default>(tag_with_default::default_value + 1);
    MAYFLY_REQUIRE(config.get<tag_with_default>() == tag_with_default::default_value + 1);
});

MAYFLY_BEGIN_SUITE("bound");

namespace
{
template<typename Tag,
    typename Config,
    typename std::enable_if<std::is_void<decltype(std::declval<Config>().template set<Tag>(std::declval<typename Tag::type>()), void())>::value, int>::type = 0>
bool check_invalid_set(test::reaver::choice<0>)
{
    return false;
}

template<typename Tag, typename Config>
bool check_invalid_set(test::reaver::choice<1>)
{
    return true;
}

template<typename Tag,
    typename Config,
    typename std::enable_if<std::is_void<decltype(std::declval<Config>().template get<Tag>(std::declval<typename Tag::type>()), void())>::value, int>::type = 0>
bool check_invalid_get(test::reaver::choice<0>)
{
    return false;
}

template<typename Tag, typename Config>
bool check_invalid_get(test::reaver::choice<1>)
{
    return true;
}

template<typename Tag,
    typename Config,
    typename std::enable_if<std::is_void<decltype(std::declval<Config>().template add<Tag>(std::declval<typename Tag::type>()), void())>::value, int>::type = 0>
bool check_invalid_add(test::reaver::choice<0>)
{
    return false;
}

template<typename Tag, typename Config>
bool check_invalid_add(test::reaver::choice<1>)
{
    return true;
}
}

MAYFLY_ADD_TESTCASE("invalid set", [] {
    MAYFLY_REQUIRE(check_invalid_set<simple_tag, test::reaver::bound_configuration<>>(test::reaver::select_overload{}));
    MAYFLY_REQUIRE(check_invalid_set<another_tag, test::reaver::bound_configuration<simple_tag>>(test::reaver::select_overload{}));
});

MAYFLY_ADD_TESTCASE("invalid get", [] {
    MAYFLY_REQUIRE(check_invalid_get<simple_tag, test::reaver::bound_configuration<>>(test::reaver::select_overload{}));
    MAYFLY_REQUIRE(check_invalid_get<another_tag, test::reaver::bound_configuration<simple_tag>>(test::reaver::select_overload{}));
});

MAYFLY_ADD_TESTCASE("add to bound configuration", [] {
    test::reaver::bound_configuration<> config;
    auto extended_config = config.add<simple_tag>(1);

    MAYFLY_REQUIRE(std::is_same<decltype(extended_config), test::reaver::bound_configuration<simple_tag>>::value);
    MAYFLY_REQUIRE(extended_config.get<simple_tag>() == 1);

    auto more_extended = extended_config.add<another_tag>(2);
    MAYFLY_REQUIRE(std::is_same<decltype(more_extended), test::reaver::bound_configuration<simple_tag, another_tag>>::value);
    MAYFLY_REQUIRE(more_extended.get<simple_tag>() == 1);
    MAYFLY_REQUIRE(more_extended.get<another_tag>() == 2);
});

MAYFLY_ADD_TESTCASE("invalid add",
    [] { MAYFLY_REQUIRE(check_invalid_add<simple_tag, test::reaver::bound_configuration<simple_tag>>(test::reaver::select_overload{})); });

MAYFLY_ADD_TESTCASE("construct from unbound", [] {
    test::reaver::configuration config;
    MAYFLY_REQUIRE_NOTHROW(test::reaver::bound_configuration<> bound = config);

    config.set<simple_tag>(1);
    MAYFLY_REQUIRE_NOTHROW(test::reaver::bound_configuration<simple_tag> bound = config);
    MAYFLY_REQUIRE_THROWS_TYPE(std::out_of_range, test::reaver::bound_configuration<simple_tag, another_tag> bound = config);
});

MAYFLY_ADD_TESTCASE("construct from bound", [] {
    test::reaver::bound_configuration<> empty;
    auto one = empty.add<simple_tag>(1);
    auto two = one.add<another_tag>(2);

    test::reaver::bound_configuration<simple_tag> simple = two;
    MAYFLY_CHECK(simple.get<simple_tag>() == 1);

    test::reaver::bound_configuration<another_tag> another = two;
    MAYFLY_CHECK(another.get<another_tag>() == 2);

    test::reaver::bound_configuration<> other_empty = two;
});

namespace
{
template<typename BoundType,
    typename Argument,
    typename std::enable_if<std::is_void<decltype(BoundType{ std::declval<Argument>() }, void())>::value, int>::type = 0>
bool check_invalid_construct(test::reaver::choice<0>)
{
    return false;
}

template<typename...>
bool check_invalid_construct(test::reaver::choice<1>)
{
    return true;
}
}

MAYFLY_ADD_TESTCASE("invalid construct from bound", [] {
    test::reaver::bound_configuration<> empty;
    auto one = empty.add<simple_tag>(1);

    MAYFLY_REQUIRE(check_invalid_construct<test::reaver::bound_configuration<another_tag>, decltype(one)>(test::reaver::select_overload{}));
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
