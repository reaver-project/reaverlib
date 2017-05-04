/**
 * Reaver Library Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "typeclass/swappable.h"

MAYFLY_BEGIN_SUITE("typeclass");
MAYFLY_BEGIN_SUITE("swappable");

MAYFLY_ADD_TESTCASE("default swap", [] {
    int a = 1;
    int b = 2;

    reaver::swap(a, b);

    MAYFLY_REQUIRE(a == 2);
    MAYFLY_REQUIRE(b == 1);
});

MAYFLY_ADD_TESTCASE("default member swap", [] {
    std::vector<int> v1{ 1, 2, 3 };
    std::vector<int> v2{ 4, 5, 6 };

    reaver::swap(v1, v2);

    MAYFLY_REQUIRE(v1 == std::vector<int>{ 4, 5, 6 });
    MAYFLY_REQUIRE(v2 == std::vector<int>{ 1, 2, 3 });
});

struct swap_test
{
    TYPECLASS_INSTANCE(typename T);

    swap_test * swapped_with = nullptr;
};

// clang-format off
INSTANCE(reaver::swappable, swap_test)
{
    static void swap(swap_test & lhs, swap_test & rhs)
    {
        lhs.swapped_with = &rhs;
        rhs.swapped_with = &lhs;
    }
};
// clang-format on

MAYFLY_ADD_TESTCASE("custom swap", [] {
    swap_test a;
    swap_test b;

    reaver::swap(a, b);

    MAYFLY_REQUIRE(a.swapped_with == &b);
    MAYFLY_REQUIRE(b.swapped_with == &a);
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
