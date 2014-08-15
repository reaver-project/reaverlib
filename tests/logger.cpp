/**
 * Reaver Library Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <reaver/logger.h>

#include <sstream>

MAYFLY_BEGIN_SUITE("logger");

MAYFLY_ADD_TESTCASE("basic printing to stringstream", []
{
    std::ostringstream stream;

    {
        reaver::logger::logger logger;
        logger.add_stream(stream);

        logger() << "hello world! " << 1 << std::boolalpha << false;
    }

    MAYFLY_REQUIRE(stream.str() == "hello world! 1false\n");
});

MAYFLY_END_SUITE;
