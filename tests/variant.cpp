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

#include <reaver/logic.h>
#include <reaver/mayfly.h>

#include <memory>
#include <string>
#include <variant>

namespace test
{
#include "variant.h"
}

MAYFLY_BEGIN_SUITE("variant");

MAYFLY_ADD_TESTCASE("fmap", []() {
    using namespace std::string_literals;

    MAYFLY_CHECK(test::reaver::fmap(std::variant<bool, int>(1), [](int i) { return !!i; }) == std::variant<bool>(true));
    MAYFLY_CHECK(test::reaver::fmap(std::variant<int>(123), [](int i) { return std::to_string(i); }) == std::variant<std::string>("123"s));

    const std::variant<std::string> v1 = "123"s;
    MAYFLY_CHECK(test::reaver::fmap(v1, [](auto && arg) { return std::stoi(arg); }) == std::variant<int>(123));
    MAYFLY_CHECK(std::get<0>(v1) == "123");

    struct foo
    {
        foo() = default;
        foo(const foo &) noexcept : moved_from(false), moved_into(false)
        {
        }

        foo(foo && other) noexcept : moved_into(true)
        {
            other.moved_from = true;
        }

        bool moved_from = false;
        bool moved_into = false;
    };

    std::variant<foo> v2 = foo{};
    test::reaver::fmap(v2, [](auto arg) { return arg; });
    MAYFLY_CHECK(std::get<0>(v2).moved_from = true);
});

MAYFLY_ADD_TESTCASE("recursive_wrapper fmap", []() {
    struct foo
    {
        foo(int i) : i{ i }
        {
        }

        int i;
    };

    std::variant<test::reaver::recursive_wrapper<foo>> v = foo{ 1 };
    MAYFLY_CHECK(std::get<0>(fmap(v, [](auto && f) { return f.i; })) == 1);
});

MAYFLY_END_SUITE;
