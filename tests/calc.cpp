#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

int main()
{
    lex::tokens_description desc;

    lex::token_definition<int64_t> integer_token{ 1, "[\\+\\-]?[0-9]+" };
    lex::token_definition<uint64_t> hex_integer_token{ 2, "0x[0-9a-fA-F]+", [](const std::string & str)
        {
            uint64_t i;
            std::stringstream s{ str };
            s >> i;
            return i;
        }
    };
//    lex::token_definition<long double> fp_token; // TODO

    lex::token_definition<std::string> operation_token{ 4, "[\\+\\-\\*/]" };

    lex::token_definition<std::string> ignore{ 5, ".*" };   // used last, so can match anything

    desc.add(integer_token)(hex_integer_token)(operation_token)(ignore);

    while (true)
    {
        std::string buf;
        std::getline(std::cin, buf);
    }
}
