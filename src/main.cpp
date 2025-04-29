#include "ui.h"
#include "common.h"

int main(int argc, char* argv[])
{
    init_paths();
    return lcs::ui::main(argc, argv);
}
