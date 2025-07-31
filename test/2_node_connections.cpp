#include "core.h"
#include <doctest.h>

using namespace lcs;
TEST_CASE("Basic Connect/Disconnect/Reconnect Test")
{
    Scene s;
    auto v = s.add_node<InputNode>();
    auto o = s.add_node<OutputNode>();
    auto r = s.connect(o, 0, v);
    REQUIRE(r);
    s.get_node<InputNode>(v)->set(true);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);
    s.disconnect(r);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::DISABLED);
    REQUIRE(s.connect(o, 0, v) != 0);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);
}

TEST_CASE("Early Disconnect Test")
{
    Scene s;
    auto v     = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();
    auto g_and = s.add_node<GateNode>(GateNode::Type::AND);

    auto r = s.connect(g_and, 0, v);
    REQUIRE(r);
    REQUIRE(s.connect(g_and, 1, v2));
    REQUIRE(s.connect(o, 0, g_and));
    s.get_node<InputNode>(v)->set(true);
    s.get_node<InputNode>(v2)->set(true);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);
    s.disconnect(r);

    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::DISABLED);
    REQUIRE_EQ(s.get_node<GateNode>(g_and)->get(), State::DISABLED);
}

TEST_CASE("Gate State Change After Disconnect")
{
    Scene s;
    auto g_or  = s.add_node<GateNode>(GateNode::Type::OR);
    auto g_and = s.add_node<GateNode>(GateNode::Type::AND);
    auto v1    = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto v3    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();

    REQUIRE(s.connect(g_or, 0, v1));
    REQUIRE(s.connect(g_or, 1, v2));
    REQUIRE(s.connect(g_and, 0, v3));
    REQUIRE(s.connect(g_and, 1, v1));
    REQUIRE(s.connect(o, 0, g_or));

    s.get_node<InputNode>(v1)->set(true);
    s.get_node<InputNode>(v2)->set(false);
    s.get_node<InputNode>(v3)->set(true);

    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);

    s.disconnect(s.connect(g_or, 0, v1));
    s.get_node<InputNode>(v1)->set(false);

    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::FALSE);
}

TEST_CASE("Invalid Connection Attempt")
{
    Scene s;
    auto o1 = s.add_node<OutputNode>();
    auto o2 = s.add_node<OutputNode>();
    auto v1 = s.add_node<InputNode>();

    REQUIRE_FALSE(s.connect(o1, 0, o2));
    REQUIRE_FALSE(s.connect(v1, 0, o1));
    REQUIRE(s.connect(o1, 0, v1));
    REQUIRE_FALSE(s.connect(o1, 0, v1));
}

TEST_CASE("Reconnect After Multiple Disconnections")
{
    Scene s;
    auto g_and = s.add_node<GateNode>(GateNode::Type::AND);
    auto v1    = s.add_node<InputNode>();
    auto v2    = s.add_node<InputNode>();
    auto o     = s.add_node<OutputNode>();

    auto r1 = s.connect(g_and, 0, v1);
    auto r2 = s.connect(g_and, 1, v2);
    REQUIRE(r1);
    REQUIRE(r2);
    auto r3 = s.connect(o, 0, g_and);
    REQUIRE(r3);

    s.get_node<InputNode>(v1)->set(true);
    s.get_node<InputNode>(v2)->set(true);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);

    s.disconnect(r1);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::DISABLED);
    s.disconnect(r2);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::DISABLED);

    REQUIRE(s.connect(g_and, 0, v1));
    REQUIRE(s.connect(g_and, 1, v2));
    REQUIRE_EQ(s.get_node<GateNode>(g_and)->get(), State::TRUE);

    s.disconnect(r3);
    REQUIRE_EQ(s.get_node<GateNode>(g_and)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::DISABLED);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE_EQ(s.get_node<OutputNode>(o)->get(), State::TRUE);
}
