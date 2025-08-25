#include "common.h"
#include "core.h"
#include "test_util.h"
#include <doctest.h>
using namespace lcs;

TEST_CASE("Connect IN to OUT, Update")
{
    Scene s;
    auto v = s.add_node<Input>();
    auto o = s.add_node<Output>();
    REQUIRE(s.connect(o, 0, v));
    lcs_assert(s.get_node<Output>(o) != nullptr);
    lcs_assert(s.get_node<Input>(v) != nullptr);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("Connect IN to OUT, update and disconnect")
{
    Scene s;
    auto v = s.add_node<Input>();
    auto o = s.add_node<Output>();
    auto r = s.connect(o, 0, v);
    REQUIRE(r != 0);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.disconnect(r);
    L_INFO("%s", to_str<State>(s.get_node<Output>(o)->get()));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
}

TEST_CASE("Connect same input to Gate")
{
    Scene s;
    auto v     = s.add_node<Input>();
    auto o     = s.add_node<Output>();
    auto g_and = s.add_node<Gate>(Gate::Type::AND);

    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v));
    REQUIRE(s.connect(o, 0, g_and));

    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.get_node<Input>(v)->set(false);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
}

TEST_CASE("[[v1,v2]->and, v2]->or->o")
{
    Scene s;
    auto g_and = s.add_node<Gate>(Gate::Type::AND);
    auto g_or  = s.add_node<Gate>(Gate::Type::OR);
    auto v     = s.add_node<Input>();
    auto v2    = s.add_node<Input>();
    auto o     = s.add_node<Output>();

    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v2));
    REQUIRE(s.connect(g_or, 0, g_and));
    REQUIRE(s.connect(g_or, 1, v));
    REQUIRE(s.connect(o, 0, g_or));

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);

    s.get_node<Input>(v)->set(true);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::FALSE);
    REQUIRE_EQ(s.get_node<Gate>(g_or)->get(), State::TRUE);
}

TEST_CASE("Multiple Inputs with AND/OR Gates")
{
    Scene s;
    auto g_and  = s.add_node<Gate>(Gate::Type::AND);
    auto g_or   = s.add_node<Gate>(Gate::Type::OR);
    auto g_nand = s.add_node<Gate>(Gate::Type::NAND);
    auto g_nor  = s.add_node<Gate>(Gate::Type::NOR);
    auto v1     = s.add_node<Input>();
    auto v2     = s.add_node<Input>();
    auto v3     = s.add_node<Input>();
    auto o      = s.add_node<Output>();

    REQUIRE(s.connect(g_and, 0, v1));
    REQUIRE(s.connect(g_and, 1, v2));
    REQUIRE(s.connect(g_or, 0, v1));
    REQUIRE(s.connect(g_or, 1, v2));
    REQUIRE(s.connect(g_nand, 0, v2));
    REQUIRE(s.connect(g_nand, 1, v3));
    REQUIRE(s.connect(g_nor, 0, v3));
    REQUIRE(s.connect(g_nor, 1, v1));
    REQUIRE(s.connect(o, 0, g_or));

    s.get_node<Input>(v1)->set(true);
    s.get_node<Input>(v2)->set(true);
    s.get_node<Input>(v3)->set(false);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_or)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_nand)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_nor)->get(), State::FALSE);
}

TEST_CASE("1-Bit Adder Circuit")
{
    Scene s;
    auto v1   = s.add_node<Input>();
    auto v2   = s.add_node<Input>();
    auto gate = s.add_node<Gate>(Gate::Type::OR);
    auto sum  = s.add_node<Output>();

    REQUIRE(s.connect(gate, 0, v1));
    REQUIRE(s.connect(gate, 1, v2));
    REQUIRE(s.connect(sum, 0, gate));

    SUBCASE("1 + 0 = 1")
    {
        s.get_node<Input>(v1)->set(true);
        s.get_node<Input>(v2)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
    }
    SUBCASE("0 + 1 = 1")
    {
        s.get_node<Input>(v1)->set(false);
        s.get_node<Input>(v2)->set(true);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
    }
    SUBCASE("0 + 0 = 0")
    {
        s.get_node<Input>(v1)->set(false);
        s.get_node<Input>(v2)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::FALSE);
    }
}

TEST_CASE("Full Adder")
{
    Scene s;
    _create_full_adder_io(s);

    auto g_xor       = s.add_node<Gate>(Gate::Type::XOR);
    auto g_xor_sum   = s.add_node<Gate>(Gate::Type::XOR);
    auto g_and_carry = s.add_node<Gate>(Gate::Type::AND);
    auto g_and       = s.add_node<Gate>(Gate::Type::AND);
    auto g_or        = s.add_node<Gate>(Gate::Type::OR);

    REQUIRE(s.connect(g_xor, 0, a));
    REQUIRE(s.connect(g_xor, 1, b));

    REQUIRE(s.connect(g_xor_sum, 0, g_xor));
    REQUIRE(s.connect(g_xor_sum, 1, c_in));
    REQUIRE(s.connect(sum, 0, g_xor_sum));

    REQUIRE(s.connect(g_and, 0, c_in));
    REQUIRE(s.connect(g_and, 1, g_xor));

    REQUIRE(s.connect(g_and_carry, 0, a));
    REQUIRE(s.connect(g_and_carry, 1, b));

    REQUIRE(s.connect(g_or, 0, g_and));
    REQUIRE(s.connect(g_or, 1, g_and_carry));
    REQUIRE(s.connect(c_out, 0, g_or));

    SUBCASE("01 + 00 = 01")
    {
        s.get_node<Input>(a)->set(true);
        s.get_node<Input>(b)->set(false);
        s.get_node<Input>(c_in)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
        REQUIRE_EQ(s.get_node<Output>(c_out)->get(), State::FALSE);
    }

    SUBCASE("01 + 01 = 10")
    {
        s.get_node<Input>(a)->set(true);
        s.get_node<Input>(b)->set(true);
        s.get_node<Input>(c_in)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::FALSE);
        REQUIRE_EQ(s.get_node<Output>(c_out)->get(), State::TRUE);
    }

    SUBCASE("10 + 01 = 11")
    {
        s.get_node<Input>(a)->set(true);
        s.get_node<Input>(b)->set(true);
        s.get_node<Input>(c_in)->set(true);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
        REQUIRE_EQ(s.get_node<Output>(c_out)->get(), State::TRUE);
    }
}
