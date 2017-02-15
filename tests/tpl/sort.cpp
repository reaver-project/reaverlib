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

namespace test
{
#include "tpl/sort.h"
}

namespace tpl = test::reaver::tpl;

MAYFLY_BEGIN_SUITE("template library");
MAYFLY_BEGIN_SUITE("sort");

namespace
{
template<std::uintmax_t Value>
struct base
{
    static constexpr std::size_t value = Value;
};

struct foo : base<1>
{
};
struct bar : base<2>
{
};
struct baz : base<3>
{
};

template<typename T, typename U>
struct less
{
    static constexpr const bool value = T::value < U::value;
};

template<typename T, typename U>
struct greater
{
    static constexpr const bool value = T::value > U::value;
};
};

MAYFLY_ADD_TESTCASE("empty vector", [] { MAYFLY_REQUIRE(std::is_same<tpl::vector<>, tpl::sort<tpl::vector<>, less>>::value); });

MAYFLY_ADD_TESTCASE("single element vector", [] { MAYFLY_REQUIRE(std::is_same<tpl::vector<foo>, tpl::sort<tpl::vector<foo>, less>>::value); });

MAYFLY_ADD_TESTCASE("repeating elements", [] {
    MAYFLY_REQUIRE(std::is_same<tpl::vector<foo, foo>, tpl::sort<tpl::vector<foo, foo>, less>>::value);
    MAYFLY_REQUIRE(std::is_same<tpl::vector<foo, foo, foo>, tpl::sort<tpl::vector<foo, foo, foo>, less>>::value);
    MAYFLY_REQUIRE(std::is_same<tpl::vector<foo, foo>, tpl::sort<tpl::vector<foo, foo>, greater>>::value);
});

MAYFLY_ADD_TESTCASE("unique elements", [] {
    MAYFLY_REQUIRE(std::is_same<tpl::vector<foo, bar, baz>, tpl::sort<tpl::vector<baz, foo, bar>, less>>::value);
    MAYFLY_REQUIRE(std::is_same<tpl::vector<baz, bar, foo>, tpl::sort<tpl::vector<foo, baz, bar>, greater>>::value);
});

MAYFLY_ADD_TESTCASE("sorted input", [] {
    MAYFLY_REQUIRE(std::is_same<tpl::vector<foo, bar, baz>, tpl::sort<tpl::vector<foo, bar, baz>, less>>::value);
    MAYFLY_REQUIRE(std::is_same<tpl::vector<baz, bar, foo>, tpl::sort<tpl::vector<baz, bar, foo>, greater>>::value);
    MAYFLY_REQUIRE(std::is_same<tpl::vector<foo, bar, bar, baz, baz>, tpl::sort<tpl::vector<foo, bar, bar, baz, baz>, less>>::value);
    MAYFLY_REQUIRE(std::is_same<tpl::vector<baz, baz, baz, foo>, tpl::sort<tpl::vector<baz, baz, baz, foo>, greater>>::value);
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
