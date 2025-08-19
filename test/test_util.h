#pragma once

#define _create_full_adder_io(s)                                               \
    using namespace lcs;                                                       \
    Node a     = s.add_node<Input>();                                          \
    Node b     = s.add_node<Input>();                                          \
    Node c_in  = s.add_node<Input>();                                          \
    Node c_out = s.add_node<Output>();                                         \
    Node sum   = s.add_node<Output>();

#define _create_full_adder(s)                                                  \
    Node g_xor       = s.add_node<Gate>(Gate::Type::XOR);                      \
    Node g_xor_sum   = s.add_node<Gate>(Gate::Type::XOR);                      \
    Node g_and_carry = s.add_node<Gate>(Gate::Type::AND);                      \
    Node g_and       = s.add_node<Gate>(Gate::Type::AND);                      \
    Node g_or        = s.add_node<Gate>(Gate::Type::OR);                       \
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

#include "core.h"
inline bool scene_cmp(lcs::Scene& s1, lcs::Scene& s2)
{
    if (s1.name != s2.name || s1.description != s2.description
        || s1.version != s2.version || s1._inputs.size() != s2._inputs.size()
        || s1._outputs.size() != s2._outputs.size()
        || s1._gates.size() != s2._gates.size()
        || s1._components.size() != s2._components.size()
        || s1.component_context.has_value()
            != s2.component_context.has_value()) {
        return false;
    }
    return true;
}
