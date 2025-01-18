#pragma once
/*******************************************************************************
 * \file
 * File: /src/core/types.h
 * Created: 03/04/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "yaml-cpp/emitter.h"
#include <cstdint>
namespace lcs {

/** id type for socket, sock_t = 0 means disconnected */
typedef uint16_t sockid;
/** id nodes for socket, node_t = 0 means disconnected */
enum node_t : uint8_t {
    GATE,
    COMPONENT,
    INPUT,
    OUTPUT,
    DISPLAY,

    NODE_S
};
constexpr const char* node_to_str(node_t s)
{
    switch (s) {
    case node_t::GATE: return "GATE";
    case node_t::COMPONENT: return "COMPONENT";
    case node_t::INPUT: return "INPUT";
    case node_t::OUTPUT: return "OUTPUT";
    case node_t::DISPLAY: return "DISPLAY";
    default: return "UNKNOWN";
    }
}
node_t str_to_node(const std::string&);

struct node {
    node(uint32_t _id = 0, node_t _type = GATE);
    node(node&&)                 = default;
    node(const node&)            = default;
    node& operator=(node&&)      = default;
    node& operator=(const node&) = default;
    uint32_t id : 24;
    node_t type : 8;
    friend std::ostream& operator<<(std::ostream& os, const node& r);
    friend YAML::Emitter& operator<<(YAML::Emitter& out, const node& v);
    bool operator<(const node& n) const { return this->id < n.id; }
};

typedef uint32_t relid;

enum state_t {
    /** Socket evaluated to false. */
    FALSE,
    /** Socket evaluated to true. */
    TRUE,
    /** The socket provides no valuable information
     * due to the disconnection of at least one prior socket.
     */
    DISABLED,
};
constexpr const char* state_t_str(state_t s)
{
    return (s) == state_t::DISABLED ? "DISABLED"
        : (s) == state_t::TRUE      ? "TRUE"
                                    : "FALSE";
}
state_t str_to_state(const std::string&);

enum direction_t { RIGHT, DOWN, LEFT, UP };
constexpr const char* direction_to_str(direction_t s)
{
    return s == RIGHT ? "r" : s == DOWN ? "d" : s == LEFT ? "l" : "d";
}
direction_t str_to_dir(const std::string&);

enum gate_t {
    NOT,
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    XNOR,

    MAX
};
constexpr const char* gate_to_str(gate_t g)
{
    return g == NOT ? "NOT"
        : g == AND  ? "AND"
        : g == OR   ? "OR"
        : g == XOR  ? "XOR"
        : g == NAND ? "NAND"
        : g == NOR  ? "NOR"
        : g == XNOR ? "XNOR"
                    : "NaN";
}
gate_t str_to_gate(const std::string&);

struct point_t {
    int x;
    int y;

    friend YAML::Emitter& operator<<(YAML::Emitter& out, const point_t& v);

    inline bool is_zero() const { return x == 0 && y == 0; }
};

} // namespace lcs
