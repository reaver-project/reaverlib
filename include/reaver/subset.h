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

#include <type_traits>

#include "tpl/vector.h"

namespace reaver
{
inline namespace _v1
{
    template<typename Checked, typename Set>
    struct is_subset : public std::false_type
    {
    };

    template<typename... Set>
    struct is_subset<tpl::vector<>, tpl::vector<Set...>> : public std::true_type
    {
    };

    template<typename... Checked>
    struct is_subset<tpl::vector<Checked...>, tpl::vector<>> : public std::false_type
    {
    };

    template<typename T>
    struct is_subset<tpl::vector<T>, tpl::vector<T>> : public std::true_type
    {
    };

    template<typename T, typename... Others>
    struct is_subset<tpl::vector<T>, tpl::vector<T, Others...>> : public std::true_type
    {
    };

    template<typename T, typename Head, typename... Tail>
    struct is_subset<tpl::vector<T>, tpl::vector<Head, Tail...>> : public is_subset<tpl::vector<T>, tpl::vector<Tail...>>
    {
    };

    template<typename... Checked, typename... Set>
    struct is_subset<tpl::vector<Checked...>, tpl::vector<Set...>> : public all_of<is_subset<tpl::vector<Checked>, tpl::vector<Set...>>::value...>
    {
    };
}
}
