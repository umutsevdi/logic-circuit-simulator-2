#include "catch2/catch_test_macros.hpp"
#include "core/engine.h"
#include "yaml-cpp/exceptions.h"
#include <catch2/catch_all.hpp>
#include <iostream>
#include <memory>

TEST_CASE("Parse YAML subnodes")
{
    lcs::parser::LcsEmitter out {};
    lcs::Scene s;

    auto g_or  = s.add_node<lcs::Gate>(lcs::gate_t::OR);
    auto g_and = s.add_node<lcs::Gate>(lcs::gate_t::AND);
    auto v1    = s.add_node<lcs::Input>();
    auto v2    = s.add_node<lcs::Input>();
    auto v3    = s.add_node<lcs::Input>();
    auto o     = s.add_node<lcs::Output>();

    s.connect(g_or, 0, v1);
    s.connect(g_or, 1, v2);
    s.connect(g_and, 0, v3);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_or);
    s.get_node<lcs::Input>(v1)->set(true);
    s.get_node<lcs::Input>(v2)->set(false);
    s.get_node<lcs::Input>(v3)->set(true);

    REQUIRE(lcs::parser::write_scene(s, out) == lcs::parser::error_t::OK);
}

TEST_CASE("Parse non-zero context")
{
    lcs::parser::LcsEmitter out {};
    lcs::Scene s;

    auto g_and = s.add_node<lcs::Gate>(lcs::gate_t::AND);
    auto v1    = s.add_node<lcs::Input>();
    auto v2    = s.add_node<lcs::Input>();
    auto o     = s.add_node<lcs::Output>();

    s.connect(g_and, 0, v2);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_and);
    s.get_base(o)->set_rotation(lcs::direction_t::DOWN);
    s.get_base(v1)->set_position({ 1, 3 });

    REQUIRE(lcs::parser::write_scene(s, out) == lcs::parser::error_t::OK);
    std::cout << (out.c_str()) << std::endl;
    REQUIRE_NOTHROW(YAML::Load(std::string { out.c_str() }));
}

TEST_CASE("Save, load and compare") // <-- runtime assert error
{
    lcs::parser::LcsEmitter emitter {};
    lcs::Scene s;
    auto v = s.add_node<lcs::Input>();
    auto o = s.add_node<lcs::Output>();
    s.get_node<lcs::Input>(v)->set(true);
    s.connect(o, 0, v);

    REQUIRE(lcs::parser::write_scene(s, emitter) == lcs::parser::error_t::OK);
    std::string str { emitter.c_str() };
    std::cout << str << std::endl;

    YAML::Node n;
    REQUIRE_NOTHROW((n = YAML::Load(str)));

    lcs::parser::error_t err = lcs::parser::error_t::OK;

    std::cout << "Bussy\t" << n["scene"] << std::endl;
    // FAILURE HAPPENS HERE
    auto newscene = lcs::parser::read_scene(n["scene"], err);

    REQUIRE((err == lcs::parser::error_t::OK && newscene != nullptr));

    lcs::parser::LcsEmitter emitter2 {};
    REQUIRE(lcs::parser::write_scene(*newscene, emitter2)
        == lcs::parser::error_t::OK);
    REQUIRE(s.get_node<lcs::Output>(o)->get()
        == newscene->get_node<lcs::Output>(o)->get());
}

TEST_CASE("Save a complicated yaml, load and run the tests")
{
    lcs::parser::LcsEmitter emitter {};

    // Taken from TEST_CASE("Full Adder")
    lcs::Scene s;
    auto a     = s.add_node<lcs::Input>();
    auto b     = s.add_node<lcs::Input>();
    auto c_in  = s.add_node<lcs::Input>();
    auto c_out = s.add_node<lcs::Output>();
    auto sum   = s.add_node<lcs::Output>();

    auto g_xor       = s.add_node<lcs::Gate>(lcs::gate_t::XOR);
    auto g_xor_sum   = s.add_node<lcs::Gate>(lcs::gate_t::XOR);
    auto g_and_carry = s.add_node<lcs::Gate>(lcs::gate_t::AND);
    auto g_and       = s.add_node<lcs::Gate>(lcs::gate_t::AND);
    auto g_or        = s.add_node<lcs::Gate>(lcs::gate_t::OR);

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
    REQUIRE(lcs::parser::write_scene(s, emitter) == lcs::parser::error_t::OK);

    YAML::Node n             = YAML::Load(emitter.c_str());
    lcs::parser::error_t err = lcs::parser::error_t::OK;
    std::cout << (emitter.c_str()) << std::endl;
    auto newscene = lcs::parser::read_scene(n["scene"], err);
    REQUIRE((err == lcs::parser::error_t::OK && newscene != nullptr));

    // Test cases from TEST_CASE("Full Adder")
    newscene->get_node<lcs::Input>(a)->set(true);
    newscene->get_node<lcs::Input>(b)->set(false);
    newscene->get_node<lcs::Input>(c_in)->set(false);
    REQUIRE(newscene->get_node<lcs::Output>(sum)->get() == lcs::state_t::TRUE);
    REQUIRE(
        newscene->get_node<lcs::Output>(c_out)->get() == lcs::state_t::FALSE);

    newscene->get_node<lcs::Input>(b)->set(true);
    REQUIRE(newscene->get_node<lcs::Output>(sum)->get() == lcs::state_t::FALSE);
    REQUIRE(
        newscene->get_node<lcs::Output>(c_out)->get() == lcs::state_t::TRUE);

    newscene->get_node<lcs::Input>(c_in)->set(true);
    REQUIRE(newscene->get_node<lcs::Output>(sum)->get() == lcs::state_t::TRUE);
    REQUIRE(
        newscene->get_node<lcs::Output>(c_out)->get() == lcs::state_t::TRUE);
    std::cout << emitter.c_str() << std::endl;
}
