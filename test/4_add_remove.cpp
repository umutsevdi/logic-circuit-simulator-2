#include "catch2/catch_test_macros.hpp"
#include "core.h"
#include <catch2/catch_all.hpp>

using namespace lcs;
TEST_CASE("Basic Add Remove Re-add Test")
{
    Scene s;
    auto v = s.add_node<InputNode>();
    auto o = s.add_node<OutputNode>();
    auto r = s.connect(o, 0, v);
    REQUIRE(r != 0);
    s.get_node<InputNode>(v)->set(true);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    s.remove_node(v);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::DISABLED);
    v = s.add_node<InputNode>();
    REQUIRE(s.connect(o, 0, v) != 0);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::FALSE);
    s.get_node<InputNode>(v)->set(true);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
}

TEST_CASE("Readd Gate Test")
{
    Scene s;
    auto v     = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    REQUIRE(s.connect(o, 0, g_and));

    auto r = s.connect(g_and, 0, v);
    REQUIRE(r != 0);
    REQUIRE(s.connect(g_and, 1, v2));
    s.get_node<InputNode>(v)->set(true);
    s.get_node<InputNode>(v2)->set(false);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::FALSE);

    s.remove_node(g_and);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::DISABLED);

    auto g_or = s.add_node<GateNode>(gate_t::OR);
    REQUIRE(s.connect(o, 0, g_or));
    REQUIRE(s.connect(g_or, 0, v));
    REQUIRE(s.connect(g_or, 1, v2));

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
}

TEST_CASE("Gate State Change After Removal")
{
    Scene s;
    auto g_or  = s.add_node<GateNode>(gate_t::OR);
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    auto v1    = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto v3    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();

    REQUIRE(s.connect(g_or, 0, v1) != 0);
    REQUIRE(s.connect(g_or, 1, v2) != 0);
    REQUIRE(s.connect(g_and, 0, v3) != 0);
    REQUIRE(s.connect(g_and, 1, v1) != 0);
    REQUIRE(s.connect(o, 0, g_or) != 0);

    s.get_node<InputNode>(v1)->set(true);
    s.get_node<InputNode>(v2)->set(false);
    s.get_node<InputNode>(v3)->set(true);

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    s.remove_node(g_or);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::DISABLED);

    g_or = s.add_node<GateNode>(gate_t::OR);
    REQUIRE(s.connect(g_or, 0, v1) != 0);
    REQUIRE(s.connect(g_or, 1, v2) != 0);
    REQUIRE(s.connect(o, 0, g_or) != 0);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);

    s.get_node<InputNode>(v1)->set(false);
    s.get_node<InputNode>(v1)->set(false);

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::FALSE);
}
