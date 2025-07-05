#include "io.h"
#include "net.h"
#include "ui.h"

using namespace lcs;
int main(int argc, char* argv[])
{
    init_paths();
    net::init();
    return ui::main(argc, argv);
}
