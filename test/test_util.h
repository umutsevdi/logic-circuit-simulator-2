#pragma once
#include "core.h"

#define _create_full_adder_io(s)                                               \
    using namespace lcs;                                                       \
    node a     = s.add_node<InputNode>();                                      \
    node b     = s.add_node<InputNode>();                                      \
    node c_in  = s.add_node<InputNode>();                                      \
    node c_out = s.add_node<OutputNode>();                                     \
    node sum   = s.add_node<OutputNode>();

#define _create_full_adder(s)                                                  \
    node g_xor       = s.add_node<GateNode>(gate_t::XOR);                      \
    node g_xor_sum   = s.add_node<GateNode>(gate_t::XOR);                      \
    node g_and_carry = s.add_node<GateNode>(gate_t::AND);                      \
    node g_and       = s.add_node<GateNode>(gate_t::AND);                      \
    node g_or        = s.add_node<GateNode>(gate_t::OR);                       \
                                                                               \
    s.connect(g_xor, 0, a);                                                    \
    s.connect(g_xor, 1, b);                                                    \
                                                                               \
    s.connect(g_xor_sum, 0, g_xor);                                            \
    s.connect(g_xor_sum, 1, c_in);                                             \
    s.connect(sum, 0, g_xor_sum);                                              \
                                                                               \
    s.connect(g_and, 0, c_in);                                                 \
    s.connect(g_and, 1, g_xor);                                                \
                                                                               \
    s.connect(g_and_carry, 0, a);                                              \
    s.connect(g_and_carry, 1, b);                                              \
                                                                               \
    s.connect(g_or, 0, g_and);                                                 \
    s.connect(g_or, 1, g_and_carry);                                           \
    s.connect(c_out, 0, g_or);
