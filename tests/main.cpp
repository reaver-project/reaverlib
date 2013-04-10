#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

int main()
{
    lex::tokens_description desc;

    lex::token_definition<uint64_t> def{ 2, "0x[0-9a-fA-F]+", [](const std::string & str)
    {
        uint64_t a; std::stringstream(str) >> std::hex >> a; return a;
    } };

    desc.add(1, "[a-zA-Z_][a-zA-Z0-9_]*")
        (def)
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

    par::rule<uint8_t> a = par::token(def);
    t = lex::tokenize("0x1000", desc);

    auto begin = t.begin();
    auto x = a.match(begin, t.end());

    if (begin != t.end() || !x)
    {
        std::cout << "Something went wrong." << std::endl;
    }

    else
    {
        std::cout << (uint32_t)*x << std::endl;
    }
}
