#include "core.h"
#include <doctest.h>

using namespace lcs;
TEST_CASE("Basic Add Remove Re-add Test")
{
    Scene s;
    auto v = s.add_node<Input>();
    auto o = s.add_node<Output>();
    auto r = s.connect(o, 0, v);
    REQUIRE(r);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.remove_node(v);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    v = s.add_node<Input>();
    REQUIRE(s.connect(o, 0, v));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("Readd Gate Test")
{
    Scene s;
    auto v     = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto o     = s.add_node<Output>();
    auto g_and = s.add_node<Gate>(Gate::Type::AND);
    REQUIRE(s.connect(o, 0, g_and));

    auto r = s.connect(g_and, 0, v);
    REQUIRE(r);
    REQUIRE(s.connect(g_and, 1, v2));
    s.get_node<Input>(v)->set(true);
    s.get_node<Input>(v2)->set(false);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);

    s.remove_node(g_and);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    auto g_or = s.add_node<Gate>(Gate::Type::OR);
    REQUIRE(s.connect(o, 0, g_or));
    REQUIRE(s.connect(g_or, 0, v));
    REQUIRE(s.connect(g_or, 1, v2));

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("Gate State Change After Removal")
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
    s.remove_node(g_or);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    g_or = s.add_node<Gate>(Gate::Type::OR);
    REQUIRE(s.connect(g_or, 0, v1));
    REQUIRE(s.connect(g_or, 1, v2));
    REQUIRE(s.connect(o, 0, g_or));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);

    s.get_node<Input>(v1)->set(false);
    s.get_node<Input>(v1)->set(false);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
}
