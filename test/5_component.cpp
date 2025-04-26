#include "core.h"
#include <doctest.h>

using namespace lcs;

TEST_CASE("Create a component context")
{
    Scene s { ComponentContext { 3, 1 }, "Create a component context" };
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    s.get_node<GateNode>(g_and)->increment();

    REQUIRE(s.component_context.has_value());
    REQUIRE(s.connect(g_and, 0, s.component_context->get_input(1)));
    REQUIRE(s.connect(g_and, 1, s.component_context->get_input(2)));
    REQUIRE(s.connect(g_and, 2, s.component_context->get_input(3)));
    REQUIRE(s.connect(s.component_context->get_output(1), 0, g_and));

    SUBCASE("Run 3x AND Gate")
    {
        REQUIRE_EQ(s.component_context->run(&s, 0b111), 1);
        REQUIRE_EQ(s.component_context->run(&s, 0b110), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b101), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b100), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b011), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b010), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b001), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b00), 0);
    }
}

TEST_CASE("Create a 2x1 MUX component")
{
    // IN1 = 1, IN2 = 2, IN3 = SEL
    Scene s { ComponentContext { 3, 1 }, "Create a 2x1 MUX component" };
    auto g_and   = s.add_node<GateNode>(gate_t::AND);
    auto g_and_2 = s.add_node<GateNode>(gate_t::AND);
    auto g_not   = s.add_node<GateNode>(gate_t::NOT);
    auto g_out   = s.add_node<GateNode>(gate_t::OR);

    s.connect(g_and, 0, s.component_context->get_input(1));
    s.connect(g_and_2, 0, s.component_context->get_input(2));
    s.connect(g_and, 1, s.component_context->get_input(3));
    s.connect(g_not, 0, s.component_context->get_input(3));
    s.connect(g_and_2, 1, g_not);
    s.connect(g_out, 0, g_and);
    s.connect(g_out, 1, g_and_2);
    s.connect(s.component_context->get_output(1), 0, g_out);

    SUBCASE("Run 2x1 MUX")
    {
        REQUIRE_EQ(s.component_context->run(&s, 0b111), 1);
        REQUIRE_EQ(s.component_context->run(&s, 0b110), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b101), 1);
        REQUIRE_EQ(s.component_context->run(&s, 0b100), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b011), 1);
        REQUIRE_EQ(s.component_context->run(&s, 0b010), 1);
        REQUIRE_EQ(s.component_context->run(&s, 0b001), 0);
        REQUIRE_EQ(s.component_context->run(&s, 0b000), 0);
    }
}
