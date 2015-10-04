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
#include "tls.h"
}

MAYFLY_BEGIN_SUITE("tls");

MAYFLY_ADD_TESTCASE("tls in a single thread", []()
{
    test::reaver::tls_variable<std::uintptr_t> v;
    v = 123;
    MAYFLY_CHECK(v == 123);
});


MAYFLY_ADD_TESTCASE("tls in multiple threads", []()
{
    MAYFLY_MAIN_THREAD;

    test::reaver::tls_variable<std::uintptr_t> v;

    auto f = [&](unsigned i) {
        MAYFLY_THREAD;

        v = i;
        MAYFLY_CHECK(v == i);
    };

    std::thread t1{ f, 123 };
    std::thread t2{ f, 321 };

    t1.join();
    t2.join();
});

MAYFLY_END_SUITE;

