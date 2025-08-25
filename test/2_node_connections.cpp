#include "core.h"
#include <doctest.h>

using namespace lcs;
TEST_CASE("Basic Connect/Disconnect/Reconnect Test")
{
    Scene s;
    auto v = s.add_node<Input>();
    auto o = s.add_node<Output>();
    auto r = s.connect(o, 0, v);
    REQUIRE(r);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.disconnect(r);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    REQUIRE(s.connect(o, 0, v) != 0);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("Early Disconnect Test")
{
    Scene s;
    auto v     = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto o     = s.add_node<Output>();
    auto g_and = s.add_node<Gate>(Gate::Type::AND);

    auto r = s.connect(g_and, 0, v);
    REQUIRE(r);
    REQUIRE(s.connect(g_and, 1, v2));
    REQUIRE(s.connect(o, 0, g_and));
    s.get_node<Input>(v)->set(true);
    s.get_node<Input>(v2)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.disconnect(r);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::DISABLED);
}

TEST_CASE("Gate State Change After Disconnect")
{
    Scene s;
    auto g_or  = s.add_node<Gate>(Gate::Type::OR);
    auto g_and = s.add_node<Gate>(Gate::Type::AND);
    auto v1    = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto v3    = s.add_node<Input>();
    auto o     = s.add_node<Output>();

    REQUIRE(s.connect(g_or, 0, v1));
    REQUIRE(s.connect(g_or, 1, v2));
    REQUIRE(s.connect(g_and, 0, v3));
    REQUIRE(s.connect(g_and, 1, v1));
    REQUIRE(s.connect(o, 0, g_or));

    s.get_node<Input>(v1)->set(true);
    s.get_node<Input>(v2)->set(false);
    s.get_node<Input>(v3)->set(true);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);

    s.disconnect(s.connect(g_or, 0, v1));
    s.get_node<Input>(v1)->set(false);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
}

TEST_CASE("Invalid Connection Attempt")
{
    Scene s;
    auto o1 = s.add_node<Output>();
    auto o2 = s.add_node<Output>();
    auto v1 = s.add_node<Input>();

    REQUIRE_FALSE(s.connect(o1, 0, o2));
    REQUIRE_FALSE(s.connect(v1, 0, o1));
    REQUIRE(s.connect(o1, 0, v1));
    REQUIRE_FALSE(s.connect(o1, 0, v1));
}

TEST_CASE("Reconnect After Multiple Disconnections")
{
    Scene s;
    auto g_and = s.add_node<Gate>(Gate::Type::AND);
    auto v1    = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto o     = s.add_node<Output>();

    auto r1 = s.connect(g_and, 0, v1);
    auto r2 = s.connect(g_and, 1, v2);
    REQUIRE(r1);
    REQUIRE(r2);
    auto r3 = s.connect(o, 0, g_and);
    REQUIRE(r3);

    s.get_node<Input>(v1)->set(true);
    s.get_node<Input>(v2)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);

    s.disconnect(r1);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    s.disconnect(r2);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    REQUIRE(s.connect(g_and, 0, v1));
    REQUIRE(s.connect(g_and, 1, v2));
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::TRUE);

    s.disconnect(r3);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}
