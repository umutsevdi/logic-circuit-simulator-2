#include "catch2/catch_test_macros.hpp"
#include "core.h"
#include "parse.h"
#include <catch2/catch_all.hpp>
#include <iostream>

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

    std::cout << v.toStyledString() << std::endl;
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
    std::cout << out.toStyledString() << std::endl;
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
    std::cout << scene_str.toStyledString() << std::endl
              << scene_loaded_str.toStyledString() << std::endl;
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

    std::cout << s_str.toStyledString() << std::endl;
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
