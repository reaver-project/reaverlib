/**
 * Reaver Library Licence
 *
 * Copyright © 2015-2016 Michał "Griwes" Dominiak
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

#include <reaver/exception.h>

namespace test
{
#include "optional.h"
}

#include <reaver/mayfly.h>

MAYFLY_BEGIN_SUITE("optional");

MAYFLY_ADD_TESTCASE("construction and comparisons", []() {
    {
        test::reaver::optional<int> o = test::reaver::none;
        MAYFLY_CHECK(!o);
    }

    {
        test::reaver::optional<int> o = 1;
        MAYFLY_CHECK(o == 1);
    }

    {
        test::reaver::optional<int> o = 1;
        test::reaver::optional<int> p = 2;
        test::reaver::optional<float> r = 1.f;

        MAYFLY_CHECK(o != p);
        MAYFLY_CHECK(o == r);
        MAYFLY_CHECK(1.f == r);
    }
});

MAYFLY_ADD_TESTCASE("references", []() {
    test::reaver::optional<int &> o;
    MAYFLY_CHECK(!o);

    int i = 1;
    o = i;
    MAYFLY_CHECK(o == 1);

    i = 2;
    MAYFLY_CHECK(o == 2);

    int j = 123;
    o = j;
    MAYFLY_CHECK(o == 123);
});

MAYFLY_ADD_TESTCASE("fmap", []() {
    MAYFLY_CHECK(test::reaver::fmap(test::reaver::make_optional(1), [](auto a) { return std::to_string(a); }) == "1");
    MAYFLY_CHECK(!test::reaver::fmap(test::reaver::optional<int>{}, [](auto...) { return reaver::unit{}; }));
});

MAYFLY_ADD_TESTCASE("variant of optionals", []() {
    test::reaver::variant<test::reaver::optional<int>, test::reaver::optional<float>> v1 = 1;
    test::reaver::variant<test::reaver::optional<int>, test::reaver::optional<float>> v2 = 2.f;

    MAYFLY_CHECK(test::reaver::get<0>(v1) == 1);
    MAYFLY_CHECK(test::reaver::get<1>(v2) == 2.f);

    // the following doesn't currently work, *by design*
    // might make variant prefer constructors in the order of arguments some day
    // but that day is not today
    // test::reaver::variant<test::reaver::optional<int>,
    // test::reaver::optional<float>> v3 =
    // test::reaver::none;
});

MAYFLY_ADD_TESTCASE("optional variant", []() {
    auto v1 = test::reaver::optional<test::reaver::variant<int, float>>{ 1 };
    auto v2 = test::reaver::optional<test::reaver::variant<int, float>>{ 2.f };

    MAYFLY_CHECK(fmap(v1, [](auto && v) { return test::reaver::get<0>(v); }) == 1);
    MAYFLY_CHECK(fmap(v2, [](auto && v) { return test::reaver::get<1>(v); }) == 2.f);

    test::reaver::optional<test::reaver::variant<int, float>> v3 = test::reaver::none;
});

MAYFLY_END_SUITE;
