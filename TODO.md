 * `reaver::parser`: callback support
 * `reaver::logger`: file change on date, date based log file names
 * `README`: mention `logger::logger_friend`
 * `README`: finish `reaver::lexer` description
 * `README`: write `reaver::parser` description
 * `reaver::parser`: always return rvalues from the parser, rather than rvalues *or* const references
 * `reaver::parser`: special case for `expect_parser<T, list_parser<U, V>>`
 * `reaver::lexer`: add `basic_lexer_iterator`
 * `reaver::parser`: add support for `basic_lexer_iterator`, as opposed to only supporting `std::vector<token>::const_iterator`
 * `reaver::parser`: add a way to get `begin` and `end` iterators of every subparser of invoked parser. Proper design work
    on this one is probably *highly required*.
    *Couldn't this be solved by providing standard callback set and do this just with callback support? (Would probably involve
    lots of boilerplate at call site, but it can be a good (as in: sane) solution - or at least a partial one.)*
