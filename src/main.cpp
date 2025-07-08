#if __TESTING__
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#else
#include "net.h"
#include "ui.h"
#endif
#include "common.h"

using namespace lcs;
int main(int argc, char* argv[])
{
    init_paths(__TESTING__);
#if __TESTING__
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
#else
    net::init();
    return ui::main(argc, argv);
#endif
}
