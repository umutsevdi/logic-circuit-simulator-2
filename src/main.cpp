#include <cstdlib>
#ifndef __TESTING__
#define __TESTING__ 0
#endif

#if __TESTING__
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#endif

#include "common.h"
#include "port.h"

namespace lcs::ui {
int main(int, char**);
}
using namespace lcs;
static void _cleanup(void)
{
    fs::close();
    net::close();
}

int main(int argc, char* argv[])
{
    fs::init(__TESTING__);
    net::init(__TESTING__);
    std::atexit(_cleanup);
#if __TESTING__
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
#else
    return ui::main(argc, argv);
#endif
}
