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

#include "typeclass/functor.h"

MAYFLY_BEGIN_SUITE("typeclass");
MAYFLY_BEGIN_SUITE("functor");

INSTANCE_TEMPLATE_HELPER;

template<typename T>
struct silly_wrapper
{
    TYPECLASS_INSTANCE_TEMPLATE((typename U), (silly_wrapper), reaver::functor);

    silly_wrapper(T t) : t{ std::move(t) }
    {
    }

    T t;
};

// clang-format off
INSTANCE_TEMPLATE((typename T), (silly_wrapper), reaver::functor, silly_wrapper<T>)
{
    template<typename Fun, typename A>
    static auto fmap(silly_wrapper<A> a, Fun && fn)
    {
        using wrapped = decltype(std::forward<Fun>(fn)(a.t));
        return silly_wrapper<wrapped>{ std::forward<Fun>(fn)(a.t) };
    }
};
// clang-format on

MAYFLY_ADD_TESTCASE("silly wrapper", [] {
    silly_wrapper<int> sw1{ 1 };
    auto sw2 = reaver::fmap(sw1, [](auto && i) { return i * 2 + 1; });

    MAYFLY_REQUIRE(std::is_same<decltype(sw2), silly_wrapper<int>>::value);
    MAYFLY_REQUIRE(sw2.t == 3);

    auto sw3 = reaver::fmap(sw2, [](auto && i) { return std::to_string(i); });

    MAYFLY_REQUIRE(std::is_same<decltype(sw3), silly_wrapper<std::string>>::value);
    MAYFLY_REQUIRE(sw3.t == "3");
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
