/**
 * Reaver Library Licence
 *
 * Copyright © 2012-2013 Michał "Griwes" Dominiak
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

#include "style.h"

#ifdef __unix__
# include <unistd.h>
#endif

// stdout and stderr only supported at the moment
std::ostream & operator<<(std::ostream & stream, const reaver::style::__v1::style & style)
{
#ifdef __unix__
    if ((&stream == &std::cout && isatty(STDOUT_FILENO)) || (&stream == &std::cerr && isatty(STDERR_FILENO)))
    {
        stream << "\033[" << (uint16_t)style._style << ';' << (uint16_t)style._forecolor << ';' << (uint16_t)style._backcolor + 10 << 'm';
    }
#endif

    return stream;
}
