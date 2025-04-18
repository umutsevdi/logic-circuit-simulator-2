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

std::ostream& operator<<(std::ostream& os, const Rel& r)
{
    os << "Rel[" << r.id << "]\t{ from: " << r.from_node << "[" << r.from_sock
       << "],\tto: " << r.to_node << "[" << r.to_sock
       << "],\tvalue: " << r.value << "}";
    return os;
}

/******************************************************************************
                                Input
*****************************************************************************/

Input::Input(Scene* _scene, node _id)
    : BaseNode { _scene, { _id.id, node_t::INPUT } }
    , value { false }
    , freq { std::nullopt } { };

std::ostream& operator<<(std::ostream& os, const Input& g)
{

    os << g.id << "{ value:" << g.value << " }";
    return os;
};

void Input::set(bool v)
{
    value = v;
    signal();
}

void Input::set_freq(uint32_t v)
{
    freq = v;
    signal();
}

void Input::toggle()
{
    value = !value;
    signal();
}

void Input::signal()
{
    scene->invoke_signal(output, value ? state_t::TRUE : state_t::FALSE);
}

bool Input::is_connected() const { return true; };

state_t Input::get() { return value ? state_t::TRUE : state_t::FALSE; };

/******************************************************************************
                                  Output
*****************************************************************************/

Output::Output(Scene* _scene, node _id)
    : BaseNode { _scene, { _id.id, node_t::OUTPUT } }
    , input { 0 }
    , value { state_t::DISABLED } { };

std::ostream& operator<<(std::ostream& os, const Output& g)
{
    os << g.id << "{ input:" << g.value << " }";
    return os;
};

state_t Output::get() { return value; }

bool Output::is_connected() const { return input != 0; };

void Output::signal()
{
    value = input ? scene->get_rel(input)->value : state_t::DISABLED;
}

/******************************************************************************
                                    Context
*****************************************************************************/

BaseNode::BaseNode(Scene* _scene, node _id, direction_t _dir, point_t _p)
    : id { _id }
    , dir { _dir }
    , point { _p }
    , scene { _scene }
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
    os << node_to_str(r.type) << "[" << r.id << "]";
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

    if (n == "GATE") {
        return node_t::GATE;
    } else if (n == "COMPONENT") {
        return node_t::COMPONENT;
    } else if (n == "INPUT") {
        return node_t::INPUT;
    } else if (n == "OUTPUT") {
        return node_t::OUTPUT;
    } else if (n == "COMPIN") {
        return node_t::COMPONENT_INPUT;
    } else if (n == "COMPOUT") {
        return node_t::COMPONENT_OUTPUT;
    }
    return node_t::NODE_S;
}

} // namespace lcs
