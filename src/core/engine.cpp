#include "common.h"
#include "core.h"

namespace lcs {

/******************************************************************************
                                    Rel
*****************************************************************************/

Rel::Rel(relid _id, Node _from_node, Node _to_node, sockid _from_sock,
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
    if (is_color_enabled()) {
        os << F_GREEN F_BOLD;
    }
    os << "Rel@" << r.id;
    if (is_color_enabled()) {
        os << F_RESET;
    }
    os << "( from: " << r.from_node;
    if (is_color_enabled()) {
        os << F_BLUE;
    }
    os << "::" << std::to_string(r.from_sock);
    if (is_color_enabled()) {
        os << F_RESET;
    }
    os << ",\tto: " << r.to_node;
    if (is_color_enabled()) {
        os << F_BLUE;
    }
    os << "::" << std::to_string(r.to_sock);
    if (is_color_enabled()) {
        os << F_RESET;
    }
    os << ",\tvalue: " << State_to_str(r.value) << ")";
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

InputNode::InputNode(Scene* _scene, Node _id, std::optional<float> freq)
    : BaseNode { _scene, { _id.id, NodeType::INPUT } }
    , _freq { freq }
    , _value { false } { };

std::ostream& operator<<(std::ostream& os, const InputNode& g)
{
    os << g.id() << "( value: " << State_to_str((State)g._value);
    if (g._freq.has_value()) {
        os << ", freq:" << g._freq.value();
    }
    os << " )";
    return os;
};

void InputNode::set(bool v)
{
    _value = v;
    on_signal();
}

void InputNode::toggle()
{
    _value = !_value;
    on_signal();
}

void InputNode::on_signal()
{
    State result = _value ? State::TRUE : State::FALSE;
    for (relid& out : output) {
        L_INFO(CLASS "Sending " << State_to_str((State)_value)
                                << " signal to rel@" << out);
        _parent->signal(out, result);
    }
}

bool InputNode::is_connected() const { return true; };

State InputNode::get(sockid) const
{
    return _value ? State::TRUE : State::FALSE;
};

/******************************************************************************
                                  OutputNode
*****************************************************************************/

OutputNode::OutputNode(Scene* _scene, Node _id)
    : BaseNode { _scene, { _id.id, NodeType::OUTPUT } }
    , input { 0 }
    , _value { State::DISABLED } { };

std::ostream& operator<<(std::ostream& os, const OutputNode& g)
{
    os << g.id() << "( " << State_to_str(g._value) << " )";
    return os;
};

State OutputNode::get(sockid) const { return _value; }

bool OutputNode::is_connected() const { return input != 0; };

void OutputNode::on_signal()
{
    _value = input ? _parent->get_rel(input)->value : State::DISABLED;
    L_INFO(CLASS "Received " << State_to_str(_value) << " signal");
}

/******************************************************************************
                                    Context
*****************************************************************************/

BaseNode::BaseNode(Scene* scene, Node id, Point _p)
    : point { _p }
    , _parent { scene }
    , _id { id }
{
}

/******************************************************************************
                                Implementation
*****************************************************************************/

Node::Node(uint16_t _id, NodeType _type)
    : id { _id }
    , type { _type }
{
}

std::ostream& operator<<(std::ostream& os, const Node& r)
{
    if (is_color_enabled()) {
        os << F_GREEN F_BOLD;
    }
    os << NodeType_to_str(r.type) << "@" << r.id;
    if (is_color_enabled()) {
        os << F_RESET;
    }
    return os;
}

} // namespace lcs
