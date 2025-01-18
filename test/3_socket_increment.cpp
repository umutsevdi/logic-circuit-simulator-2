#include "catch2/catch_test_macros.hpp"
#include "common/common.h"
#include "core/engine.h"
#include <catch2/catch_all.hpp>

using namespace lcs;
TEST_CASE("Increment socket size, expect update")
{
    Scene s;

    auto v     = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto g_and = s.add_node<Gate>(gate_t::AND);
    auto o     = s.add_node<Output>();

    s.get_node<Input>(v)->set(true);
    s.get_node<Input>(v2)->set(true);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v2));

    REQUIRE(s.get_node<Output>(o)->get() == state_t::TRUE);
    s.get_node<Gate>(g_and)->increment();
    REQUIRE(s.get_node<Output>(o)->get() == state_t::DISABLED);
    s.get_node<Gate>(g_and)->decrement();
    REQUIRE(s.get_node<Output>(o)->get() == state_t::TRUE);
}
