#include "catch2/catch_test_macros.hpp"
#include "core.h"
#include "test_util.h"
#include <catch2/catch_all.hpp>

using namespace lcs;
TEST_CASE("Connect IN to OUT, Update")
{
    Scene s;
    auto v = s.add_node<InputNode>();
    auto o = s.add_node<OutputNode>();
    REQUIRE(s.connect(o, 0, v) != 0);

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::FALSE);
    s.get_node<InputNode>(v)->set(true);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
}

TEST_CASE("Connect IN to OUT, update and disconnect")
{
    Scene s;
    auto v = s.add_node<InputNode>();
    auto o = s.add_node<OutputNode>();
    auto r = s.connect(o, 0, v);
    REQUIRE(r != 0);
    s.get_node<InputNode>(v)->set(true);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    s.disconnect(r);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::DISABLED);
}

TEST_CASE("Connect same input to Gate")
{
    Scene s;
    auto v     = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    REQUIRE(s.connect(g_and, 0, v) != 0);
    REQUIRE(s.connect(g_and, 1, v) != 0);
    REQUIRE(s.connect(o, 0, g_and) != 0);

    s.get_node<InputNode>(v)->set(true);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    s.get_node<InputNode>(v)->set(false);
    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::FALSE);
}

TEST_CASE("[[v1,v2]->and, v2]->or->o")
{
    Scene s;
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    auto g_or  = s.add_node<GateNode>(gate_t::OR);
    auto v     = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();

    REQUIRE(s.connect(g_and, 0, v) != 0);
    REQUIRE(s.connect(g_and, 1, v2) != 0);
    REQUIRE(s.connect(g_or, 0, g_and) != 0);
    REQUIRE(s.connect(g_or, 1, v) != 0);
    REQUIRE(s.connect(o, 0, g_or) != 0);

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::FALSE);

    s.get_node<InputNode>(v)->set(true);

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    REQUIRE(s.get_node<GateNode>(g_and)->get() == state_t::FALSE);
    REQUIRE(s.get_node<GateNode>(g_or)->get() == state_t::TRUE);
}

TEST_CASE("Multiple Inputs with AND/OR Gates")
{
    Scene s;
    auto g_and  = s.add_node<GateNode>(gate_t::AND);
    auto g_or   = s.add_node<GateNode>(gate_t::OR);
    auto g_nand = s.add_node<GateNode>(gate_t::NAND);
    auto g_nor  = s.add_node<GateNode>(gate_t::NOR);
    auto v1     = s.add_node<InputNode>();
    auto v2     = s.add_node<InputNode>();
    auto v3     = s.add_node<InputNode>();
    auto o      = s.add_node<OutputNode>();

    REQUIRE(s.connect(g_and, 0, v1) != 0);
    REQUIRE(s.connect(g_and, 1, v2) != 0);
    REQUIRE(s.connect(g_or, 0, v1) != 0);
    REQUIRE(s.connect(g_or, 1, v2) != 0);
    REQUIRE(s.connect(g_nand, 0, v2) != 0);
    REQUIRE(s.connect(g_nand, 1, v3) != 0);
    REQUIRE(s.connect(g_nor, 0, v3) != 0);
    REQUIRE(s.connect(g_nor, 1, v1) != 0);
    REQUIRE(s.connect(o, 0, g_or) != 0);

    s.get_node<InputNode>(v1)->set(true);
    s.get_node<InputNode>(v2)->set(true);
    s.get_node<InputNode>(v3)->set(false);

    REQUIRE(s.get_node<OutputNode>(o)->get() == state_t::TRUE);
    REQUIRE(s.get_node<GateNode>(g_and)->get() == state_t::TRUE);
    REQUIRE(s.get_node<GateNode>(g_or)->get() == state_t::TRUE);
    REQUIRE(s.get_node<GateNode>(g_nand)->get() == state_t::TRUE);
    REQUIRE(s.get_node<GateNode>(g_nor)->get() == state_t::FALSE);
}

TEST_CASE("1-Bit Adder Circuit")
{
    Scene s;
    auto v1   = s.add_node<InputNode>();
    auto v2   = s.add_node<InputNode>();
    auto gate = s.add_node<GateNode>(gate_t::OR);
    auto sum  = s.add_node<OutputNode>();

    REQUIRE(s.connect(gate, 0, v1) != 0);
    REQUIRE(s.connect(gate, 1, v2) != 0);
    REQUIRE(s.connect(sum, 0, gate) != 0);

    s.get_node<InputNode>(v1)->set(true);
    s.get_node<InputNode>(v2)->set(false);
    REQUIRE(s.get_node<OutputNode>(sum)->get() == state_t::TRUE); // 1 + 0 = 1

    s.get_node<InputNode>(v1)->set(false);                        // v1 = 0
    s.get_node<InputNode>(v2)->set(true);                         // v2 = 1
    REQUIRE(s.get_node<OutputNode>(sum)->get() == state_t::TRUE); // 0 + 1 = 1

    s.get_node<InputNode>(v1)->set(false);                         // v1 = 0
    s.get_node<InputNode>(v2)->set(false);                         // v2 = 0
    REQUIRE(s.get_node<OutputNode>(sum)->get() == state_t::FALSE); // 0 + 0 = 0
}

TEST_CASE("Full Adder")
{
    Scene s;
    _create_full_adder_io(s);

    auto g_xor       = s.add_node<GateNode>(gate_t::XOR);
    auto g_xor_sum   = s.add_node<GateNode>(gate_t::XOR);
    auto g_and_carry = s.add_node<GateNode>(gate_t::AND);
    auto g_and       = s.add_node<GateNode>(gate_t::AND);
    auto g_or        = s.add_node<GateNode>(gate_t::OR);

    REQUIRE(s.connect(g_xor, 0, a) != 0);
    REQUIRE(s.connect(g_xor, 1, b) != 0);

    REQUIRE(s.connect(g_xor_sum, 0, g_xor) != 0);
    REQUIRE(s.connect(g_xor_sum, 1, c_in) != 0);
    REQUIRE(s.connect(sum, 0, g_xor_sum) != 0);

    REQUIRE(s.connect(g_and, 0, c_in) != 0);
    REQUIRE(s.connect(g_and, 1, g_xor) != 0);

    REQUIRE(s.connect(g_and_carry, 0, a) != 0);
    REQUIRE(s.connect(g_and_carry, 1, b) != 0);

    REQUIRE(s.connect(g_or, 0, g_and) != 0);
    REQUIRE(s.connect(g_or, 1, g_and_carry) != 0);
    REQUIRE(s.connect(c_out, 0, g_or) != 0);

    s.get_node<InputNode>(a)->set(true);
    s.get_node<InputNode>(b)->set(false);
    s.get_node<InputNode>(c_in)->set(false);
    REQUIRE(s.get_node<OutputNode>(sum)->get() == state_t::TRUE);
    REQUIRE(s.get_node<OutputNode>(c_out)->get() == state_t::FALSE);

    s.get_node<InputNode>(b)->set(true);
    REQUIRE(s.get_node<OutputNode>(sum)->get() == state_t::FALSE);
    REQUIRE(s.get_node<OutputNode>(c_out)->get() == state_t::TRUE);

    s.get_node<InputNode>(c_in)->set(true);
    REQUIRE(s.get_node<OutputNode>(sum)->get() == state_t::TRUE);
    REQUIRE(s.get_node<OutputNode>(c_out)->get() == state_t::TRUE);
}
