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

GateNode::GateNode(Scene* _scene, Node id, GateType type, sockid _max_in)
    : BaseNode { _scene, { id.id, NodeType::GATE } }
    , _type { type }
    , _value { State::DISABLED }
    , _is_disabled { true }
    , _max_in { _max_in }

{
    if (_type == GateType::NOT) {
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

State GateNode::get(sockid) const
{
    return _is_disabled ? State::DISABLED : _value;
}

void GateNode::on_signal()
{
    _value = State::DISABLED;
    if (is_connected()) {
        std::vector<bool> v {};
        v.reserve(inputs.size());
        _is_disabled = false;
        for (relid in : inputs) {
            auto rel = _parent->get_rel(in);
            lcs_assert(rel != nullptr);
            if (rel->value == State::DISABLED) {
                _is_disabled = true;
                break;
            }
            v.push_back(rel->value == TRUE ? TRUE : FALSE);
        }
        if (!_is_disabled) {
            _value = _apply(v) ? State::TRUE : State::FALSE;
        }
    } else {
        _is_disabled = true;
    }
    for (relid& out : output) {
        C_DEBUG("Sending %s signal to rel@%d", State_to_str(get()), out);
        _parent->signal(out, get());
    }
}

bool GateNode::increment()
{
    if (_type == GateType::NOT) {
        return false;
    }
    _max_in++;
    inputs.reserve(_max_in);
    inputs.push_back(0);
    on_signal();
    return true;
}

bool GateNode::decrement()
{
    if (_type == GateType::NOT || _max_in == 2) {
        return false;
    }
    if (inputs[inputs.size() - 1] != 0) {
        _parent->disconnect(inputs[inputs.size() - 1]);
    }
    _max_in--;
    inputs.pop_back();
    on_signal();
    return true;
}

static bool _and(const std::vector<bool>& in)
{
    for (const auto& s : in) {
        if (s == false) {
            return false;
        }
    }
    return true;
}
static bool _or(const std::vector<bool>& in)
{
    for (const auto& s : in) {
        if (s == true) {
            return true;
        }
    }
    return false;
}
static bool _xor(const std::vector<bool>& in)
{
    bool result = false;
    for (const auto& s : in) {
        if (s == true) {
            result = !result;
        }
    }
    return result;
}
static bool _nand(const std::vector<bool>& in) { return !_and(in); }
static bool _nor(const std::vector<bool>& in) { return !_or(in); }
static bool _xnor(const std::vector<bool>& in) { return !_xor(in); }
static bool _not(const std::vector<bool>& in) { return in[0] != true; }
} // namespace lcs
