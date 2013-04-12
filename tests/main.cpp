#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

int main()
{
    lex::tokens_description desc;

    lex::token_definition<uint64_t> hex{ 2, "0x[0-9a-fA-F]+", [](const std::string & str)
    {
        uint64_t a; std::stringstream(str) >> std::hex >> a; return a;
    } };

    lex::token_definition<std::string> ident{ 1, "[a-zA-Z_][a-zA-Z0-9_]*" };

    lex::token_definition<std::string> op{ 6, "[\\+\\-\\*\\/]" };

    desc.add(ident)
        (hex)
        (3, "[0-9]+", lex::match_type<uint64_t>{})
        (4, "\"([^\"\\\\]|\\\\.)*\"")
        (5, "[ \n]")
        (op);

    auto t = lex::tokenize("identifier and then hexnumber 0x1000 \"quoted \\\"string\" new line\n and then decnumber 1000", desc);

    for (auto elem : t)
    {
        switch (elem.type())
        {
            case 1:
                std::cout << "Type: `identifier`, value: " << elem.as<std::string>() << std::endl;
                break;
            case 2:
                std::cout << "Type: `hex int`, value: " << elem.as<uint64_t>() << std::endl;
                break;
            case 3:
                std::cout << "Type: `int`, value: " << elem.as<uint64_t>() << std::endl;
                break;
            case 4:
                std::cout << "Type: `text`, value: " << elem.as<std::string>() << std::endl;
                break;
            case 5:
                std::cout << "Type: `white space`" << std::endl;
        }
    }

    auto ident_parser = par::token(ident);
    auto hex_parser = par::token(hex);

    auto alternative = ident_parser | hex_parser;

    t = lex::tokenize("0x1000", desc);

    auto begin = t.cbegin();
    auto x = alternative.match(begin, t.end());

    std::cout << *x << std::endl;

    t = lex::tokenize("foobar1_", desc);

    begin = t.cbegin();
    x = alternative.match(begin, t.end());

    std::cout << *x << std::endl;

    auto sequence = +alternative;
    t = lex::tokenize("0x1 foo 0x2 foo 0x3 bar 0x4 bar 0x5", desc);

    begin = t.cbegin();
    auto y = sequence.match(begin, t.cend(), par::token<std::string>(desc[5]));

    std::cout << " ---- " << std::endl;
    for (auto & val : *y)
    {
        std::cout << val << std::endl;
    }
    std::cout << " ---- " << std::endl;

    auto hex_no_string = hex_parser - ident_parser;

    begin = t.cbegin();
    auto z = hex_no_string.match(begin, t.cend(), par::token<std::string>(desc[5]));

    if (z)
    {
        std::cout << *z << " (shouldn't've been matched)" << std::endl;
    }

    else
    {
        std::cout << "no match (correct!)" << std::endl;
    }

    begin = t.cbegin();
    auto many_blocks = *(hex_parser >> ident_parser) >> hex_parser;
    auto w = many_blocks.match(begin, t.cend(), par::token<std::string>(desc[5]));

    if (w)
    {
        std::cout << " - number of pairs: " << std::get<0>(*w).size() << std::endl;
        std::cout << " - last number: " << std::get<1>(*w) << std::endl;
    }

    else
    {
        std::cout << "no match (wrong this time)" << std::endl;
    }

    struct op_desc
    {
        uint64_t first;
        std::string op;
        uint64_t second;
    };

    t = tokenize("0x1 + 0x2", desc);
    auto operation = par::token(op);
    par::rule<op_desc> expression = hex_parser >> operation >> hex_parser;

    begin = t.cbegin();
    auto foo = expression.match(begin, t.cend(), par::token<std::string>(desc[5]));

    t = lex::tokenize("0x0 0x1", desc);
    par::rule<std::vector<uint64_t>> seq = hex_parser >> hex_parser;

    begin = t.cbegin();
    auto vec = seq.match(begin, t.cend(), par::token<std::string>(desc[5]));

    std::cout << " ----" << std::endl;
    for (auto & elem : *vec)
    {
        std::cout << elem << std::endl;
    }
    std::cout << " ----" << std::endl;
}
