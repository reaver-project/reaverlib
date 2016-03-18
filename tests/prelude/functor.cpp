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

#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace test
{
#   include "prelude/functor.h"
#   include "swallow.h"
#   include "tpl/vector.h"
}

MAYFLY_BEGIN_SUITE("prelude");
MAYFLY_BEGIN_SUITE("functor");

MAYFLY_ADD_TESTCASE("std::vector", []
{
    MAYFLY_CHECK(test::reaver::fmap(std::vector<int>{ 1, 2, 3, 4, 5}, [](auto v) { return v * 2; }) == std::vector<int>{ 2, 4, 6, 8, 10 });
    MAYFLY_CHECK(test::reaver::fmap(std::vector<int>{}, [](auto v) { return 0; }) == std::vector<int>{});
    MAYFLY_CHECK(test::reaver::fmap(std::vector<int>{ 1, 2, 3 }, [](auto v) { return std::to_string(v); }) == std::vector<std::string>{ "1", "2", "3" });
});

MAYFLY_ADD_TESTCASE("std::unique_ptr", []
{
    MAYFLY_CHECK(*test::reaver::fmap(std::make_unique<int>(1), [](auto v) { return v * 2; }) == 2);
    MAYFLY_CHECK(test::reaver::fmap(std::unique_ptr<int>(nullptr), [](auto v) { return v * 2; }) == nullptr);
    MAYFLY_CHECK(*test::reaver::fmap(std::make_unique<int>(1), [](auto v) { return std::to_string(v); }) == "1");
});

MAYFLY_ADD_TESTCASE("std::shared_ptr", []
{
    MAYFLY_CHECK(*test::reaver::fmap(std::make_shared<int>(1), [](auto v) { return v * 2; }) == 2);
    MAYFLY_CHECK(test::reaver::fmap(std::shared_ptr<int>(nullptr), [](auto v) { return v * 2; }) == nullptr);
    MAYFLY_CHECK(*test::reaver::fmap(std::make_shared<int>(1), [](auto v) { return std::to_string(v); }) == "1");
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;

