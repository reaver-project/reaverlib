#include <future>

#include <reaver/logger.h>

using namespace reaver::logger;
using namespace reaver::style;

int main()
{
    log.add_stream(stream_wrapper(std::make_shared<std::fstream>("tests/test.out", std::ios::out)));

    auto x = std::thread([]()
    {
        log(debug) << "Hello!";
        log() << "Always printed.";
        log(warning) << "Something is probably going wrong.";
        log(error) << "Something bad happened.";
        log(crash) << "Oh, seriously?";
        log() << style(colors::bgreen, colors::def, styles::bold) << "Success: " << style() << "This went good.";
    });

    auto y = std::thread([]()
    {
        log(debug) << "Hello!";
        log() << "Always printed.";
        log(warning) << "Something is probably going wrong.";
        log(error) << "Something bad happened.";
        log(crash) << "Oh, seriously?";
        log() << style(colors::bgreen, colors::def, styles::bold) << "Success: " << style() << "This went good.";
    });

    x.join();
    y.join();
}
