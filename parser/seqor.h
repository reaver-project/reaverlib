/**
 * Reaver Library Licence
 *
 * Copyright (C) 2013 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
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
 * Michał "Griwes" Dominiak
 *
 **/

#pragma once

#include <type_traits>
#include <tuple>

#include <boost/optional.hpp>

#include "helpers.h"

namespace reaver
{
    namespace parser
    {
        template<typename Tref, typename Uref>
        class seqor_parser : public parser
        {
            using T = typename std::remove_reference<Tref>::type;
            using U = typename std::remove_reference<Uref>::type;

            using value_type = typename std::conditional<
                std::is_same<typename T::value_type, void>::value,
                typename std::conditional<
                    std::is_same<typename U::value_type, void>::value,
                    void,
                    boost::optional<typename U::value_type>
                >::type,
                typename std::conditional<
                    std::is_same<typename U::value_type, void>::value,
                    boost::optional<typename T::value_type>,
                    std::tuple<boost::optional<typename T::value_type>, boost::optional<typename U::value_type>>
                >::type
            >::type;
        };
    }
}
