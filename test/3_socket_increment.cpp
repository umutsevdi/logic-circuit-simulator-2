#include "core.h"
#include <doctest.h>

using namespace lcs;
TEST_CASE("Increment socket size, expect update")
{
    Scene s;

    auto v     = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto g_and = s.add_node<Gate>(Gate::Type::AND);
    auto o     = s.add_node<Output>();

    s.get_node<Input>(v)->set(true);
    s.get_node<Input>(v2)->set(true);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v2));

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.get_node<Gate>(g_and)->increment();
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    s.get_node<Gate>(g_and)->decrement();
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}
