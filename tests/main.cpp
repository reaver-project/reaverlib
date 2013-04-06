#include <future>

#include <reaver/logger.h>
#include <reaver/parser/lexer.h>

using namespace reaver::logger;
using namespace reaver::style;

int main()
{
    std::chrono::system_clock::now();

    dlog.add_stream(stream_wrapper(std::make_shared<std::fstream>("tests/test.out", std::ios::out)));

    std::vector<std::future<void>> v;

    std::string{};

    for (uint64_t i = 0; i < 10000; ++i)
    {
        v.emplace_back(std::async([]()
        {
            dlog(debug) << "Hello!";
            dlog() << "Always printed.";
            dlog(warning) << "Something is probably going wrong.";
            dlog(error) << "Something bad happened.";
            dlog(crash) << "Oh, seriously?";
            dlog() << style(colors::bgreen, colors::def, styles::bold) << "Success: " << style() << "This went good.";
        }));
    }
}
