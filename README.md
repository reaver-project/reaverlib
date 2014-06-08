# ReaverLib

ReaverLib is a collection of libraries used throughout the project, that should
also be useful for people from outside of it. This document briefly explains how
to use each of the libraries.

## Installation

You will need the following tools installed in your system:

 * POSIX shell
 * make
 * git
 * clang++, libc++
 * Boost, built with clang++ and libc++ (most prebuilt Boost distributions for Linux are
built with GCC and libstdc++ and they do not work properly with clang++ and libc++)

GCC will be tested somewhen in the future, when I get 4.9 out of the box with my Linux
distribution; I don't care about it enough to build it and set up in a working environment.

To install ReaverLib, enter the following commands in your console:

    git clone git://github.com/griwes/ReaverLib
    cd ReaverLib
    make
    sudo make install


## `reaver::style`

`reaver::style` is a simple library that allows to style a console output. At the
moment, it only support UNIX `tty`s, and only on `stdout` and `stderr`, as it uses
`isatty` on standard C handles.

Here's an example.

    #include <iostream>

    #include <reaver/style.h>

    using namespace reaver::style;

    int main()
    {
        std::cout << style(colors::bred, colors::def, styles::bold) << "Here's bold, bright red text.\n";
        std::cout << style() << "Here's normal text.\n";
    }


## `reaver::logger`

`reaver::logger` is a lightweight logger library. So far, it offers seven logging
levels and supports both standard streams passed by reference and `fstream`s passed
as `shared_ptr`s. The library also provides a ready-for-use logger - `reaver::logger::dlog`
(called this insanely because of stupid standard library implementations polluting global
namespace with `log`; you generally want to `using namespace reaver::logger;`, don't
you?).

This library supports and internally uses `reaver::style`. `logger() << ...;` does not
block outputting, although it might block on a push to a action queue, so using it
in massively multithreading environment may slow it down.

An example:

    #include <reaver/logger.h>

    using namespace reaver::logger;

    int main()
    {
        dlog.add_stream({ std::make_shared<std::fstream>{ "test.out", std::ios::out } });

        dlog() << "This will always be printed."; // no '\n' needed
        dlog(trace) << "This will normally be invisible.";
        dlog(crash) << "Something bad has happened.";
    }

Support for more fancy features, like changing output filenames at certain time
points and similar are planned.
