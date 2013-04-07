#include <reaver/parser/lexer.h>

using namespace reaver::lexer;

int main()
{
    tokens_description desc;

    desc.add(1, "[a-zA-Z_][a-zA-Z0-9_]*")
        (2, "0x[0-9a-fA-F]+", match_type<uint64_t>{}, [](const std::string & str)
        {
            uint64_t a; std::stringstream(str) >> std::hex >> a; return a;
        })
        (3, "[0-9]+", match_type<uint64_t>{})
        (4, " ");

    auto t = tokenize("identifier and then hexnumber 0x1000 and then decnumber 1000", desc);

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
        }
    }
}
