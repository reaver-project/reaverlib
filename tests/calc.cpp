#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

namespace calculator
{
    struct addition
    {
        int64_t first, second;

        operator int64_t() const
        {
            return first + second;
        }
    };

    struct subtraction
    {
        int64_t first, second;

        operator int64_t() const
        {
            return first - second;
        }
    };

    struct multiplication
    {
        int64_t first, second;

        operator int64_t() const
        {
            return first * second;
        }
    };

    struct division
    {
        int64_t first, second;

        operator int64_t() const
        {
            return first / second;
        }
    };
}

int main()
{
    lex::tokens_description desc;

    lex::token_definition<int64_t> hex_integer_token{ 1, "0x[0-9a-fA-F]+", [](const std::string & str)
        {
            uint64_t i;
            std::stringstream s{ str };
            s >> std::hex >> i;
            return i;
        }
    };
    lex::token_definition<int64_t> integer_token{ 2, "[0-9]+" };

//    lex::token_definition<long double> fp_token; // TODO

    lex::token_definition<std::string> operation_token{ 4, "[\\+\\-\\*\\/]" };
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

    // FIXME: current set of rules fails miserably at operator precendence
    // find some smart way to fix it

    par::rule<int64_t> number = integer | hex;

    par::rule<int64_t> operation;
    par::rule<int64_t> expr;

    par::rule<int64_t> lower = addition | subtraction;
    par::rule<int64_t> higher = multiplication | division;

    addition = number >> ~plus >> expr;
    subtraction = number >> ~minus >> expr;
    multiplication = number >> ~star >> expr;
    division = number >> ~slash >> expr;

    operation = addition | subtraction | multiplication | division;
    expr = operation | number;

    while (true)
    {
        std::string buf;
        std::getline(std::cin, buf);

        auto t = lex::tokenize(buf, desc);
        auto begin = t.cbegin();
        auto result = expr.match(begin, t.cend(), par::token(ignore));

        if (result && begin == t.cend())
        {
            std::cout << "Result: " << *result << std::endl;
        }

        else
        {
            std::cout << "Syntax error, try again." << std::endl;
        }
    }
}
