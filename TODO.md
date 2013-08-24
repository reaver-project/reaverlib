 * `reaver::lexer`: add `basic_lexer_iterator`. `tokenize` will no longer exist. The iterator will be an iterator into
    a (most probably) `std::deque`, and the tokenization will happen in asynchronous manner.
 * `reaver::parser`: migrate to `basic_lexer_iterator`. Parametrize parsers on `CharType` (it's a must since `token`
    became `basic_token<CharType>`).
 * `reaver::parser`: add `literal`, as opposed to `rule`. `literal`s and `rule`s cannot be mixed in a single parser;
    `rule`s use `basic_lexer_iterator`, while `literal`s will use iterators over a CharType to work; it's not really possible
    to allow mixing those.
 * `reaver::lexer`: (?) move `iterator_wrapper` out of `reaver::lexer`; put it in its own file and stuff. (?)
 * `reaver::parser`: callback support
 * `reaver::logger`: file change on date, date based log file names
 * `README`: mention `logger::logger_friend`
 * `README`: finish `reaver::lexer` description
 * `README`: write `reaver::parser` description
 * `reaver::parser`: always return rvalues from the parser, rather than rvalues *or* const references (so a move out is
    always possible)
 * `reaver::parser`: special case for `expect_parser<T, list_parser<U, V>>`
 * `reaver::parser`: add a way to get `begin` and `end` iterators of every subparser of invoked parser. Proper design work
    on this one is probably *highly required*.
    *Couldn't this be solved by providing standard callback set and do this just with callback support? (Would probably involve
    lots of boilerplate at call site, but it can be a good (as in: sane) solution - or at least a partial one.)*
 * `reaver::thread_pool`: testing, testing, testing!
 * `reaver::async`: write as a sane wrapper over a default `reaver::thread_pool`.
