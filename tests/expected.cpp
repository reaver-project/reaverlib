/**
 * Reaver Library Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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
#   include "expected.h"
#   include "prelude/monad.h"
}

MAYFLY_BEGIN_SUITE("expected");

MAYFLY_ADD_TESTCASE("construction", []()
{
    {
        auto exp = test::reaver::make_expected(1);
        MAYFLY_CHECK(*exp == 1);
    }

    {
        auto err = test::reaver::make_error<int>(std::string("oh well"));
        MAYFLY_CHECK_THROWS_TYPE(std::string, *err);
        MAYFLY_CHECK(err.get_error() == "oh well");
    }
});

MAYFLY_ADD_TESTCASE("fmap", []()
{
    {
        auto exp = test::reaver::make_expected(1);
        MAYFLY_CHECK(*test::reaver::fmap(exp, [](auto i){ return i + 2; }) == 3);
    }

    {
        auto err = test::reaver::make_error<int>(std::string("oh well"));
        MAYFLY_CHECK_THROWS_TYPE(std::string, *fmap(err, [](auto){ MAYFLY_REQUIRE(false); return test::reaver::unit{}; }));
    }

    {
        const auto exp = test::reaver::make_expected(1);
        MAYFLY_CHECK(*test::reaver::fmap(exp, [](auto i){ return i + 2; }) == 3);
    }

    {
        const auto err = test::reaver::make_error<int>(std::string("oh well"));
        MAYFLY_CHECK_THROWS_TYPE(std::string, *fmap(err, [](auto){ MAYFLY_REQUIRE(false); return test::reaver::unit{}; }));
    }

    {
        MAYFLY_CHECK(*test::reaver::fmap(test::reaver::make_expected(1), [](auto i){ return i + 2; }) == 3);
    }

    {
        MAYFLY_CHECK_THROWS_TYPE(std::string, *fmap(
            test::reaver::make_error<int>(std::string("oh well")),
            [](auto){ MAYFLY_REQUIRE(false); return test::reaver::unit{}; }
        ));
    }
});

MAYFLY_ADD_TESTCASE("join", []()
{
    {
        auto exp = test::reaver::make_expected(test::reaver::make_expected(1));
        MAYFLY_CHECK(*test::reaver::join(exp) == 1);
    }

    {
        auto err = test::reaver::make_expected(test::reaver::make_error<int>(1));
        MAYFLY_CHECK(test::reaver::join(err).get_error() == test::reaver::variant<int, std::exception_ptr>(1));
    }

    {
        const auto exp = test::reaver::make_expected(test::reaver::make_expected(1));
        MAYFLY_CHECK(*test::reaver::join(exp) == 1);
    }

    {
        const auto err = test::reaver::make_expected(test::reaver::make_error<int>(1));
        MAYFLY_CHECK(test::reaver::join(err).get_error() == test::reaver::variant<int, std::exception_ptr>(1));
    }

    {
        MAYFLY_CHECK(*test::reaver::join(test::reaver::make_expected(test::reaver::make_expected(1))) == 1);
    }

    {
        MAYFLY_CHECK(test::reaver::join(test::reaver::make_expected(test::reaver::make_error<int>(1))).get_error() == test::reaver::variant<int, std::exception_ptr>(1));
    }

});

MAYFLY_ADD_TESTCASE("bind", []()
{
    {
        auto exp = test::reaver::make_expected(1);
        MAYFLY_CHECK(*test::reaver::mbind(exp, [](auto i){ return test::reaver::make_expected(i * 2); }) == 2);
        MAYFLY_CHECK(test::reaver::mbind(exp, [](auto i){ return test::reaver::make_error<int>(i * 3); }).get_error() == test::reaver::variant<int, std::exception_ptr>(3));
    }
});

MAYFLY_END_SUITE;

