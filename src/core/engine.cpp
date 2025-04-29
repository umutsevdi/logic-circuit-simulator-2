#include "common.h"
#include "core.h"

namespace lcs {

/******************************************************************************
                                    Rel
*****************************************************************************/

Rel::Rel(relid _id, node _from_node, node _to_node, sockid _from_sock,
    sockid _to_sock)
    : id { _id }
    , from_node { _from_node }
    , to_node { _to_node }
    , from_sock { _from_sock }
    , to_sock { _to_sock }
    , value { FALSE } { };

Rel::Rel()
    : id { 0 }
    , from_node {}
    , to_node {}
    , from_sock { 0 }
    , to_sock { 0 }
    , value { FALSE } { };

std::ostream& operator<<(std::ostream& os, const Rel& r)
{
    if (is_color_enabled()) { os << F_GREEN F_BOLD; }
    os << "rel@" << r.id;
    if (is_color_enabled()) { os << F_RESET; }
    os << "( from: " << r.from_node;
    if (is_color_enabled()) { os << F_BLUE; }
    os << "::" << r.from_sock;
    if (is_color_enabled()) { os << F_RESET; }
    os << ",\tto: " << r.to_node;
    if (is_color_enabled()) { os << F_BLUE; }
    os << "::" << r.to_sock;
    if (is_color_enabled()) { os << F_RESET; }
    os << ",\tvalue: " << state_t_str(r.value) << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BaseNode& g)
{
    os << g._id;
    return os;
}

/******************************************************************************
                                InputNode
*****************************************************************************/

InputNode::InputNode(Scene* _scene, node _id, std::optional<uint32_t> freq)
    : BaseNode { _scene, { _id.id, node_t::INPUT } }
    , _value { false }
    , _freq { freq } { };

std::ostream& operator<<(std::ostream& os, const InputNode& g)
{
    os << g.id() << "( value: " << state_t_str((state_t)g._value);
    if (g._freq.has_value()) { os << ", freq:" << g._freq.value(); }
    os << " )";
    return os;
};

void InputNode::set(bool v)
{
    _value = v;
    on_signal();
}

void InputNode::set_freq(uint32_t v)
{
    if (_freq.has_value() && v != 0) {
        _freq = v;
        on_signal();
    }
}

void InputNode::toggle()
{
    _value = !_value;
    on_signal();
}

void InputNode::on_signal()
{
    state_t result = _value ? state_t::TRUE : state_t::FALSE;
    for (relid& out : output) {
        L_INFO(CLASS "Sending " << state_t_str((state_t)_value)
                                << " signal to rel@" << out);
        _parent->signal(out, result);
    }
}

bool InputNode::is_connected() const { return true; };

state_t InputNode::get(sockid) const
{
    return _value ? state_t::TRUE : state_t::FALSE;
};

/******************************************************************************
                                  OutputNode
*****************************************************************************/

OutputNode::OutputNode(Scene* _scene, node _id)
    : BaseNode { _scene, { _id.id, node_t::OUTPUT } }
    , input { 0 }
    , _value { state_t::DISABLED } { };

std::ostream& operator<<(std::ostream& os, const OutputNode& g)
{
    os << g.id() << "( " << state_t_str(g._value) << " )";
    return os;
};

state_t OutputNode::get(sockid) const { return _value; }

bool OutputNode::is_connected() const { return input != 0; };

void OutputNode::on_signal()
{
    _value = input ? _parent->get_rel(input)->value : state_t::DISABLED;
    L_INFO(CLASS "Received " << state_t_str(_value) << " signal");
}

/******************************************************************************
                                    Context
*****************************************************************************/

BaseNode::BaseNode(Scene* scene, node id, direction_t _dir, point_t _p)
    : dir { _dir }
    , point { _p }
    , _parent { scene }
    , _id { id }
{
}

/******************************************************************************
                                Implementation
*****************************************************************************/

node::node(uint32_t _id, node_t _type)
    : id { _id }
    , type { _type }
{
}

std::ostream& operator<<(std::ostream& os, const node& r)
{
    if (is_color_enabled()) { os << F_GREEN F_BOLD; }
    os << node_to_str(r.type) << "@" << r.id;
    if (is_color_enabled()) { os << F_RESET; }
    return os;
}

gate_t str_to_gate(const std::string& type)
{
    if (type == "NOT")
        return NOT;
    else if (type == "AND")
        return AND;
    else if (type == "OR")
        return OR;
    else if (type == "XOR")
        return XOR;
    else if (type == "NAND")
        return NAND;
    else if (type == "NOR")
        return NOR;
    else
        return XNOR;
}

direction_t str_to_dir(const std::string& dir)
{
    if (dir == "r") {
        return direction_t::RIGHT;
    } else if (dir == "l") {
        return direction_t::LEFT;
    } else if (dir == "u") {
        return direction_t::UP;
    } else {
        return direction_t::DOWN;
    }
}

node_t str_to_node(const std::string& n)
{

    if (n == "gate") {
        return node_t::GATE;
    } else if (n == "comp") {
        return node_t::COMPONENT;
    } else if (n == "in") {
        return node_t::INPUT;
    } else if (n == "out") {
        return node_t::OUTPUT;
    } else if (n == "cin") {
        return node_t::COMPONENT_INPUT;
    } else if (n == "cout") {
        return node_t::COMPONENT_OUTPUT;
    }
    return node_t::NODE_S;
}

} // namespace lcs
