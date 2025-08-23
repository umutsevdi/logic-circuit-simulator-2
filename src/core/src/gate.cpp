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
static bool (*_operations[])(const std::vector<bool>&)
    = { _not, _and, _or, _xor, _nand, _nor, _xnor, _not };

Gate::Gate(Scene* _scene, Type type, sockid _max_in)
    : BaseNode { _scene }
    , _type { type }
    , _value { State::DISABLED }

{
    if (type == Type::NOT) {
        _max_in = 1;
    } else if (_max_in < 2) {
        _max_in = 2;
    }
    inputs.reserve(_max_in);
    for (size_t i = 0; i < _max_in; i++) {
        inputs.push_back(0);
    }
}

bool Gate::is_connected() const
{
    return std::all_of(
        inputs.begin(), inputs.end(), [&](relid i) { return i != 0; });
}

State Gate::get(sockid) const { return _value; }

void Gate::clean(void)
{
    for (auto r : output) {
        if (r != 0) {
            _parent->disconnect(r);
        }
    }
    for (auto r : inputs) {
        if (r != 0) {
            _parent->disconnect(r);
        }
    }
}

void Gate::on_signal(void)
{
    if (is_connected()) {
        std::vector<bool> v {};
        v.reserve(inputs.size());
        for (relid in : inputs) {
            auto rel = _parent->get_rel(in);
            lcs_assert(rel != nullptr);
            v.push_back(rel->value == TRUE ? TRUE : FALSE);
        }
        _value = _operations[_type](v) ? State::TRUE : State::FALSE;
    } else {
        _value = State::DISABLED;
    }
    for (relid& out : output) {
        _parent->signal(out, get());
    }
}

bool Gate::increment()
{
    if (_type == Type::NOT) {
        return false;
    }
    inputs.push_back(0);
    on_signal();
    return true;
}

bool Gate::decrement()
{
    if (_type == Type::NOT || inputs.size() == 2) {
        return false;
    }
    if (inputs[inputs.size() - 1] != 0) {
        _parent->disconnect(inputs[inputs.size() - 1]);
    }
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
