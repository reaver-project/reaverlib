/**
 * Reaver Library License
 *
 * Copyright © 2013-2014 Michał "Griwes" Dominiak
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

#include <boost/bimap.hpp>

#include "target.h"
#include "exception.h"

namespace
{
    using namespace reaver::target;

    boost::bimap<arch, std::string> _arch_strings = []{
        boost::bimap<arch, std::string> ret;
        ret.left.insert({ arch::i386, "i386" });
        ret.left.insert({ arch::i486, "i486" });
        ret.left.insert({ arch::i586, "i586" });
        ret.left.insert({ arch::i686, "i686" });
        ret.left.insert({ arch::i786, "i786" });
        ret.left.insert({ arch::x86_64, "x86_64" });
        return ret;
    }();

    boost::bimap<os, std::string> _os_strings = []{
        boost::bimap<os, std::string> ret;
        ret.left.insert({ os::none, "none" });
        ret.left.insert({ os::linux, "linux" });
        return ret;
    }();

    boost::bimap<env, std::string> _env_strings = []{
        boost::bimap<env, std::string> ret;
        ret.left.insert({ env::elf, "elf" });
        return ret;
    }();
}

reaver::target::__v1::triple::triple(std::string triple_string)
{
    auto first = triple_string.find('-');
    auto second = triple_string.find('-', first + 1);

    std::string arch = triple_string.substr(0, first);
    std::string os = triple_string.substr(first + 1, second - first - 1);
    std::string env = triple_string.substr(second + 1);

    try
    {
        _arch = _arch_strings.right.at(arch);
    }

    catch (...)
    {
        throw unknown_architecture{ arch };
    }

    try
    {
        _os = _os_strings.right.at(os);
    }

    catch (...)
    {
        throw unknown_os{ os };
    }

    try
    {
        _env = _env_strings.right.at(env);
    }

    catch (...)
    {
        throw unknown_environment{ env };
    }
}

std::string reaver::target::__v1::triple::arch_string() const
{
    return _arch_strings.left.at(_arch);
}

std::string reaver::target::__v1::triple::os_string() const
{
    return _os_strings.left.at(_os);
}

std::string reaver::target::__v1::triple::env_string() const
{
    return _env_strings.left.at(_env);
}
