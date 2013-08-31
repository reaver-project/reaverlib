 * `reaver::parser`: add `literal`, as opposed to `rule`. `literal`s and `rule`s cannot be mixed in a single parser;
    `rule`s use `basic_lexer_iterator`, while `literal`s will use iterators over a CharType to work; it's not really possible
    to allow mixing those.
 * `reaver::lexer`: split `lexer.h` into several files
 * `reaver::parser`: callback support
 * `reaver::logger`: file change on date, date based log file names
 * `README`: mention `logger::logger_friend`
 * `README`: finish `reaver::lexer` description
 * `README`: write `reaver::parser` description
 * `README`: write `reaver::exception` and `reaver::error_engine` description
 * `README`: write `reaver::plugin` description
 * `README`: write `reaver::semaphore` description
 * `README`: write `reaver::static_for` description
 * `README`: write `reaver::callbacks` description
 * `README`: write `reaver::target` description
 * `README`: describe traits and helpers found in `tmp.h`
 * `reaver::parser`: always return rvalues from the parser, rather than rvalues *or* const references (so a move out is
    always possible)
 * `reaver::parser`: special case for `expect_parser<T, list_parser<U, V>>`
 * `reaver::parser`: add a way to get `begin` and `end` iterators of every subparser of invoked parser. Proper design work
    on this one is probably *highly required*.
    *Couldn't this be solved by providing standard callback set and do this just with callback support? (Would probably involve
    lots of boilerplate at call site, but it can be a good (as in: sane) solution - or at least a partial one.)*
 * `reaver::thread_pool`: testing, testing, testing!
 * `reaver::async`: write as a sane wrapper over a default `reaver::thread_pool`.
 * `README`: write `reaver::thread_pool` and `reaver::async` description
 * `reaver::format::executable`: write as the first part of Reaver Format library, which will provide uniform interfaces
    for loading and saving various types of file formats. For starters, executable file formats for use by the linker.
