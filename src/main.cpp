#include "common.h"
#include "ui/Application.h"

int main(int argc, char* argv[])
{
    init_paths();
    return lcs::ui::start(argc, argv);
}
