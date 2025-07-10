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
    , value { FALSE }
{
}

Rel::Rel()
    : id { 0 }
    , from_sock { 0 }
    , to_sock { 0 }
    , value { FALSE }
{
}

/******************************************************************************
                                InputNode
*****************************************************************************/

InputNode::InputNode(Scene* _scene, Node id, std::optional<float> freq)
    : BaseNode { _scene, { id.id, NodeType::INPUT } }
    , _freq { freq }
    , _value { false }
{
    if (_freq.has_value()) {
        _parent->_timerlist.emplace(_id, 0);
    }
}

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
        C_DEBUG(
            "Sending %s signal to rel@%d", State_to_str((State)_value), out);
        _parent->signal(out, result);
    }
}

bool InputNode::is_connected() const { return true; }

State InputNode::get(sockid) const
{
    return _value ? State::TRUE : State::FALSE;
}

/******************************************************************************
                                  OutputNode
*****************************************************************************/

OutputNode::OutputNode(Scene* _scene, Node _id)
    : BaseNode { _scene, { _id.id, NodeType::OUTPUT } }
    , input { 0 }
    , _value { State::DISABLED }
{
}

State OutputNode::get(sockid) const { return _value; }

bool OutputNode::is_connected() const { return input != 0; }

void OutputNode::on_signal()
{
    _value = input ? _parent->get_rel(input)->value : State::DISABLED;
    C_DEBUG("Received %s signal", State_to_str(_value));
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

std::string Node::to_str(void) const
{
    return std::string { NodeType_to_str_full(type) } + "@"
        + std::to_string(id);
}

} // namespace lcs
