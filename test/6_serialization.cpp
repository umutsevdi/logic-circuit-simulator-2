#include "core.h"
#include "parse.h"
#include <catch2/catch_all.hpp>

using namespace lcs;

TEST_CASE("Parse JSON subnodes")
{
    Scene s { "Parse JSON subnodes" };

    auto g_or  = s.add_node<Gate>(gate_t::OR);
    auto g_and = s.add_node<Gate>(gate_t::AND);
    auto v1    = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto v3    = s.add_node<Input>();
    auto o     = s.add_node<Output>();

    s.connect(g_or, 0, v1);
    s.connect(g_or, 1, v2);
    s.connect(g_and, 0, v3);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_or);
    s.get_node<Input>(v1)->set(true);
    s.get_node<Input>(v2)->set(false);
    s.get_node<Input>(v3)->set(true);
    Json::Value v = parse::to_json(s);
    REQUIRE(!v.empty());
}

TEST_CASE("Parse non-zero context")
{
    Scene s { "Parse non-zero context" };

    auto g_and = s.add_node<Gate>(gate_t::AND);
    auto v1    = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto o     = s.add_node<Output>();
    s.get_node<Input>(v1)->set_freq(3);

    s.connect(g_and, 0, v2);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_and);
    s.get_base(o)->dir    = direction_t::DOWN;
    s.get_base(v1)->point = { 1, 3 };

    Json::Value out = parse::to_json(s);
    REQUIRE(!out.empty());
}

TEST_CASE("Save, load and compare")
{
    Scene s { "Save, load and compare" };
    auto v = s.add_node<Input>();
    auto o = s.add_node<Output>();
    s.get_node<Input>(v)->set(true);
    s.connect(o, 0, v);

    Json::Value scene_str = parse::to_json(s);
    Scene s_loaded;
    REQUIRE(parse::from_json(scene_str, s_loaded) == parse::OK);

    Json::Value scene_loaded_str = parse::to_json(s_loaded);

    REQUIRE(scene_str.toStyledString() == scene_loaded_str.toStyledString());
    REQUIRE(s_loaded.get_node<Output>(o)->get() == state_t::TRUE);
}

TEST_CASE("Save a complicated JSON, load and run the tests")
{

    // Taken from TEST_CASE("Full Adder")
    Scene s { "Save a complicated JSON, load and run the tests" };
    auto a     = s.add_node<Input>();
    auto b     = s.add_node<Input>();
    auto c_in  = s.add_node<Input>();
    auto c_out = s.add_node<Output>();
    auto sum   = s.add_node<Output>();

    auto g_xor       = s.add_node<Gate>(gate_t::XOR);
    auto g_xor_sum   = s.add_node<Gate>(gate_t::XOR);
    auto g_and_carry = s.add_node<Gate>(gate_t::AND);
    auto g_and       = s.add_node<Gate>(gate_t::AND);
    auto g_or        = s.add_node<Gate>(gate_t::OR);

    s.connect(g_xor, 0, a);
    s.connect(g_xor, 1, b);
    s.connect(g_xor_sum, 0, g_xor);
    s.connect(g_xor_sum, 1, c_in);
    s.connect(sum, 0, g_xor_sum);
    s.connect(g_and, 0, c_in);
    s.connect(g_and, 1, g_xor);
    s.connect(g_and_carry, 0, a);
    s.connect(g_and_carry, 1, b);
    s.connect(g_or, 0, g_and);
    s.connect(g_or, 1, g_and_carry);
    s.connect(c_out, 0, g_or);
    // Taken from TEST_CASE("Full Adder")

    Json::Value s_str = parse::to_json(s);
    Scene s_loaded {};

    REQUIRE(parse::from_json(s_str, s_loaded) == parse::error_t::OK);

    // Test cases from TEST_CASE("Full Adder")
    s_loaded.get_node<Input>(a)->set(true);
    s_loaded.get_node<Input>(b)->set(false);
    s_loaded.get_node<Input>(c_in)->set(false);
    REQUIRE(s_loaded.get_node<Output>(sum)->get() == state_t::TRUE);
    REQUIRE(s_loaded.get_node<Output>(c_out)->get() == state_t::FALSE);

    s_loaded.get_node<Input>(b)->set(true);
    REQUIRE(s_loaded.get_node<Output>(sum)->get() == state_t::FALSE);
    REQUIRE(s_loaded.get_node<Output>(c_out)->get() == state_t::TRUE);

    s_loaded.get_node<Input>(c_in)->set(true);
    REQUIRE(s_loaded.get_node<Output>(sum)->get() == state_t::TRUE);
    REQUIRE(s_loaded.get_node<Output>(c_out)->get() == state_t::TRUE);
}

TEST_CASE("Save a component")
{
    // Taken from "Create a 2x1 MUX component"
    // IN1 = 1, IN2 = 2, IN3 = SEL
    Scene s { ComponentContext { 3, 1 }, "Create a 2x1 MUX component" };
    auto g_and   = s.add_node<Gate>(gate_t::AND);
    auto g_and_2 = s.add_node<Gate>(gate_t::AND);
    auto g_not   = s.add_node<Gate>(gate_t::NOT);
    auto g_out   = s.add_node<Gate>(gate_t::OR);

    s.connect(g_and, 0, s.component_context->get_input(1));
    s.connect(g_and_2, 0, s.component_context->get_input(2));
    s.connect(g_and, 1, s.component_context->get_input(3));
    s.connect(g_not, 0, s.component_context->get_input(3));
    s.connect(g_and_2, 1, g_not);
    s.connect(g_out, 0, g_and);
    s.connect(g_out, 1, g_and_2);
    s.connect(s.component_context->get_output(1), 0, g_out);

    REQUIRE(s.component_context->run(&s, 0b111) == 1);
    REQUIRE(s.component_context->run(&s, 0b110) == 0);
    REQUIRE(s.component_context->run(&s, 0b101) == 1);
    REQUIRE(s.component_context->run(&s, 0b100) == 0);
    REQUIRE(s.component_context->run(&s, 0b011) == 1);
    REQUIRE(s.component_context->run(&s, 0b010) == 1);
    REQUIRE(s.component_context->run(&s, 0b001) == 0);
    REQUIRE(s.component_context->run(&s, 0b000) == 0);

    Json::Value s_str = parse::to_json(s);
    Scene s_loaded {};
    REQUIRE(parse::from_json(s_str, s_loaded) == parse::error_t::OK);
    Json::Value scene_loaded_str = parse::to_json(s_loaded);

    REQUIRE(s_str.toStyledString() == scene_loaded_str.toStyledString());

    REQUIRE(s.component_context->run(&s_loaded, 0b111) == 1);
    REQUIRE(s.component_context->run(&s_loaded, 0b110) == 0);
    REQUIRE(s.component_context->run(&s_loaded, 0b101) == 1);
    REQUIRE(s.component_context->run(&s_loaded, 0b100) == 0);
    REQUIRE(s.component_context->run(&s_loaded, 0b011) == 1);
    REQUIRE(s.component_context->run(&s_loaded, 0b010) == 1);
    REQUIRE(s.component_context->run(&s_loaded, 0b001) == 0);
    REQUIRE(s.component_context->run(&s_loaded, 0b000) == 0);
}
