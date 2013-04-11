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

    desc.add(ident)
        (hex)
        (3, "[0-9]+", lex::match_type<uint64_t>{})
        (4, "\"([^\"\\\\]|\\\\.)*\"")
        (5, "[ \n]");

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

    auto sequence = +par::token(hex);
    t = lex::tokenize("0x1 0x2 0x3 0x4 0x5", desc);

    begin = t.cbegin();
    auto y = sequence.match(begin, t.cend(), par::token<std::string>(desc[5]));

    std::cout << " ---- " << std::endl;
    for (auto & val : *y)
    {
        std::cout << val << std::endl;
    }
    std::cout << " ---- " << std::endl;
}
