#include "core.h"

namespace lcs {

int encode_pair(Node node, sockid sock, bool is_out)
{
    lcs_assert(node.index < 0xFFFF);
    int x = node.index | (node.type << 16) | (sock << 20);
    if (is_out) {
        x |= 1 << 31;
    }
    return x;
}

Node decode_pair(int code, sockid* sock, bool* is_out)
{
    if (is_out != nullptr) {
        *is_out = code >> 31;
    }
    if (sock != nullptr) {
        *sock = (code >> 20) & 0xFF;
    }
    return Node { static_cast<uint16_t>(code & 0xFFFF),
        static_cast<Node::Type>((code >> 16) & 0x0F) };
}
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

InputNode::InputNode(Scene* _scene, std::optional<float> freq)
    : BaseNode { _scene }
    , _freq { freq }
    , _value { false }
{
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

void InputNode::on_signal(void)
{
    State sig = _value ? TRUE : FALSE;
    for (relid& out : output) {
        L_DEBUG("Sending %s signal to rel@%d", to_str<State>(sig), out);
        _parent->signal(out, sig);
    }
}

bool InputNode::is_connected() const { return true; }

State InputNode::get(sockid) const
{
    return _value ? State::TRUE : State::FALSE;
}

void InputNode::clean(void)
{
    if (is_timer()) {
        _parent->_timerlist.erase(_id);
    }
    for (auto r : output) {
        if (r != 0) {
            _parent->disconnect(r);
        }
    }
}

/******************************************************************************
                                  OutputNode
*****************************************************************************/

OutputNode::OutputNode(Scene* _scene)
    : BaseNode { _scene }
    , input { 0 }
    , _value { State::DISABLED }
{
}

State OutputNode::get(sockid) const { return _value; }

bool OutputNode::is_connected() const { return input != 0; }

void OutputNode::on_signal(void)
{
    if (input != 0) {
        auto r = _parent->get_rel(input);
        lcs_assert(r != nullptr);
        _value = r->value;
        L_DEBUG("Received %s signal", to_str<State>(r->value));
    }
}

void OutputNode::clean()
{
    if (input != 0) {
        _parent->disconnect(input);
    }
}

/******************************************************************************
                                    Context
*****************************************************************************/

BaseNode::BaseNode(Scene* scene, Point _p)
    : point { _p }
    , _parent { scene }
{
}

/******************************************************************************
                                Implementation
*****************************************************************************/

Node::Node(uint16_t _id, Node::Type _type)
    : index { _id }
    , type { _type }
{
}

template <> const char* to_str<Node>(Node node)
{
    static std::string s;
    s = std::string { to_str<Node::Type>(node.type) } + "@"
        + std::to_string(node.index);
    return s.c_str();
}

} // namespace lcs
