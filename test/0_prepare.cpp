#include "io.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char** argv)
{
    lcs::io::init_paths(true);
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}
