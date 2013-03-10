#include <reaver/logger.h>

using namespace reaver::logger;

int main()
{
    log(debug) << "Hello!";
    log() << "Always printed.";
    log(warning) << "Something is probably going wrong.";
    log(error) << "Something bad happened.";
    log(crash) << "Oh, seriously?";
}
