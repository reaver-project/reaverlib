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

#include <numeric>

namespace test
{
#include "prelude/fold.h"
}

MAYFLY_BEGIN_SUITE("prelude");
MAYFLY_BEGIN_SUITE("fold");

namespace
{
struct foo
{
    template<typename T>
    bool operator==(T)
    {
        return false;
    }

    bool operator==(foo)
    {
        return true;
    }
};

struct bar
{
    template<typename T>
    bool operator==(T)
    {
        return false;
    }

    bool operator==(bar)
    {
        return true;
    }
};

struct baz
{
    template<typename T>
    bool operator==(T)
    {
        return false;
    }

    bool operator==(baz)
    {
        return true;
    }
};

struct f
{
    template<typename T, typename U>
    auto operator()(T, U)
    {
        return T{};
    }
};

struct g
{
    template<typename T, typename U>
    auto operator()(T, U)
    {
        return U{};
    }
};
}

MAYFLY_ADD_TESTCASE("foldr", []() {
    MAYFLY_CHECK(test::reaver::foldr(std::plus<>{}, 0) == 0);
    MAYFLY_CHECK(test::reaver::foldr(std::plus<>{}, 1, 2, 3, 4) == 10);

    MAYFLY_CHECK(test::reaver::foldr(f{}, foo{}, bar{}, baz{}) == bar{});
    MAYFLY_CHECK(test::reaver::foldr(g{}, foo{}, bar{}, baz{}) == foo{});
});

MAYFLY_ADD_TESTCASE("foldl", []() {
    MAYFLY_CHECK(test::reaver::foldl(std::plus<>{}, 0) == 0);
    MAYFLY_CHECK(test::reaver::foldl(std::plus<>{}, 1, 2, 3, 4) == 10);

    MAYFLY_CHECK(test::reaver::foldl(f{}, foo{}, bar{}, baz{}) == foo{});
    MAYFLY_CHECK(test::reaver::foldl(g{}, foo{}, bar{}, baz{}) == baz{});
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
