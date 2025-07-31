#include "core.h"
#include <doctest.h>

using namespace lcs;
TEST_CASE("Increment socket size, expect update")
{
    Scene s;

    auto v     = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto g_and = s.add_node<GateNode>(GateNode::Type::AND);
    auto o     = s.add_node<OutputNode>();

    s.get_node<InputNode>(v)->set(true);
    s.get_node<InputNode>(v2)->set(true);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v2));

    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);
    s.get_node<GateNode>(g_and)->increment();
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::DISABLED);
    s.get_node<GateNode>(g_and)->decrement();
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);
}
