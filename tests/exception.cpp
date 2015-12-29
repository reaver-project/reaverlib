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

#include <unordered_set>

#include <reaver/logger.h>

namespace test
{
#   include "../include/reaver/exception.h"
}

#include <reaver/mayfly.h>

MAYFLY_BEGIN_SUITE("exception");

MAYFLY_ADD_TESTCASE("validity of construction", []()
{
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::invalid_exception_level, test::reaver::exception{ test::reaver::logger::debug });
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::invalid_exception_level, test::reaver::exception{ test::reaver::logger::trace });
    MAYFLY_CHECK_NOTHROW(test::reaver::exception{ test::reaver::logger::error });
});

MAYFLY_ADD_TESTCASE("what", []()
{
    using namespace std::string_literals;
    MAYFLY_CHECK((test::reaver::exception{ test::reaver::logger::always } << "abc" << 1).what() == "abc1\n"s);
});

MAYFLY_END_SUITE;

