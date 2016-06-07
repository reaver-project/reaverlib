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
#   include "prelude/monad.h"
}

MAYFLY_BEGIN_SUITE("prelude");
MAYFLY_BEGIN_SUITE("monad");

MAYFLY_ADD_TESTCASE("std::vector", [](){
    MAYFLY_REQUIRE(test::reaver::mbind(std::vector<int>{ 1, 2, 3 }, [](auto i){ return std::vector<float>{ i * 0.5f, i * 1.f }; }) == std::vector<float>{ 0.5, 1, 1, 2, 1.5, 3 });

    std::vector<int> v{ 1, 2, 3 };
    MAYFLY_REQUIRE(test::reaver::mbind(v, [](auto i){ return std::vector<float>{ i * 0.5f, i * 1.f }; }) == std::vector<float>{ 0.5, 1, 1, 2, 1.5, 3 });
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;

