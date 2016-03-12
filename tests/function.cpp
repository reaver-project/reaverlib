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
#   include "function.h"
}

MAYFLY_BEGIN_SUITE("function");

MAYFLY_ADD_TESTCASE("free function", []()
{
    test::reaver::function<bool (int)> is_even = +[](int i){ return !(i % 2); };

    MAYFLY_CHECK(is_even(1) == false);
    MAYFLY_CHECK(is_even(120) == true);
});

MAYFLY_ADD_TESTCASE("function object with const call operator", []()
{
    {
        struct foo
        {
            int i;

            int operator()() const
            {
                return i * 2;
            }
        };

        test::reaver::function<int ()> twice = foo{ 123 };

        MAYFLY_CHECK(twice() == 246);
    }
});

MAYFLY_ADD_TESTCASE("function object without const call operator", []()
{
    {
        struct foo
        {
            int i;

            int operator()()
            {
                return i *= 2;
            }
        };

        test::reaver::function<int ()> twice = foo{ 111 };

        MAYFLY_CHECK(twice() == 222);
        MAYFLY_CHECK(twice() == 444);

        const auto & ctwice = twice;

        MAYFLY_CHECK_THROWS_TYPE(test::reaver::const_call_operator_not_available, ctwice());
    }
});

MAYFLY_ADD_TESTCASE("function object with overloaded call operator", []()
{
    {
        struct foo
        {
            int i;

            int operator()() &
            {
                return i;
            }

            int operator()() &&
            {
                return i + 1;
            }

            int operator()() const &
            {
                return i + 2;
            }
        };

        test::reaver::function<int ()> f = foo{ 1 };
        const auto & cf = f;

        MAYFLY_CHECK(f() == 1);
        MAYFLY_CHECK(cf() == 3);
        MAYFLY_CHECK(std::move(f)() == 2);
    }
});

MAYFLY_ADD_TESTCASE("copy constructor", []()
{
});

MAYFLY_ADD_TESTCASE("move constructor", []()
{
});

MAYFLY_END_SUITE;

