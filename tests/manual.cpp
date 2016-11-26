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
#   include "manual.h"
}

MAYFLY_BEGIN_SUITE("manual object");

MAYFLY_ADD_TESTCASE("construction and destruction", []()
{
    {
        test::reaver::manual_object<int> object;
        object.emplace(1);
        MAYFLY_REQUIRE(object.reference() == 1);
        const auto & ref = object;
        MAYFLY_REQUIRE(ref.reference() == 1);
    }

    {
        test::reaver::manual_object<int> object{ 1 };
        MAYFLY_REQUIRE(object.reference() == 1);
    }
});

MAYFLY_ADD_TESTCASE("copy and move", []()
{
    struct test_subject
    {
        test_subject(std::string str) : buf(std::move(str))
        {
        }

        test_subject(const test_subject & ref) : buf(ref.buf)
        {
        }

        test_subject(test_subject && ref) : buf(std::move(ref.buf))
        {
            ref.moved = true;
        }

        bool moved = false;
        std::string buf;
    };

    {
        test::reaver::manual_object<test_subject> object;

        object.emplace("123");
        MAYFLY_REQUIRE(object.reference().buf == "123");

        MAYFLY_CHECK(object.copy().buf == "123");
        MAYFLY_CHECK(object.reference().moved == false);

        MAYFLY_CHECK(object.move().buf == "123");
        MAYFLY_CHECK(object.reference().moved == true);
    }
});

MAYFLY_ADD_TESTCASE("destruction", []()
{
    struct test_subject
    {
        test_subject(bool & flag) : flag{ flag }
        {
        }

        ~test_subject()
        {
            flag = true;
        }

        bool & flag;
    };

    {
        bool flag = false;

        {
            test::reaver::manual_object<test_subject> object;
            object.emplace(flag);
        }

        MAYFLY_REQUIRE(flag == false);

        {
            test::reaver::manual_object<test_subject> object;
            object.emplace(flag);
            object.destroy();
        }

        MAYFLY_REQUIRE(flag == true);

        flag = false;

        {
            test::reaver::manual_object<test_subject> object;
            object.emplace(flag);
            object.move();
        }
    }
});

MAYFLY_END_SUITE;

