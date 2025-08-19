#include "common.h"
#include "core.h"
#include <doctest.h>
#include <json/json.h>

using namespace lcs;

TEST_CASE("Save a scene and load")
{
    size_t s_handle = tabs::create("subnode", "author", "", 1);
    NRef<Scene> s   = tabs::active(s_handle);
    auto g_or       = s->add_node<Gate>(Gate::Type::OR);
    auto g_and      = s->add_node<Gate>(Gate::Type::AND);
    auto v1         = s->add_node<Input>();
    auto v2         = s->add_node<Input>();
    auto v3         = s->add_node<Input>();
    auto o          = s->add_node<Output>();

    s->connect(g_or, 0, v1);
    s->connect(g_or, 1, v2);
    s->connect(g_and, 0, v3);
    s->connect(g_and, 1, v1);
    s->connect(o, 0, g_or);
    tabs::notify(s_handle);
    REQUIRE_EQ(tabs::save_as(lcs::fs::TMP / "test.lcs", s_handle), Error::OK);
    REQUIRE_EQ(tabs::close(s_handle), Error::OK);
    size_t idx = -1;
    REQUIRE_EQ(tabs::open(lcs::fs::TMP / "test.lcs", idx), Error::OK);
    REQUIRE(idx != -1);
}

/*
TEST_CASE("Save a component and load")
{
    size_t s_handle = tabs::create("Save a component and load");
    NRef<Scene> s   = tabs::get(s_handle);
    s->component_context.emplace(&s, 2, 2);

    Node g_and = s->add_node<Gate>(Gate::Type::AND);
    Node g_or  = s->add_node<Gate>(Gate::Type::OR);
    s->connect(g_and, 0, s->component_context->get_input(0));
    s->connect(g_and, 1, s->component_context->get_input(1));
    s->connect(g_or, 0, s->component_context->get_input(0));
    s->connect(g_or, 1, s->component_context->get_input(1));
    s->connect(s->component_context->get_output(0), 0, g_and);
    s->connect(s->component_context->get_output(1), 0, g_or);

    std::string s_str  = s->to_json().toStyledString();
    std::string depstr = s->to_dependency();

    REQUIRE_EQ(tabs::save(s_handle), Error::OK);
    REQUIRE_EQ(tabs::close(s_handle), Error::OK);
    REQUIRE_EQ(io::component::fetch(depstr), Error::OK);
    auto compref = io::component::get(depstr);
    REQUIRE_NE(compref, nullptr);
    REQUIRE_EQ(s_str, compref->to_json().toStyledString());
}

TEST_CASE("Save a component, load it to a scene")
{
    // Taken from TEST_CASE("Full Adder")
    size_t s_handle = tabs::create("Save a component, load it to a scene");
    NRef<Scene> s   = tabs::get(s_handle);
    {
        s->component_context = { &s, 3, 2 };
        Node a               = s->component_context->get_input(0);
        Node b               = s->component_context->get_input(1);
        Node c_in            = s->component_context->get_input(2);
        Node sum             = s->component_context->get_output(0);
        Node c_out           = s->component_context->get_output(1);
        Node g_xor           = s->add_node<Gate>(Gate::Type ::XOR);
        Node g_xor_sum       = s->add_node<Gate>(Gate::Type ::XOR);
        Node g_and_carry     = s->add_node<Gate>(Gate::Type ::AND);
        Node g_and           = s->add_node<Gate>(Gate::Type ::AND);
        Node g_or            = s->add_node<Gate>(Gate::Type ::OR);
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
    tabs::save(s_handle);
    tabs::close(s_handle);

    Scene s2 {};
    s2.dependencies.push_back(dependency);
    REQUIRE(s2.load_dependencies() == Error::OK);
    REQUIRE(s2.dependencies.size() > 0);

    Node cnode = s2.add_node<Component>(dependency);
    Node i1    = s2.add_node<Input>();
    Node i2    = s2.add_node<Input>();
    Node i3    = s2.add_node<Input>();
    Node o1    = s2.add_node<Output>();
    Node o2    = s2.add_node<Output>();

    REQUIRE(s2.connect(cnode, 0, i1));
    REQUIRE(s2.connect(cnode, 1, i2));
    REQUIRE(s2.connect(cnode, 2, i3));
    REQUIRE(s2.connect(o1, 0, cnode, 0));
    REQUIRE(s2.connect(o2, 0, cnode, 1));

    {
        s2.get_node<Input>(i1)->set(true);
        s2.get_node<Input>(i2)->set(true);
        s2.get_node<Input>(i3)->set(true);
        REQUIRE_EQ(s2.get_node<Output>(o1)->get(), State::TRUE);
        REQUIRE_EQ(s2.get_node<Output>(o2)->get(), State::TRUE);
    }
}

TEST_CASE("Fetch a non-local component")
{
    size_t s_handle = tabs::create("non-local-component", "TestUser");
    NRef<Scene> s   = tabs::active_scene(s_handle);
    {
        s->version           = 2;
        s->component_context = { &s, 2, 1 };

        Node n_gate = s->add_node<Gate>(Gate::Type::NOT);
        Node a_gate = s->add_node<Gate>(Gate::Type::AND);
        s->connect(n_gate, 0, s->component_context->get_input(0));
        s->connect(a_gate, 0, n_gate);
        s->connect(a_gate, 1, s->component_context->get_input(1));
        s->connect(s->component_context->get_output(0), 0, a_gate);
    }
    std::string dependency = s->to_dependency();
    tabs::save(s_handle);
    tabs::close(s_handle);

    Scene s2 {};
    s2.dependencies.push_back(dependency);
    REQUIRE(s2.load_dependencies() == Error::OK);

    Node cnode = s2.add_node<Component>(dependency);
    Node i1    = s2.add_node<Input>();
    Node i2    = s2.add_node<Input>();
    Node o     = s2.add_node<Output>();

    REQUIRE(s2.connect(cnode, 0, i1));
    REQUIRE(s2.connect(cnode, 1, i2));
    REQUIRE(s2.connect(o, 0, cnode));

    {
        s2.get_node<Input>(i1)->set(false);
        s2.get_node<Input>(i2)->set(true);
        REQUIRE_EQ(s2.get_node<Output>(o)->get(), State::FALSE);
    }
    {
        s2.get_node<Input>(i1)->set(true);
        s2.get_node<Input>(i2)->set(false);
        REQUIRE_EQ(s2.get_node<Output>(o)->get(), State::TRUE);
    }
}
*/
