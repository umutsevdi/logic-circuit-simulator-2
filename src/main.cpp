#include "io.h"
#include "ui.h"

int main(int argc, char* argv[])
{
    lcs::io::init_paths();
    return lcs::ui::main(argc, argv);
}
