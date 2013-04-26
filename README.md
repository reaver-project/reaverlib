# ReaverLib

ReaverLib is a collection of libraries used throughout the project, that should
also be useful for people from outside of it. This document briefly explains how
to use each of the libraries.


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


## `reaver::lexer`

`reaver::lexer` is a simple and dumb lexer, based on `<regex>`, so it needs an environment
with implemented and working implementation of standard C++ regular expressions.

[ - **Note**: it is currently unusable with Unicode. As C++11 `<regex>` isn't Unicode-aware
(hopefully it will be, one day), you can't use UTF-8 regular expressions (you can, though,
call it *on* UTF-8 strings, but that ain't really a good idea). The only sane way is to
use `char32_t` and `std::u32string`, but, as `reaver::lexer` (and `reaver::parser`) are not
**yet** parametrized on `CharType` as most such libraries, it is impossible to use it with
real Unicode. Support for `CharType` parameters is planned in near feature, but probably
only after Reaver Assembler is implemented. - ]

To use `reaver::lexer`, you must first define allowed tokens. They are then contained in
class resembling `boost::program_options::options_description`, and passed to the
`reaver::lexer::tokenize()` function. Upon encountering string not matching any of the
defined tokens, `reaver::lexer::tokenize()` throws `reaver::lexer::unexpected_characters`.

There are two ways to define a token:

  1. Define a `token_description`, or
  2. Define a `token_definition`.

`token_description` is a template, which carries its type information with it; it will be
mostly useful when writing `reaver::parser`-based parser, which can automatically infer
the `reaver::parser::rule` return type from `token_definition`'s template parameter.
Let's see how to declare a `token_definition`, which will use a default conversion into
`uint64_t` from string representing unsigned integer.

    reaver::lexer::token_definition<uint64_t> integer{
        0,                  // (1)
        "[1-9][0-9]*"       // (2)
    };

  1. Type identifier, also used as a priority, i.e., token descriptions are internally
evaluated starting with identifier of type 0, then 1, up to `(uint64_t)-2`; `(uint64_t)-1`
means *invalid type*.
  2. Regular expression used to match this token.

`reaver::lexer` uses `boost::lexical_cast` to cast `std::string` into other types by
default. You can define own conversion in two ways:

  1. By specializing function `template<typename Out, typename In> Out convert(const In &)`
in namespace `reaver::lexer` for given type to change the conversion globally, or
  2. By providing a conversion function to the `token_definition`.

Here's an example for (2).

    reaver::lexer::token_definition<uint64_t> hex{
        2,                                                                      // (1)
        "0x[0-9a-fA-F]+",                                                       // (2)
        [](const std::string & str)                                             // (3)
        {
            uint64_t a; std::stringstream{ str } >> std::hex >> a; return a;
        }
    };

(1) and (2) are the same as before, (3) is a function object or pointer that takes apropriate
string type ane returns type of `token_definition`'s template parameter.

`token_definition` is not a template, it doesn't carry compile-time information about type.
It is used internally as a polymorphic wrapper for internal token description. You should
have no interest in declaring `token_description` by yourself; here's short description of its
constructors, which will be helpful when talking about `tokens_description`. This is essentially
the same thing as last `token_definition` example, just not carrying type information.

    reaver::lexer::token_description hex{
        2,
        "0x[0-9a-fA-F]+",
        reaver::lexer::match_type<uint64_t>{},
        [](const std::string & str)
        {
            uint64_t a; std::stringstream{ str } >> std::hex >> a; return a;
        }
    };

**TODO**: adding `token_description` and `token_definition` to the `tokens_description`, using
`lexer::tokenize()`, getting lexed tokens out from vector of `token`s, using `tokens_description::operator[]`,
using token aliases.
