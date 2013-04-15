#include <reaver/parser/parser.h>

namespace lex = reaver::lexer;
namespace par = reaver::parser;

struct op_desc
{
    uint64_t first;
    std::string op;
    uint64_t second;
};

struct recursive_op_desc
{
    boost::variant<boost::recursive_wrapper<recursive_op_desc>, uint64_t> first;
    std::string op;
    boost::variant<boost::recursive_wrapper<recursive_op_desc>, uint64_t> second;
};

std::ostream & operator<<(std::ostream & o, const recursive_op_desc & r)
{
    return o << r.first << r.op << r.second;
}

int main()
{
    lex::tokens_description desc;

    lex::token_definition<uint64_t> hex{ 2, "0x[0-9a-fA-F]+", [](const std::string & str)
    {
        uint64_t a; std::stringstream(str) >> std::hex >> a; return a;
    } };

    lex::token_definition<std::string> ident{ 1, "[a-zA-Z_][a-zA-Z0-9_]*" };

// soon:    lex::token_definition<std::string> op{ 6, "[\\+\\-\\*\\/]" };
    lex::token_definition<std::string> plus{ 6, "\\+" };
    lex::token_definition<std::string> star{ 7, "\\*" };

    desc.add(ident)
        (hex)
        (3, "[0-9]+", lex::match_type<uint64_t>{})
        (4, "\"([^\"\\\\]|\\\\.)*\"")
        (5, "[ \n]")
        (plus)
        (star);

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

//    t = tokenize("0x1 + 0x2", desc);
//    auto operation = par::token(op);
//    par::rule<op_desc> expression = hex_parser >> operation >> hex_parser;

//    begin = t.cbegin();
//    auto foo = expression.match(begin, t.cend(), par::token<std::string>(desc[5]));

//    std::cout << "op_desc = { .first = " << foo->first << ", .op = " << foo->op << ", .second = " << foo->second << " }" << std::endl;

    par::rule<recursive_op_desc> expr;
    par::rule<recursive_op_desc> term;
    par::rule<recursive_op_desc> factor;

    expr = term >> *(par::token(plus) >> term);
    term = factor >> *(par::token(star) >> factor);
    factor = hex_parser;

    begin = t.begin();
    auto bar = expr.match(begin, t.cend(), par::token<std::string>(desc[5]));

    std::cout << *bar << std::endl;

    t = lex::tokenize("0x1 + 0x2 * 0x3 - 0x4", desc);
    begin = t.begin();
    bar = expr.match(begin, t.cend(), par::token<std::string>(desc[5]));

    std::cout << *bar << std::endl;
}
