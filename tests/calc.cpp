#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

namespace calculator
{
    struct addition
    {
        int64_t first, second;

        operator int64_t()
        {
            return first + second;
        }
    };

    struct subtraction
    {
        int64_t first, second;

        operator int64_t()
        {
            return first - second;
        }
    };

    struct multiplication
    {
        int64_t first, second;

        operator int64_t()
        {
            return first * second;
        }
    };

    struct division
    {
        int64_t first, second;

        operator int64_t()
        {
            return first / second;
        }
    };
}

int main()
{
    lex::tokens_description desc;

    lex::token_definition<int64_t> integer_token{ 1, "[\\+\\-]?[0-9]+" };
    lex::token_definition<int64_t> hex_integer_token{ 2, "0x[0-9a-fA-F]+", [](const std::string & str)
        {
            uint64_t i;
            std::stringstream s{ str };
            s >> i;
            return i;
        }
    };

//    lex::token_definition<long double> fp_token; // TODO

    lex::token_definition<std::string> operation_token{ 4, "[\\+\\-\\*/]" };
//    lex::token_definition<std::string> parens_token; // TODO

    lex::token_definition<std::string> ignore{ 6, ".*" };   // used last, so can match anything

    desc.add(integer_token)(hex_integer_token)(operation_token)(ignore);

    auto integer = par::token(integer_token);
    auto hex = par::token(hex_integer_token);

    auto plus = par::token(operation_token)({ "+" });
    auto minus = par::token(operation_token)({ "-" });
    auto star = par::token(operation_token)({ "*" });
    auto slash = par::token(operation_token)({ "/" });

    par::rule<calculator::addition> addition;
    par::rule<calculator::subtraction> subtraction;
    par::rule<calculator::multiplication> multiplication;
    par::rule<calculator::division> division;

    par::rule<int64_t> number = integer | hex;

    par::rule<int64_t> term;
    par::rule<int64_t> factor;

    addition = factor >> ~plus >> factor;
    subtraction = factor >> ~minus >> factor;
    multiplication = term >> ~star >> term;
    division = term >> ~slash >> term;

    factor = number | multiplication | division;
    term = addition | subtraction;

    while (true)
    {
        std::string buf;
        std::getline(std::cin, buf);
    }
}
