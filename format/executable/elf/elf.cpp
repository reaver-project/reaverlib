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

#include "elf.h"
#include "elf32.h"
#include "elf64.h"

std::unique_ptr< reaver::format::executable::executable > reaver::format::executable::elf::read(std::istream & in)
{
    auto p = in.tellg();
    char ident[16];
    in.read(ident, 16);
    in.seekg(p);

    if (!(ident[0] == 0x7F && ident[1] == 'E' && ident[2] == 'L' && ident[3] == 'F'))
    {
        return nullptr;
    }

    switch (ident[4])
    {
        case 1:
            return elf32::read(in);
        case 2:
            return elf64::read(in);
        default:
            return nullptr;
    }
}
