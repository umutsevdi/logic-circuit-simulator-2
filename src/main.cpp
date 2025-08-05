#if __TESTING__
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#endif
#include "common.h"
#include "port.h"

int main(int argc, char* argv[])
{
    using namespace lcs;
    fs::init(__TESTING__);
    net::init(__TESTING__);
#ifdef __TESTING__
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int result = context.run();
#else
    int result = ui::main(argc, argv);
#endif
    fs::close();
    net::close();
    return result;
}
