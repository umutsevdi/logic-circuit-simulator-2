#include "core.h"
#include "io.h"
#include "test_util.h"
#include <doctest.h>
#include <json/json.h>

using namespace lcs;

TEST_CASE("parse JSON subnodes")
{
    Scene s { "parse JSON subnodes" };

    auto g_or  = s.add_node<GateNode>(gate_t::OR);
    auto g_and = s.add_node<GateNode>(gate_t::AND);
    auto v1    = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto v3    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();

    s.connect(g_or, 0, v1);
    s.connect(g_or, 1, v2);
    s.connect(g_and, 0, v3);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_or);
    s.get_node<InputNode>(v1)->set(true);
    s.get_node<InputNode>(v2)->set(false);
    s.get_node<InputNode>(v3)->set(true);
    std::string v = s.to_json().toStyledString();
    REQUIRE(!v.empty());
}

TEST_CASE("parse non-zero context")
{
    Scene s { "parse non-zero context" };

    auto g_and = s.add_node<GateNode>(gate_t::AND);
    auto v1    = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();
    s.get_node<InputNode>(v1)->set_freq(3);

    s.connect(g_and, 0, v2);
    s.connect(g_and, 1, v1);
    s.connect(o, 0, g_and);
    s.get_base(o)->dir    = direction_t::DOWN;
    s.get_base(v1)->point = { 1, 3 };

    std::string out = s.to_json().toStyledString();
    REQUIRE(!out.empty());
}

TEST_CASE("Save, load and compare")
{
    Scene s { "Save, load and compare" };
    auto v = s.add_node<InputNode>();
    auto o = s.add_node<OutputNode>();
    s.get_node<InputNode>(v)->set(true);
    s.connect(o, 0, v);

    std::string scene_str = s.to_json().toStyledString();
    Scene s_loaded;
    REQUIRE_EQ(io::load(scene_str, s_loaded), lcs::error_t::OK);

    std::string scene_loaded_str = s_loaded.to_json().toStyledString();

    REQUIRE_EQ(scene_str, scene_loaded_str);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(o)->get(), state_t::TRUE);
}

TEST_CASE("Save a complicated JSON, load and run the tests")
{
    // Taken from TEST_CASE("Full Adder")
    Scene s { "Save a complicated JSON, load and run the tests" };
    _create_full_adder_io(s);
    _create_full_adder(s);

    std::string s_str = s.to_json().toStyledString();
    Scene s_loaded {};

    Json::Value doc;
    REQUIRE_EQ(io::load(s_str, s_loaded), lcs::error_t::OK);
    s_loaded.get_node<InputNode>(a)->set(true);
    s_loaded.get_node<InputNode>(b)->set(false);
    s_loaded.get_node<InputNode>(c_in)->set(false);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(sum)->get(), state_t::TRUE);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(c_out)->get(), state_t::FALSE);

    s_loaded.get_node<InputNode>(b)->set(true);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(sum)->get(), state_t::FALSE);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(c_out)->get(), state_t::TRUE);

    s_loaded.get_node<InputNode>(c_in)->set(true);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(sum)->get(), state_t::TRUE);
    REQUIRE_EQ(s_loaded.get_node<OutputNode>(c_out)->get(), state_t::TRUE);
}

TEST_CASE("Save a component, reload and run")
{
    // Taken from "Create a 2x1 MUX component"
    // IN1 = 1, IN2 = 2, IN3 = SEL
    Scene s { ComponentContext { &s, 3, 1 }, "Create a 2x1 MUX component" };
    auto g_and   = s.add_node<GateNode>(gate_t::AND);
    auto g_and_2 = s.add_node<GateNode>(gate_t::AND);
    auto g_not   = s.add_node<GateNode>(gate_t::NOT);
    auto g_out   = s.add_node<GateNode>(gate_t::OR);

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

    std::string s_str = s.to_json().toStyledString();
    Scene s_loaded {};
    REQUIRE_EQ(io::load(s_str, s_loaded), lcs::error_t::OK);
    REQUIRE_EQ(s_str, s_loaded.to_json().toStyledString());
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
    Scene s { ComponentContext { &s, 2, 1 },
        "Save a simple component, and use it in a scene" };
    node g_and = s.add_node<GateNode>(gate_t::AND);
    s.connect(s.component_context->get_output(0), 0, g_and);
    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and, 1, s.component_context->get_input(1));

    REQUIRE_EQ(s.component_context->run(0b11), 1);
    REQUIRE_EQ(s.component_context->run(0b10), 0);
    REQUIRE_EQ(s.component_context->run(0b01), 0);
    REQUIRE_EQ(s.component_context->run(0b00), 0);
    std::string dependency = s.to_dependency();
    REQUIRE_EQ(io::component::fetch(dependency, s.to_json().toStyledString()),
        lcs::error_t::OK);
    REQUIRE(io::component::get(dependency) != nullptr);
    REQUIRE_EQ(io::component::run(dependency, 0b11), 1);
    REQUIRE_EQ(io::component::run(dependency, 0b10), 0);
    REQUIRE_EQ(io::component::run(dependency, 0b01), 0);
    REQUIRE_EQ(io::component::run(dependency, 0b00), 0);

    Scene s2 {};
    s2.dependencies.push_back(dependency);
    s2.load_dependencies();

    node component = s2.add_node<ComponentNode>(dependency);
    node i1        = s2.add_node<InputNode>();
    node i2        = s2.add_node<InputNode>();
    node o         = s2.add_node<OutputNode>();

    REQUIRE(s2.connect(component, 0, i1));
    REQUIRE(s2.connect(component, 1, i2));
    REQUIRE(s2.connect(o, 0, component, 0));
}

TEST_CASE("Save a component, load it to a scene")
{

    // Taken from TEST_CASE("Full Adder")
    Scene s { "Save a component, load it to a scene" };
    s.component_context = { &s, 3, 2 };
    node a              = s.component_context->get_input(0);
    node b              = s.component_context->get_input(1);
    node c_in           = s.component_context->get_input(2);
    node sum            = s.component_context->get_output(0);
    node c_out          = s.component_context->get_output(1);
    _create_full_adder(s);

    std::string dependency = s.to_dependency();
    REQUIRE_EQ(io::component::fetch(dependency, s.to_json().toStyledString()),
        lcs::error_t::OK);

    Scene s2 {};
    s2.dependencies.push_back(s.to_dependency());
    REQUIRE(s2.load_dependencies() == lcs::error_t::OK);
    REQUIRE(s2.dependencies.size() > 0);

    node cnode = s2.add_node<ComponentNode>(s.to_dependency());
    REQUIRE_GT(cnode.id, 0);
    REQUIRE_EQ(io::component::run(dependency, 0b011), 0b10);

    node i1 = s2.add_node<InputNode>();
    node i2 = s2.add_node<InputNode>();
    node i3 = s2.add_node<InputNode>();

    node o1 = s2.add_node<OutputNode>();
    node o2 = s2.add_node<OutputNode>();

    REQUIRE(s2.connect(cnode, 0, i1));
    REQUIRE(s2.connect(cnode, 1, i2));
    REQUIRE(s2.connect(cnode, 2, i3));

    REQUIRE(s2.connect(o1, 0, cnode, 0));
    REQUIRE(s2.connect(o2, 0, cnode, 1));

    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(false);
        s2.get_node<InputNode>(i3)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), state_t::TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), state_t::FALSE);
    }

    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(true);
        s2.get_node<InputNode>(i3)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), state_t::FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), state_t::TRUE);
    }

    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(true);
        s2.get_node<InputNode>(i3)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), state_t::TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), state_t::TRUE);
    }
}

TEST_CASE("Load a component to a scene, then update component(out)")
{
    Scene s { ComponentContext { &s, 2, 2 },
        "Load a component to a scene, then update component(out)" };

    node g_and = s.add_node<GateNode>(gate_t::AND);
    node g_or  = s.add_node<GateNode>(gate_t::OR);
    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and, 1, s.component_context->get_input(1));
    s.connect(g_or, 0, s.component_context->get_input(0));
    s.connect(g_or, 1, s.component_context->get_input(1));
    s.connect(s.component_context->get_output(0), 0, g_and);
    s.connect(s.component_context->get_output(1), 0, g_or);
    std::string component_name = s.to_dependency();
    io::component::fetch(component_name, s.to_json().toStyledString());

    Scene s2 {};
    s2.dependencies.push_back(component_name);
    s2.load_dependencies();

    node c  = s2.add_node<ComponentNode>(component_name);
    node i1 = s2.add_node<InputNode>();
    node i2 = s2.add_node<InputNode>();
    node o1 = s2.add_node<OutputNode>();
    node o2 = s2.add_node<OutputNode>();

    s2.connect(o1, 0, c, 0);
    s2.connect(o2, 0, c, 1);
    s2.connect(c, 0, i1);
    s2.connect(c, 1, i2);

    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), TRUE);

        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), TRUE);

        s2.get_node<InputNode>(i1)->set(false);
        s2.get_node<InputNode>(i2)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), TRUE);

        s2.get_node<InputNode>(i1)->set(false);
        s2.get_node<InputNode>(i2)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), FALSE);
    }

    node g_not = s.add_node<GateNode>(gate_t::NOT);
    s.component_context->setup(2, 3);
    REQUIRE(s.connect(g_not, 0, s.component_context->get_input(0)));
    REQUIRE(s.connect(s.component_context->get_output(2), 0, g_not));
    REQUIRE_EQ(io::component::fetch(
                   component_name, s.to_json().toStyledString(), true),
        0);
    node o3 = s2.add_node<OutputNode>();
    REQUIRE(s2.connect(o3, 0, c, 2));

    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o3)->get(), FALSE);

        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o3)->get(), TRUE);

        s2.get_node<InputNode>(i1)->set(false);
        s2.get_node<InputNode>(i2)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o3)->get(), FALSE);

        s2.get_node<InputNode>(i1)->set(false);
        s2.get_node<InputNode>(i2)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), FALSE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o3)->get(), TRUE);
    }
}
