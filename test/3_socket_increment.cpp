#include "catch2/catch_test_macros.hpp"
#include "common.h"
#include "core.h"
#include <catch2/catch_all.hpp>

using namespace lcs;
TEST_CASE("Increment socket size, expect update")
{
    Scene s;

    auto v     = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    auto o     = s.add_node<OutputNode>();

    s.get_node<InputNode>(v)->set(true);
    s.get_node<InputNode>(v2)->set(true);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v2));

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    s.get_node<GateNode>(g_and)->increment();
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::DISABLED);
    s.get_node<GateNode>(g_and)->decrement();
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
}
