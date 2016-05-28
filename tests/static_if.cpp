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
#   include "static_if.h"
}

MAYFLY_BEGIN_SUITE("static_if");

MAYFLY_ADD_TESTCASE("if", [](){
    int i = 0;

    test::reaver::static_if(std::true_type{}, [&](auto &&) {
        ++i;
    });

    MAYFLY_REQUIRE(i == 1);

    test::reaver::static_if(std::false_type{}, [&](auto &&) {
        ++i;
    });

    MAYFLY_REQUIRE(i == 1);
});

MAYFLY_ADD_TESTCASE("if-else", [](){
    int i = 0;

    test::reaver::static_if(std::true_type{}, [&](auto &&) {
        i = 1;
    }).static_else([&](auto &&... args) {
        i = 2;
    });

    MAYFLY_REQUIRE(i == 1);

    test::reaver::static_if(std::false_type{}, [&](auto &&) {
        i = 1;
    }).static_else([&](auto &&... args) {
        i = 2;
    });

    MAYFLY_REQUIRE(i == 2);
});

MAYFLY_ADD_TESTCASE("if-elseif", [](){
    int i = 0;

    test::reaver::static_if(std::false_type{}, [&](auto &&) {
        i = 1;
    }).static_else_if(std::true_type{}, [&](auto &&) {
        i = 2;
    });

    MAYFLY_REQUIRE(i == 2);

    test::reaver::static_if(std::false_type{}, [&](auto &&) {
        i = 0;
    }).static_else_if(std::false_type{}, [&](auto &&) {
        i = 1;
    });

    MAYFLY_REQUIRE(i == 2);
});

MAYFLY_ADD_TESTCASE("if-elseif-else", [](){
    int i = 0;

    test::reaver::static_if(std::false_type{}, [&](auto &&) {
        i = 1;
    }).static_else_if(std::true_type{}, [&](auto &&) {
        i = 2;
    }).static_else([&](auto &&) {
        i = 3;
    });

    MAYFLY_REQUIRE(i == 2);

    test::reaver::static_if(std::false_type{}, [&](auto &&) {
        i = 0;
    }).static_else_if(std::false_type{}, [&](auto &&) {
        i = 1;
    }).static_else([&](auto &&) {
        i = 3;
    });

    MAYFLY_REQUIRE(i == 3);

    test::reaver::static_if(std::true_type{}, [&](auto &&) {
        i = 0;
    }).static_else_if(std::true_type{}, [&](auto &&) {
        i = 1;
    }).static_else([&](auto &&) {
        MAYFLY_REQUIRE(false); // should never be called
        i = 2;
    });

    MAYFLY_REQUIRE(i == 0);
});

MAYFLY_END_SUITE;

