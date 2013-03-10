/**
 * Reaver Library
 *
 * Copyright (C) 2012-2013 Reaver Project Team:
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

#include <iostream>

namespace reaver
{
    namespace style
    {
        class style;
    }
}

std::ostream & operator<<(std::ostream &, const reaver::style::style &);

namespace reaver
{
    namespace style
    {
        enum class colors
        {
            black = 30,
            red,
            green,
            brown,
            blue,
            magenta,
            cyan,
            gray,
            bblack = 90,
            bred,
            bgreen,
            bbrown,
            bblue,
            bmagenta,
            bcyan,
            bgray,
            def = 9
        };

        enum class styles
        {
            bold = 1,
            underline = 4,
            bright,
            def = 0
        };

        class style
        {
        public:
            style(colors fore = colors::def, colors back = colors::def, styles style = styles::def) : _forecolor(fore),
                _backcolor(back), _style(style)
            {
            }

            friend std::ostream & ::operator<<(std::ostream &, const style &);

        private:
            colors _forecolor;
            colors _backcolor;
            styles _style;
        };
    }
}
