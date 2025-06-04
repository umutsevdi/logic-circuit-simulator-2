#pragma once

#define _create_full_adder_io(s)                                               \
    using namespace lcs;                                                       \
    Node a     = s.add_node<InputNode>();                                      \
    Node b     = s.add_node<InputNode>();                                      \
    Node c_in  = s.add_node<InputNode>();                                      \
    Node c_out = s.add_node<OutputNode>();                                     \
    Node sum   = s.add_node<OutputNode>();

#define _create_full_adder(s)                                                  \
    Node g_xor       = s.add_node<GateNode>(GateType::XOR);                    \
    Node g_xor_sum   = s.add_node<GateNode>(GateType::XOR);                    \
    Node g_and_carry = s.add_node<GateNode>(GateType::AND);                    \
    Node g_and       = s.add_node<GateNode>(GateType::AND);                    \
    Node g_or        = s.add_node<GateNode>(GateType::OR);                     \
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
