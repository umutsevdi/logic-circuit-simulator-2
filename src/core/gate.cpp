#include <algorithm>

#include "common.h"
#include "core.h"

namespace lcs {
static bool _and(const std::vector<bool>&);
static bool _or(const std::vector<bool>&);
static bool _xor(const std::vector<bool>&);
static bool _nand(const std::vector<bool>&);
static bool _nor(const std::vector<bool>&);
static bool _xnor(const std::vector<bool>&);
static bool _not(const std::vector<bool>&);

GateNode::GateNode(Scene* _scene, node id, gate_t type, sockid _max_in)
    : BaseNode { _scene, { id.id, node_t::GATE } }
    , inputs {}
    , output {}
    , _type { type }
    , _value { state_t::DISABLED }
    , _is_disabled { true }
    , _max_in { _max_in }

{
    if (_type == gate_t::NOT) {
        _max_in = 1;
    } else if (_max_in < 2) {
        _max_in = 2;
    }
    inputs.reserve(_max_in);
    for (size_t i = 0; i < _max_in; i++) {
        inputs.push_back(0);
    }
    static bool (*__functions[])(const std::vector<bool>&)
        = { _not, _and, _or, _xor, _nand, _nor, _xnor, _not };
    _apply = __functions[_type];
}

bool GateNode::is_connected() const
{
    return std::all_of(
        inputs.begin(), inputs.end(), [&](relid i) { return i != 0; });
}

state_t GateNode::get(sockid) const
{
    return _is_disabled ? state_t::DISABLED : _value;
}

void GateNode::on_signal()
{
    _value = state_t::DISABLED;
    if (is_connected()) {
        std::vector<bool> v {};
        v.reserve(inputs.size());
        _is_disabled = false;
        for (relid in : inputs) {
            auto rel = _parent->get_rel(in);
            lcs_assert(rel != nullptr);
            if (rel->value == state_t::DISABLED) {
                _is_disabled = true;
                break;
            }
            v.push_back(rel->value == TRUE ? TRUE : FALSE);
        }
        if (!_is_disabled) {
            _value = _apply(v) ? state_t::TRUE : state_t::FALSE;
        }
    } else {
        _is_disabled = true;
    }
    for (relid& out : output) {
        L_INFO(
            CLASS "Sending " << state_t_str(get()) << " signal to rel@" << out);
        _parent->signal(out, get());
    }
}

bool GateNode::increment()
{
    if (_type == gate_t::NOT) { return false; }
    _max_in++;
    inputs.reserve(_max_in);
    inputs.push_back(0);
    on_signal();
    return true;
}

bool GateNode::decrement()
{
    if (_type == gate_t::NOT || _max_in == 2) { return false; }
    if (inputs[inputs.size() - 1] != 0) {
        _parent->disconnect(inputs[inputs.size() - 1]);
    }
    inputs.pop_back();
    on_signal();
    return true;
}

std::ostream& operator<<(std::ostream& os, const GateNode& g)
{

    os << g.id() << "( " << gate_to_str(g.type()) << ", "
       << state_t_str(g._value) << " )";
    return os;
};

static bool _and(const std::vector<bool>& in)
{
    for (const auto& s : in) {
        if (s == false) { return false; }
    }
    return true;
}
static bool _or(const std::vector<bool>& in)
{
    for (const auto& s : in) {
        if (s == true) { return true; }
    }
    return false;
}
static bool _xor(const std::vector<bool>& in)
{
    bool result = false;
    for (const auto& s : in) {
        if (s == true) { result = !result; }
    }
    return result;
}
static bool _nand(const std::vector<bool>& in) { return !_and(in); }
static bool _nor(const std::vector<bool>& in) { return !_or(in); }
static bool _xnor(const std::vector<bool>& in) { return !_xor(in); }
static bool _not(const std::vector<bool>& in) { return in[0] != true; }
} // namespace lcs
