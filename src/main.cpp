#if __TESTING__
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#else
namespace ui {
extern int main(int argc, char* argv[]);
}
#endif
#include "common.h"

using namespace lcs;
int main(int argc, char* argv[])
{
    init_paths(
#ifdef __TESTING__
        true);
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
#else
    );
    net::init();
    return ui::main(argc, argv);
#endif
}
