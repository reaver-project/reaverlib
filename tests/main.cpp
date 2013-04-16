#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

struct op_desc
{
    uint64_t first;
    std::string op;
    uint64_t second;
};

struct expression;

struct operation
{
    std::string op;
    boost::variant<uint64_t, boost::recursive_wrapper<expression>> operand;
};

struct expression
{
    boost::variant<uint64_t, boost::recursive_wrapper<expression>> first;
    std::vector<operation> ops;
};

std::ostream & operator<<(std::ostream & o, const expression & r)
{
    o << r.first;
    for (const auto & x : r.ops)
    {
        o << x.op << x.operand;
    }
    return o;
}

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

    {
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
    }

    {
        auto hex_no_string = hex_parser - ident_parser;

        auto begin = t.cbegin();
        auto z = hex_no_string.match(begin, t.cend(), par::token<std::string>(desc[5]));

        if (z)
        {
            std::cout << *z << " (shouldn't've been matched)" << std::endl;
        }

        else
        {
            std::cout << "no match (correct!)" << std::endl;
        }
    }

    {
        auto begin = t.cbegin();
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
    }

    {
        t = lex::tokenize("0x0 0x1", desc);
        par::rule<std::vector<uint64_t>> seq = hex_parser >> hex_parser;

        auto begin = t.cbegin();
        auto vec = seq.match(begin, t.cend(), par::token<std::string>(desc[5]));

        std::cout << " ----" << std::endl;
        for (auto & elem : *vec)
        {
            std::cout << elem << std::endl;
        }
        std::cout << " ----" << std::endl;

        t = tokenize("0x1 + 0x2", desc);
        auto operation = par::token(op);
        par::rule<op_desc> expression = hex_parser >> operation >> hex_parser;

        begin = t.cbegin();
        auto foo = expression.match(begin, t.cend(), par::token<std::string>(desc[5]));

        std::cout << "op_desc = { .first = " << foo->first << ", .op = " << foo->op << ", .second = " << foo->second << " }" << std::endl;

        begin = t.cbegin();
        par::rule<std::pair<uint64_t, uint64_t>> expr_with_discarded = hex_parser >> ~operation >> hex_parser;
        auto bar = expr_with_discarded.match(begin, t.cend(), par::token<std::string>(desc[5]));

        std::cout << bar->first << ", " << bar->second << std::endl;
    }

    {
        par::rule<expression> expr;
        par::rule<expression> term;
        par::rule<operation> plusop;
        par::rule<operation> timesop;

        auto plus = par::token(op)({ "+" });
        auto times = par::token(op)({ "*" });

        plusop = plus >> term;
        timesop = times >> hex_parser;

        expr = term >> *plusop;
        term = hex_parser >> *timesop;

        auto begin = t.cbegin();
        auto bar = expr.match(begin, t.cend(), par::token<std::string>(desc[5]));

        std::cout << *bar << std::endl;

        t = lex::tokenize("0x1 + 0x2 * 0x3 + 0x4", desc);
        begin = t.cbegin();
        bar = expr.match(begin, t.cend(), par::token<std::string>(desc[5]));

        if (bar)
        {
            std::cout << *bar << std::endl;
        }

        else
        {
            std::cout << "not mached (wrong)" << std::endl;
        }
    }
}
