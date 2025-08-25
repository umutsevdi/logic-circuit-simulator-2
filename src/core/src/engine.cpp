#include "core.h"

namespace lcs {

template <> const char* to_str<Node::Type>(Node::Type s)
{
    switch (s) {
    case Node::Type::GATE: return _("Gate");
    case Node::Type::COMPONENT: return _("Component");
    case Node::Type::INPUT: return _("Input");
    case Node::Type::OUTPUT: return _("Output");
    case Node::Type::COMPONENT_INPUT: return _("Component Input");
    case Node::Type::COMPONENT_OUTPUT: return _("Component Output");
    default: return "Unknown";
    }
}

template <> const char* to_str<Node>(Node node)
{
    static std::array<char, 64> buf;
    std::snprintf(buf.data(), buf.size(), "%s@%d",
        to_str<Node::Type>(node.type), node.index);
    return buf.data();
}

template <> const char* to_str<State>(State s)
{
    switch (s) {
    case State::TRUE: return "TRUE";
    case State::FALSE: return "FALSE";
    default: return _("DISABLED");
    }
}

template <> const char* to_str<Gate::Type>(Gate::Type s)
{
    switch (s) {
    case Gate::Type::NOT: return "NOT";
    case Gate::Type::AND: return "AND";
    case Gate::Type::OR: return "OR";
    case Gate::Type::XOR: return "XOR";
    case Gate::Type::NAND: return "NAND";
    case Gate::Type::NOR: return "NOR";
    case Gate::Type::XNOR: return "XNOR";
    default: return "null";
    }
}

int encode_pair(Node node, sockid sock, bool is_out)
{
    lcs_assert(node.index < 0xFFFF);
    int x = node.index | (node.type << 16) | (sock << 20);
    if (is_out) {
        x |= 1 << 29;
    }
    return x;
}

Node decode_pair(int code, sockid* sock, bool* is_out)
{
    if (is_out != nullptr) {
        *is_out = (code >> 29) & 1;
    }
    if (sock != nullptr) {
        *sock = (code >> 20) & 0xFF;
    }
    return Node { static_cast<uint16_t>(code & 0xFFFF),
        static_cast<Node::Type>((code >> 16) & 0x0F) };
}

/******************************************************************************
                                InputNode
*****************************************************************************/

void Input::set(bool v)
{
    _value = v;
    on_signal();
}

void Input::toggle()
{
    _value = !_value;
    on_signal();
}

void Input::on_signal(void)
{
    State sig = _value ? TRUE : FALSE;
    for (relid& out : output) {
        _parent->signal(out, sig);
    }
}

bool Input::is_connected() const { return true; }

State Input::get(sockid) const { return _value ? State::TRUE : State::FALSE; }

void Input::clean(void)
{
    for (auto r : output) {
        if (r != 0) {
            _parent->disconnect(r);
        }
    }
}

/******************************************************************************
                                  OutputNode
*****************************************************************************/

State Output::get(sockid) const { return _value; }

bool Output::is_connected() const { return input != 0; }

void Output::on_signal(void)
{
    if (input != 0) {
        auto r = _parent->get_rel(input);
        lcs_assert(r != nullptr);
        _value = r->value;
    } else {
        _value = DISABLED;
    }
    L_DEBUG("Received %s signal", to_str<State>(_value));
}

void Output::clean()
{
    if (input != 0) {
        _parent->disconnect(input);
    }
}

/******************************************************************************
                                    Context
*****************************************************************************/

} // namespace lcs
