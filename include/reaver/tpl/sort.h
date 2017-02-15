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

#include <type_traits>

#include "../id.h"
#include "../static_assert.h"
#include "../swallow.h"
#include "../unit.h"
#include "concat.h"
#include "nth.h"
#include "vector.h"

namespace reaver
{
namespace tpl
{
    inline namespace _v1
    {
        namespace _detail
        {
            template<typename Vector, std::size_t Position, typename Type>
            struct _insert_nth;

            template<typename... Elements, typename Type>
            struct _insert_nth<vector<Elements...>, 0, Type>
            {
                using type = vector<Type, Elements...>;
            };

            template<typename Vector, std::size_t Position, typename Type, bool Inside>
            struct _insert_nth_helper;

            template<typename Head, typename... Elements, std::size_t Position, typename Type>
            struct _insert_nth_helper<vector<Head, Elements...>, Position, Type, true>
            {
                using type = concat<vector<Head>, typename _insert_nth<vector<Elements...>, Position - 1, Type>::type>;
            };

            template<typename... Elements, std::size_t Position, typename Type>
            struct _insert_nth_helper<vector<Elements...>, Position, Type, false>
            {
                static_assert_(Position == sizeof...(Elements));
                using type = vector<Elements..., Type>;
            };

            template<typename... Elements, std::size_t Position, typename Type>
            struct _insert_nth<vector<Elements...>, Position, Type>
            {
                using type = typename _insert_nth_helper < vector<Elements...>, Position, Type, Position<sizeof...(Elements)>::type;
            };

            template<typename Vector, template<typename...> typename Comparator, typename... Elements>
            struct _sorted_insert;

            template<typename Vector, template<typename...> typename Comparator>
            struct _sorted_insert<Vector, Comparator>
            {
                using type = Vector;
            };

            template<template<typename...> typename Comparator, typename Head, typename... Tail>
            struct _sorted_insert<vector<>, Comparator, Head, Tail...>
            {
                using type = typename _sorted_insert<vector<Head>, Comparator, Tail...>::type;
            };

            template<std::size_t Position,
                std::size_t Begin,
                std::size_t End,
                typename Vector,
                template<typename...> typename Comparator,
                typename Type,
                typename = void>
            struct _sorted_position_impl;

            template<std::size_t Begin, std::size_t End, typename Vector, template<typename...> typename Comparator, typename Type>
            struct _sorted_position_impl<~0ull, Begin, End, Vector, Comparator, Type>
            {
                static constexpr const std::size_t value = -1;
            };

            template<template<typename...> typename Comparator, typename Type>
            struct _sorted_position_impl<0, 0, 0, vector<>, Comparator, Type>
            {
                static constexpr const std::size_t value = 0;
            };

            template<typename Single, template<typename...> typename Comparator, typename Type>
            struct _sorted_position_impl<0, 0, 1, vector<Single>, Comparator, Type>
            {
                static constexpr const std::size_t value = Comparator<Single, Type>::value;
            };

            template<std::size_t Position, std::size_t Begin, std::size_t End, typename... Elements, template<typename...> typename Comparator, typename Type>
            struct _sorted_position_impl<Position, Begin, End, vector<Elements...>, Comparator, Type, typename std::enable_if<Position != ~0ull>::type>
            {
            private:
                static constexpr const std::size_t condition1 =
                    Comparator<nth<vector<Elements...>, Position - 1>, Type>::value || Position == Begin || Position == End;
                static constexpr const std::size_t condition2 = Comparator<Type, nth<vector<Elements...>, Position>>::value
                    || (!Comparator<Type, nth<vector<Elements...>, Position>>::value && !Comparator<nth<vector<Elements...>, Position>, Type>::value)
                    || Position == Begin || Position == End;

            public:
                static constexpr const std::size_t value = Comparator<nth<vector<Elements...>, sizeof...(Elements) - 1>,
                                                               Type>::value
                    ? sizeof...(Elements) // greater than the last element
                                          // should be placed at the end
                    : (condition1 ? (condition2 ? Position : _sorted_position_impl < condition2 ? -1 : (Position + (End - Position) / 2),
                                        Position,
                                        End,
                                        vector<Elements...>,
                                        Comparator,
                                        Type > ::value)
                                  : _sorted_position_impl < condition1 ? -1 : (Position - (Position - Begin) / 2),
                          Begin,
                          Position,
                          vector<Elements...>,
                          Comparator,
                          Type > ::value);
            };

            template<typename Vector, template<typename...> typename Comparator, typename Type>
            struct _sorted_position : _sorted_position_impl<Vector::size / 2, 0, Vector::size, Vector, Comparator, Type>
            {
            };

            template<typename... Sorted, template<typename...> typename Comparator, typename Head, typename... Tail>
            struct _sorted_insert<vector<Sorted...>, Comparator, Head, Tail...>
            {
                using type =
                    typename _sorted_insert<typename _insert_nth<vector<Sorted...>, _sorted_position<vector<Sorted...>, Comparator, Head>::value, Head>::type,
                        Comparator,
                        Tail...>::type;
            };

            template<typename... Sorted, template<typename...> typename Comparator>
            struct _sorted_insert<vector<Sorted...>, Comparator>
            {
                using type = vector<Sorted...>;
            };

            template<typename Vector, template<typename...> typename Comparator>
            struct _sort;

            template<typename... Elements, template<typename...> typename Comparator>
            struct _sort<vector<Elements...>, Comparator>
            {
                using type = typename _sorted_insert<vector<>, Comparator, Elements...>::type;
            };
        }

        template<typename Vector, template<typename...> typename Comparator>
        using sort = typename _detail::_sort<Vector, Comparator>::type;
    }
}
}
