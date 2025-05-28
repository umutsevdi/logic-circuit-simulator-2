#include "io.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_CASE("Create Test Environment") { lcs::io::init_paths(true); }
