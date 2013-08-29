#include <reaver/parser.h>

template<typename T>
void foo(const T &)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

namespace lex = reaver::lexer;
namespace par = reaver::parser;
using namespace reaver::logger;

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

    for (lex::iterator t{ "identifier and then hexnumber 0x1000 \"quoted \\\"string\" new line\n and then decnumber 1000", desc };
        t != lex::iterator{}; ++t)
    {
        switch (t->type())
        {
            case 1:
                dlog() << "Type: `identifier`, value: " << t->as<std::string>();
                break;
            case 2:
                dlog() << "Type: `hex int`, value: " << t->as<uint64_t>();
                break;
            case 3:
                dlog() << "Type: `int`, value: " << t->as<uint64_t>();
                break;
            case 4:
                dlog() << "Type: `text`, value: " << t->as<std::string>();
                break;
            case 5:
                dlog() << "Type: `white space`";
        }
    }

    auto ident_parser = par::token(ident);
    auto hex_parser = par::token(hex);

    lex::iterator it;
    lex::iterator orig{ "0x1 foo 0x2 foo 0x3 bar 0x4 bar 0x5", desc };

    {
        auto alternative = ident_parser | hex_parser;

        it = lex::iterator{ "0x1000", desc };
        auto x = alternative.match(it, lex::iterator{});
        if (x)
        {
            std::cout << *x << std::endl;
        }

        it = lex::iterator{ "foobar1_", desc };
        x = alternative.match(it, lex::iterator{});
        if (x)
        {
            std::cout << *x << std::endl;
        }

        it = orig;
        auto sequence = +alternative;
        auto y = sequence.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        std::cout << " ---- " << std::endl;
        for (auto & val : *y)
        {
            std::cout << val << std::endl;
        }
        std::cout << " ---- " << std::endl;

        auto opt = -hex_parser >> ident_parser >> hex_parser;
        auto z = opt.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        if (z)
        {
            std::cout << "optional #1: matched" << std::endl;
        }

        auto opt2 = -ident_parser >> hex_parser;

        auto w = opt2.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        if (w)
        {
            std::cout << "optional #2: matched" << std::endl;
        }

        auto opt3 = hex_parser >> -hex_parser >> ident_parser;

        auto v = opt3.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        if (v)
        {
            std::cout << "optional #3: matched" << std::endl;
        }
    }

    {
        it = orig;

        auto many_blocks = *(hex_parser >> ident_parser) >> hex_parser;
        auto w = many_blocks.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

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
        it = lex::iterator{ "0x0 0x1", desc };
        par::rule<std::vector<uint64_t>> seq = hex_parser >> hex_parser;

        auto vec = seq.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        std::cout << " ----" << std::endl;
        for (auto & elem : *vec)
        {
            std::cout << elem << std::endl;
        }
        std::cout << " ----" << std::endl;

        it = lex::iterator{ "0x1 + 0x2", desc };
        auto operation = par::token(op);
        par::rule<op_desc> expression = hex_parser >> operation >> hex_parser;

        auto i = it;
        auto foo = expression.match(i, lex::iterator{}, par::token<std::string>(desc[5]));

        std::cout << "op_desc = { .first = " << foo->first << ", .op = " << foo->op << ", .second = " << foo->second << " }" << std::endl;

        i = it;
        par::rule<std::vector<uint64_t>> expr_with_discarded = hex_parser >> ~operation >> hex_parser;
        auto bar = expr_with_discarded.match(i, lex::iterator{}, par::token<std::string>(desc[5]));

        std::cout << bar->at(0) << ", " << bar->at(1) << std::endl;
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

        auto bar = expr.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        std::cout << *bar << std::endl;

        it = lex::iterator{ "0x1 + 0x2 * 0x3 + 0x4", desc };
        bar = expr.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        if (bar)
        {
            std::cout << *bar << std::endl;
        }

        else
        {
            std::cout << "not mached (wrong)" << std::endl;
        }
    }

    {
        par::rule<std::vector<uint64_t>> r = hex_parser % par::token(op);

        it = lex::iterator{ "0x1 + 0x2 + 0x3 + 0x4", desc };
        auto ret = r.match(it, lex::iterator{}, par::token<std::string>(desc[5]));

        if (ret)
        {
            for (const auto & x : *ret)
            {
                std::cout << x << ", ";
            }
            std::cout << std::endl;
        }

        else
        {
            std::cout << "list test failed" << std::endl;
        }
    }
}
