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
#include <reaver/logic.h>

#include <string>

namespace test
{
#include "variant.h"
}

MAYFLY_BEGIN_SUITE("variant");

struct alignas(4096) foo
{
    int i;
};

MAYFLY_ADD_TESTCASE("size and alignment", []()
{
    {
        using type = test::reaver::variant<int>;
        MAYFLY_CHECK(sizeof(type) > sizeof(int));
        MAYFLY_CHECK(alignof(type) >= alignof(int));
    }

    {
        using type = test::reaver::variant<int, long double>;
        MAYFLY_CHECK(sizeof(type) > sizeof(long double));
        MAYFLY_CHECK(alignof(type) >= alignof(long double));
    }

    {
        using type = test::reaver::variant<foo, int, bool>;
        MAYFLY_CHECK(sizeof(type) > sizeof(foo));
        MAYFLY_CHECK(alignof(type) >= alignof(foo));
    }
});

MAYFLY_ADD_TESTCASE("construction and inspection without references", []()
{
    test::reaver::variant<int, bool> v1 = 1;
    MAYFLY_CHECK(v1.index() == 0);
    MAYFLY_CHECK(test::reaver::get<0>(v1) == 1);
    MAYFLY_CHECK_THROWS_TYPE(test::reaver::invalid_variant_get, test::reaver::get<1>(v1));

    test::reaver::variant<int, bool> v2 = false;
    MAYFLY_CHECK(v2.index() == 1);
    MAYFLY_CHECK(test::reaver::get<1>(v2) == false);
});

MAYFLY_ADD_TESTCASE("copy construction", []()
{
    {
        std::string value(128, 'a');

        const test::reaver::variant<int, std::string> v1 = value;
        test::reaver::variant<int, std::string> v2 = v1;

        MAYFLY_CHECK(v1 == test::reaver::variant<int, std::string>(value));
        MAYFLY_CHECK(v2 == test::reaver::variant<int, std::string>(value));
    }

    {
        struct foo
        {
            foo() = default;
            foo(foo &&) noexcept = default;
            foo(const foo &)
            {
                throw 1;
            }

            bool operator==(const foo &) const
            {
                return true;
            }
        };

        std::string value = "hello yes this is dog how can I help you";
        test::reaver::variant<std::string, foo> v1 = value;
        foo f;

        try
        {
            const test::reaver::variant<std::string, foo> v2 = f;
            MAYFLY_CHECK(false && "an exception was supposed to be thrown");
        }
        catch (int)
        {
        }
        catch (...)
        {
            MAYFLY_CHECK(false && "an exception of wrong type thrown");
        }

        MAYFLY_CHECK(v1 == test::reaver::variant<std::string, foo>(value));
    }
});

MAYFLY_ADD_TESTCASE("move construction", []()
{
    {
        struct foo
        {
            foo() = default;
            foo(const foo &) noexcept : moved_from(false), moved_into(false)
            {
            }

            foo(foo && other) noexcept : moved_into(true)
            {
                other.moved_from = true;
            }

            bool moved_from = false;
            bool moved_into = false;
        };

        test::reaver::variant<int, foo> v1 = foo{};
        test::reaver::variant<int, foo> v2 = std::move(v1);

        MAYFLY_CHECK(test::reaver::get<1>(v2).moved_into == true);
        MAYFLY_CHECK(test::reaver::get<1>(v1).moved_from == true);
    }
});

MAYFLY_ADD_TESTCASE("copy assignment", []()
{
    {
        std::string value(128, 'a');

        const test::reaver::variant<int, std::string> v1 = value;
        test::reaver::variant<int, std::string> v2 = 1;

        v2 = v1;

        MAYFLY_CHECK(v1 == test::reaver::variant<int, std::string>(value));
        MAYFLY_CHECK(v2 == test::reaver::variant<int, std::string>(value));
    }

    {
        struct foo
        {
            foo() = default;
            foo(foo &&) noexcept = default;
            foo(const foo &)
            {
                throw 1;
            }

            bool operator==(const foo &) const
            {
                return true;
            }
        };

        std::string value = "hello yes this is dog how can I help you";
        test::reaver::variant<std::string, foo> v1 = value;
        const test::reaver::variant<std::string, foo> v2 = foo{};
        MAYFLY_CHECK_THROWS_TYPE(int, v1 = v2);
        MAYFLY_CHECK(v1 == test::reaver::variant<std::string, foo>(value));
    }
});

MAYFLY_ADD_TESTCASE("move assignment", []()
{
    {
        struct foo
        {
            foo() = default;
            foo(const foo &) noexcept : moved_from(false), moved_into(false)
            {
            }

            foo(foo && other) noexcept : moved_into(true)
            {
                other.moved_from = true;
            }

            bool moved_from = false;
            bool moved_into = false;
        };

        test::reaver::variant<int, foo> v1 = 1;
        test::reaver::variant<int, foo> v2 = foo{};
        v1 = std::move(v2);

        MAYFLY_CHECK(test::reaver::get<1>(v1).moved_into == true);
        MAYFLY_CHECK(test::reaver::get<1>(v2).moved_from == true);
    }
});

namespace
{
    struct dtor_type1
    {
        ~dtor_type1() { --count; }

        static std::size_t count;
    };
    std::size_t dtor_type1::count = 0;

    struct dtor_type2
    {
        ~dtor_type2() { --count; }

        static std::size_t count;
    };
    std::size_t dtor_type2::count = 0;
}

MAYFLY_ADD_TESTCASE("destruction", []()
{
    {
        test::reaver::variant<dtor_type1, dtor_type2> v1 = dtor_type1{};
        test::reaver::variant<dtor_type1, dtor_type2> v2 = dtor_type2{};
        test::reaver::variant<dtor_type1, dtor_type2> v3 = dtor_type1{};

        dtor_type1::count = 2;
        dtor_type2::count = 1;
    }

    MAYFLY_CHECK(dtor_type1::count == 0);
    MAYFLY_CHECK(dtor_type2::count == 0);
});

MAYFLY_ADD_TESTCASE("fmap", []()
{
    using namespace std::string_literals;

    MAYFLY_CHECK(test::reaver::fmap(test::reaver::variant<bool, int>(1), [](int i) { return !!i; }) == test::reaver::variant<bool>(true));
    MAYFLY_CHECK(test::reaver::fmap(test::reaver::variant<int>(123), [](int i) { return std::to_string(i); }) == test::reaver::variant<std::string>("123"s));

    const test::reaver::variant<std::string> v1 = "123"s;
    MAYFLY_CHECK(test::reaver::fmap(v1, [](auto && arg) { return std::stoi(arg); }) == test::reaver::variant<int>(123));
    MAYFLY_CHECK(test::reaver::get<0>(v1) == "123");

    struct foo
    {
        foo() = default;
        foo(const foo &) noexcept : moved_from(false), moved_into(false)
        {
        }

        foo(foo && other) noexcept : moved_into(true)
        {
            other.moved_from = true;
        }

        bool moved_from = false;
        bool moved_into = false;
    };

    test::reaver::variant<foo> v2 = foo{};
    test::reaver::fmap(v2, [](auto arg) { return arg; });
    MAYFLY_CHECK(test::reaver::get<0>(v2).moved_from = true);
});

MAYFLY_END_SUITE;

