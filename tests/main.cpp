#include <reaver/style.h>

using namespace reaver::style;

int main()
{
    std::cout << style(colors::bred, colors::def, styles::bold) << "This is bold red text!\n";
}
