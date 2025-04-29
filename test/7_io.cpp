#include "common.h"
#include "core.h"
#include "io.h"
#include <doctest.h>
#include <json/json.h>

using namespace lcs;

TEST_CASE("Save a scene and load")
{
    size_t s_handle = io::scene::create("io JSON subnodes");
    NRef<Scene> s   = io::scene::get(s_handle);
    auto g_or       = s->add_node<GateNode>(gate_t::OR);
    auto g_and      = s->add_node<GateNode>(gate_t::AND);
    auto v1         = s->add_node<InputNode>();
    auto v2         = s->add_node<InputNode>();
    auto v3         = s->add_node<InputNode>();
    auto o          = s->add_node<OutputNode>();

    s->connect(g_or, 0, v1);
    s->connect(g_or, 1, v2);
    s->connect(g_and, 0, v3);
    s->connect(g_and, 1, v1);
    s->connect(o, 0, g_or);

    std::string s_str = s->to_json().toStyledString();

    REQUIRE_EQ(io::scene::save_as(io::TMP / "test.json"), lcs::error_t::OK);
    REQUIRE_EQ(io::scene::close(), lcs::error_t::OK);
    size_t idx = -1;
    REQUIRE_EQ(io::scene::open(io::TMP / "test.json", idx), lcs::error_t::OK);
    REQUIRE(idx != -1);
    REQUIRE_EQ(s_str, io::scene::get()->to_json().toStyledString());
}

TEST_CASE("Save a component and load")
{
    size_t s_handle = io::scene::create("Save a component and load");
    NRef<Scene> s   = io::scene::get(s_handle);
    s->component_context.emplace(&s, 2, 2);

    node g_and = s->add_node<GateNode>(gate_t::AND);
    node g_or  = s->add_node<GateNode>(gate_t::OR);
    s->connect(g_and, 0, s->component_context->get_input(0));
    s->connect(g_and, 1, s->component_context->get_input(1));
    s->connect(g_or, 0, s->component_context->get_input(0));
    s->connect(g_or, 1, s->component_context->get_input(1));
    s->connect(s->component_context->get_output(0), 0, g_and);
    s->connect(s->component_context->get_output(1), 0, g_or);

    std::string s_str  = s->to_json().toStyledString();
    std::string depstr = s->to_dependency();

    REQUIRE_EQ(io::scene::save(s_handle), lcs::error_t::OK);
    REQUIRE_EQ(io::scene::close(s_handle), lcs::error_t::OK);
    REQUIRE_EQ(io::component::fetch(depstr), lcs::error_t::OK);
    auto compref = io::component::get(depstr);
    REQUIRE_NE(compref, nullptr);
    REQUIRE_EQ(s_str, compref->to_json().toStyledString());
}

TEST_CASE("Save a component, load it to a scene")
{
    // Taken from TEST_CASE("Full Adder")
    size_t s_handle = io::scene::create("Save a component, load it to a scene");
    NRef<Scene> s   = io::scene::get(s_handle);
    {
        s->component_context = { &s, 3, 2 };
        node a               = s->component_context->get_input(0);
        node b               = s->component_context->get_input(1);
        node c_in            = s->component_context->get_input(2);
        node sum             = s->component_context->get_output(0);
        node c_out           = s->component_context->get_output(1);
        node g_xor           = s->add_node<GateNode>(gate_t ::XOR);
        node g_xor_sum       = s->add_node<GateNode>(gate_t ::XOR);
        node g_and_carry     = s->add_node<GateNode>(gate_t ::AND);
        node g_and           = s->add_node<GateNode>(gate_t ::AND);
        node g_or            = s->add_node<GateNode>(gate_t ::OR);
        s->connect(g_xor, 0, a);
        s->connect(g_xor, 1, b);
        s->connect(g_xor_sum, 0, g_xor);
        s->connect(g_xor_sum, 1, c_in);
        s->connect(sum, 0, g_xor_sum);
        s->connect(g_and, 0, c_in);
        s->connect(g_and, 1, g_xor);
        s->connect(g_and_carry, 0, a);
        s->connect(g_and_carry, 1, b);
        s->connect(g_or, 0, g_and);
        s->connect(g_or, 1, g_and_carry);
        s->connect(c_out, 0, g_or);
    }
    std::string dependency = s->to_dependency();
    io::scene::save(s_handle);
    io::scene::close(s_handle);

    Scene s2 {};
    s2.dependencies.push_back(dependency);
    REQUIRE(s2.load_dependencies() == lcs::error_t::OK);
    REQUIRE(s2.dependencies.size() > 0);

    node cnode = s2.add_node<ComponentNode>(dependency);
    node i1    = s2.add_node<InputNode>();
    node i2    = s2.add_node<InputNode>();
    node i3    = s2.add_node<InputNode>();
    node o1    = s2.add_node<OutputNode>();
    node o2    = s2.add_node<OutputNode>();

    REQUIRE(s2.connect(cnode, 0, i1));
    REQUIRE(s2.connect(cnode, 1, i2));
    REQUIRE(s2.connect(cnode, 2, i3));
    REQUIRE(s2.connect(o1, 0, cnode, 0));
    REQUIRE(s2.connect(o2, 0, cnode, 1));

    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(true);
        s2.get_node<InputNode>(i3)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o1)->get(), state_t::TRUE);
        REQUIRE_EQ(s2.get_node<OutputNode>(o2)->get(), state_t::TRUE);
    }
}

TEST_CASE("Fetch a non-local component")
{
    size_t s_handle = io::scene::create("non-local-component");
    NRef<Scene> s   = io::scene::get(s_handle);
    {
        s->author            = "TestUser";
        s->version           = 2;
        s->component_context = { &s, 2, 1 };

        node n_gate = s->add_node<GateNode>(gate_t::NOT);
        node a_gate = s->add_node<GateNode>(gate_t::AND);
        s->connect(n_gate, 0, s->component_context->get_input(0));
        s->connect(a_gate, 0, n_gate);
        s->connect(a_gate, 1, s->component_context->get_input(1));
        s->connect(s->component_context->get_output(0), 0, a_gate);
    }
    std::string dependency = s->to_dependency();
    io::scene::save(s_handle);
    io::scene::close(s_handle);

    Scene s2 {};
    s2.dependencies.push_back(dependency);
    REQUIRE(s2.load_dependencies() == lcs::error_t::OK);

    node cnode = s2.add_node<ComponentNode>(dependency);
    node i1    = s2.add_node<InputNode>();
    node i2    = s2.add_node<InputNode>();
    node o     = s2.add_node<OutputNode>();

    REQUIRE(s2.connect(cnode, 0, i1));
    REQUIRE(s2.connect(cnode, 1, i2));
    REQUIRE(s2.connect(o, 0, cnode));

    {
        s2.get_node<InputNode>(i1)->set(false);
        s2.get_node<InputNode>(i2)->set(true);
        REQUIRE_EQ(s2.get_node<OutputNode>(o)->get(), state_t::FALSE);
    }
    {
        s2.get_node<InputNode>(i1)->set(true);
        s2.get_node<InputNode>(i2)->set(false);
        REQUIRE_EQ(s2.get_node<OutputNode>(o)->get(), state_t::TRUE);
    }
}
