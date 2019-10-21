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

#include "typeclass/typeclass.h"

#include <reaver/function.h>

#include <reaver/mayfly.h>

MAYFLY_BEGIN_SUITE("typeclass");

struct counter_definition
{
    using increment = void();
    using get_value = int();
};

DEFINE_TYPECLASS(counter, increment, get_value);

// clang-format off
DEFAULT_INSTANCE(counter, T)
{
    static void increment(T & t)
    {
        ++t;
    }

    static int get_value(T & t)
    {
        return t.get_value();
    }
};
// clang-format on

class simple_counter : public counter
{
public:
    simple_counter & operator++()
    {
        ++_value;
        return *this;
    }

    int get_value() const
    {
        return _value;
    }

private:
    std::uintmax_t _value = 0;
};

MAYFLY_ADD_TESTCASE("default implementation", [] {
    simple_counter some_counter;
    MAYFLY_REQUIRE(counter::get_value(some_counter) == 0);
    counter::increment(some_counter);
    MAYFLY_REQUIRE(counter::get_value(some_counter) == 1);
});

MAYFLY_ADD_TESTCASE("erased", [] {
    simple_counter some_counter;

    counter::erased erased = some_counter;
    counter::erased_ref erased_ref = some_counter;

    counter::increment(some_counter);
    counter::increment(erased);
    counter::increment(erased_ref);

    MAYFLY_REQUIRE(counter::get_value(erased) == 1);
    MAYFLY_REQUIRE(counter::get_value(erased_ref) == 2);
    MAYFLY_REQUIRE(counter::get_value(some_counter) == 2);
});

class broken_counter
{
public:
    TYPECLASS_INSTANCE(typename T);

    int get_value() const
    {
        return 123;
    }
};

// clang-format off
INSTANCE(counter, broken_counter)
{
    static void increment(broken_counter &)
    {
        throw 1;
    }
};
// clang-format on

MAYFLY_ADD_TESTCASE("overriden implementation", [] {
    broken_counter some_counter;
    MAYFLY_REQUIRE(counter::get_value(some_counter) == 123);
    MAYFLY_REQUIRE_THROWS_TYPE(int, counter::increment(some_counter));
    MAYFLY_REQUIRE(counter::get_value(some_counter) == 123);
});

template<typename T>
struct sizeof_counter_definition
{
    using increment = void();
    using get_value = int();
};

DEFINE_TYPECLASS_TEMPLATE((typename T), (T), sizeof_counter, increment, get_value);

// clang-format off
DEFAULT_INSTANCE_TEMPLATE((typename T), (T), sizeof_counter, U)
{
    static void increment(U & u)
    {
        ++u;
    }

    static int get_value(U & u)
    {
        return u.get_value();
    }
};
// clang-format on

template<typename T>
struct some_sizeof_counter : sizeof_counter<T>
{
    TYPECLASS_INSTANCE_TEMPLATE((typename U), (U), some_sizeof_counter);

    void increment()
    {
        _value += sizeof(T);
    }

    int get_value() const
    {
        return _value;
    }

private:
    int _value = 0;
};

// clang-format off
INSTANCE_TEMPLATE((typename T), (T), sizeof_counter, some_sizeof_counter<T>)
{
    static void increment(some_sizeof_counter<T> & t)
    {
        t.increment();
    }
};
// clang-format on

MAYFLY_ADD_TESTCASE("typeclass template", [] {
    some_sizeof_counter<int> some_counter;
    MAYFLY_REQUIRE(sizeof_counter<int>::get_value(some_counter) == 0);
    sizeof_counter<int>::increment(some_counter);
    MAYFLY_REQUIRE(sizeof_counter<int>::get_value(some_counter) == sizeof(int));

    sizeof_counter<int>::erased erased = some_counter;
    sizeof_counter<int>::erased_ref erased_ref = some_counter;

    sizeof_counter<int>::increment(some_counter);
    MAYFLY_REQUIRE(sizeof_counter<int>::get_value(erased) == sizeof(int));
    MAYFLY_REQUIRE(sizeof_counter<int>::get_value(erased_ref) == sizeof(int) * 2);
});

MAYFLY_END_SUITE;
