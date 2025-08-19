#include "common.h"
#include "core.h"
#include "test_util.h"
#include <doctest.h>
#include <json/json.h>

using namespace lcs;
TEST_CASE("parse Scene subnodes")
{
    Scene s { "parse-scene-subnodes" };

    auto g_or  = s.add_node<Gate>(Gate::Type::OR);
    auto g_and = s.add_node<Gate>(Gate::Type::AND);
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

    std::vector<uint8_t> v;
    REQUIRE_EQ(s.write_to(v), Error::OK);
    REQUIRE(!v.empty());
}

TEST_CASE("parse non-zero context")
{
    Scene s { "parse-non-zero-context" };

    auto g_and                   = s.add_node<Gate>(Gate::Type::AND);
    auto v1                      = s.add_node<Input>();
    auto v2                      = s.add_node<Input>();
    auto o                       = s.add_node<Output>();
    s.get_node<Input>(v1)->_freq = 3;

    s.connect(g_and, 0, v2);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_and);
    s.get_base(v1)->point = { 1, 3 };

    std::vector<uint8_t> v;
    REQUIRE_EQ(s.write_to(v), Error::OK);
    REQUIRE(!v.empty());
}

TEST_CASE("Save, load and compare")
{
    Scene s { "load-and-compare" };
    auto v = s.add_node<Input>();
    auto o = s.add_node<Output>();
    s.get_node<Input>(v)->set(true);
    s.connect(o, 0, v);

    std::vector<uint8_t> data { 0 };
    REQUIRE_EQ(s.write_to(data), Error::OK);
    Scene s_loaded;
    REQUIRE_EQ(s_loaded.read_from(data), Error::OK);
    std::vector<uint8_t> data_loaded { 0 };
    REQUIRE_EQ(s_loaded.write_to(data_loaded), Error::OK);

    REQUIRE_EQ(s_loaded.get_node<Output>(o)->get(), State::TRUE);

    REQUIRE_EQ(data.size(), data_loaded.size());
    REQUIRE(scene_cmp(s, s_loaded));
    // WARN Two bytes have invalid values: [00:00:00] INFO  |6_serialization.cpp
    // |lcs               |DOCTEST_ANON_FUNC_6      |0 == 7f WARN Two bytes have
    // invalid values: [00:00:00] INFO  |6_serialization.cpp  |lcs
    // |DOCTEST_ANON_FUNC_6      |0 == 4d
}

TEST_CASE("Save a complicated Scene, load and run the tests")
{
    // Taken from TEST_CASE("Full Adder")
    Scene s { "complicated-scene" };
    _create_full_adder_io(s);
    _create_full_adder(s);

    std::vector<uint8_t> data;
    REQUIRE_EQ(s.write_to(data), Error::OK);
    Scene s_loaded;
    REQUIRE_EQ(s_loaded.read_from(data), Error::OK);

    s_loaded.get_node<Input>(a)->set(true);
    s_loaded.get_node<Input>(b)->set(false);
    s_loaded.get_node<Input>(c_in)->set(false);
    REQUIRE_EQ(s_loaded.get_node<Output>(sum)->get(), State::TRUE);
    REQUIRE_EQ(s_loaded.get_node<Output>(c_out)->get(), State::FALSE);

    s_loaded.get_node<Input>(b)->set(true);
    REQUIRE_EQ(s_loaded.get_node<Output>(sum)->get(), State::FALSE);
    REQUIRE_EQ(s_loaded.get_node<Output>(c_out)->get(), State::TRUE);

    s_loaded.get_node<Input>(c_in)->set(true);
    REQUIRE_EQ(s_loaded.get_node<Output>(sum)->get(), State::TRUE);
    REQUIRE_EQ(s_loaded.get_node<Output>(c_out)->get(), State::TRUE);
}

TEST_CASE("Save a component, reload and run")
{
    // Taken from "Create a 2x1 MUX component"
    // IN1 = 1, IN2 = 2, IN3 = SEL
    Scene s { ComponentContext { &s, 3, 1 }, "2x1-mux" };
    auto g_and   = s.add_node<Gate>(Gate::Type::AND);
    auto g_and_2 = s.add_node<Gate>(Gate::Type::AND);
    auto g_not   = s.add_node<Gate>(Gate::Type::NOT);
    auto g_out   = s.add_node<Gate>(Gate::Type::OR);

    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and_2, 0, s.component_context->get_input(1));
    s.connect(g_and, 1, s.component_context->get_input(2));
    s.connect(g_not, 0, s.component_context->get_input(2));
    s.connect(g_and_2, 1, g_not);
    s.connect(g_out, 0, g_and);
    s.connect(g_out, 1, g_and_2);
    s.connect(s.component_context->get_output(0), 0, g_out);

    REQUIRE_EQ(s.component_context->run(0b111), 1);
    REQUIRE_EQ(s.component_context->run(0b110), 0);
    REQUIRE_EQ(s.component_context->run(0b101), 1);
    REQUIRE_EQ(s.component_context->run(0b100), 0);
    REQUIRE_EQ(s.component_context->run(0b011), 1);
    REQUIRE_EQ(s.component_context->run(0b010), 1);
    REQUIRE_EQ(s.component_context->run(0b001), 0);
    REQUIRE_EQ(s.component_context->run(0b000), 0);

    std::vector<uint8_t> data;
    REQUIRE_EQ(s.write_to(data), Error::OK);
    Scene s_loaded;
    REQUIRE_EQ(s_loaded.read_from(data), Error::OK);
    std::vector<uint8_t> data_loaded;
    REQUIRE_EQ(s_loaded.write_to(data_loaded), Error::OK);
    REQUIRE_EQ(s_loaded.component_context->run(0b111), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b110), 0);
    REQUIRE_EQ(s_loaded.component_context->run(0b101), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b100), 0);
    REQUIRE_EQ(s_loaded.component_context->run(0b011), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b010), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b001), 0);
    REQUIRE_EQ(s_loaded.component_context->run(0b000), 0);
}

TEST_CASE("Save a simple component, and use it in a scene")
{
    Scene s { ComponentContext { &s, 2, 1 }, "simple-component", "someone",
        "component description" };
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    s.connect(s.component_context->get_output(0), 0, g_and);
    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and, 1, s.component_context->get_input(1));

    REQUIRE_EQ(s.component_context->run(0b11), 1);
    REQUIRE_EQ(s.component_context->run(0b10), 0);
    REQUIRE_EQ(s.component_context->run(0b01), 0);
    REQUIRE_EQ(s.component_context->run(0b00), 0);
    Scene s2 {};
    s2.add_dependency(std::move(s));
    Node component = s2.add_node<Component>();
    REQUIRE_EQ(s2.get_node<Component>(component)->set_component(0), Error::OK);
    Node i1 = s2.add_node<Input>();
    Node i2 = s2.add_node<Input>();
    Node o  = s2.add_node<Output>();

    REQUIRE(s2.connect(component, 0, i1));
    REQUIRE(s2.connect(component, 1, i2));
    REQUIRE(s2.connect(o, 0, component, 0));
}

TEST_CASE("Save a component, load it to a scene")
{

    // Taken from TEST_CASE("Full Adder")
    Scene s { "component" };
    s.component_context = { &s, 3, 2 };
    Node a              = s.component_context->get_input(0);
    Node b              = s.component_context->get_input(1);
    Node c_in           = s.component_context->get_input(2);
    Node sum            = s.component_context->get_output(0);
    Node c_out          = s.component_context->get_output(1);
    _create_full_adder(s);

    Scene s2 {};
    s2.add_dependency(std::move(s));
    REQUIRE(s2.dependencies().size() > 0);

    Node cnode = s2.add_node<Component>();
    REQUIRE_EQ(s2.get_node<Component>(cnode)->set_component(0), Error::OK);

    Node i1 = s2.add_node<Input>();
    Node i2 = s2.add_node<Input>();
    Node i3 = s2.add_node<Input>();

    Node o1 = s2.add_node<Output>();
    Node o2 = s2.add_node<Output>();

    REQUIRE(s2.connect(cnode, 0, i1));
    REQUIRE(s2.connect(cnode, 1, i2));
    REQUIRE(s2.connect(cnode, 2, i3));

    REQUIRE(s2.connect(o1, 0, cnode, 0));
    REQUIRE(s2.connect(o2, 0, cnode, 1));

    {
        s2.get_node<Input>(i1)->set(true);
        s2.get_node<Input>(i2)->set(false);
        s2.get_node<Input>(i3)->set(false);
        REQUIRE_EQ(s2.get_node<Output>(o1)->get(), State::TRUE);
        REQUIRE_EQ(s2.get_node<Output>(o2)->get(), State::FALSE);
    }

    {
        s2.get_node<Input>(i1)->set(true);
        s2.get_node<Input>(i2)->set(true);
        s2.get_node<Input>(i3)->set(false);
        REQUIRE_EQ(s2.get_node<Output>(o1)->get(), State::FALSE);
        REQUIRE_EQ(s2.get_node<Output>(o2)->get(), State::TRUE);
    }

    {
        s2.get_node<Input>(i1)->set(true);
        s2.get_node<Input>(i2)->set(true);
        s2.get_node<Input>(i3)->set(true);
        REQUIRE_EQ(s2.get_node<Output>(o1)->get(), State::TRUE);
        REQUIRE_EQ(s2.get_node<Output>(o2)->get(), State::TRUE);
    }
}
