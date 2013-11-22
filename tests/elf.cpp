#include <reaver/format/executable/executable.h>

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        return 0;
    }

    try
    {
        auto foo = reaver::format::executable::read(std::fstream{ argv[1], std::ios::binary | std::ios::in });
    }

    catch (reaver::exception & e)
    {
        e.print(reaver::logger::dlog);
    }

    catch (std::exception & e)
    {
        reaver::logger::dlog(reaver::logger::error) << e.what();
    }
}
