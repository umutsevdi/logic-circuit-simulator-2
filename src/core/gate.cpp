#include "engine.h"
#include <algorithm>

namespace lcs {
static bool _and(const std::vector<bool>&);
static bool _or(const std::vector<bool>&);
static bool _xor(const std::vector<bool>&);
static bool _nand(const std::vector<bool>&);
static bool _nor(const std::vector<bool>&);
static bool _xnor(const std::vector<bool>&);
static bool _not(const std::vector<bool>&);

Gate::Gate(Scene* _scene, node _id, gate_t _type, sockid _max_in)
    : BaseNode { _scene, { _id.id, node_t::GATE } }
    , type { _type }
    , max_in { _max_in }
    , value { state_t::DISABLED }
{
    if (type == gate_t::NOT) {
        max_in = 1;
    } else if (max_in < 2) {
        max_in = 2;
    }
    inputs.reserve(max_in);
    for (size_t i = 0; i < max_in; i++) {
        inputs.push_back(0);
    }
    static bool (*__functions[])(const std::vector<bool>&)
        = { _not, _and, _or, _xor, _nand, _nor, _xnor, _not };
    _apply = __functions[type];
}

bool Gate::is_connected() const
{
    return std::all_of(
        inputs.begin(), inputs.end(), [&](relid i) { return i != 0; });
}

state_t Gate::get()
{
    if (!is_connected()) {
        value = state_t::DISABLED;
        return value;
    }
    value = state_t::DISABLED;
    std::vector<bool> v {};
    v.reserve(inputs.size());
    bool is_disabled = false;
    for (relid in : inputs) {
        L_INFO(<<in);
        auto rel = scene->get_rel(in);
        lcs_assert(rel != nullptr);
        if (rel->value == state_t::DISABLED) {
            is_disabled = true;
            break;
        }
        v.push_back(rel->value == TRUE ? TRUE : FALSE);
    }
    if (!is_disabled) { value = _apply(v) ? state_t::TRUE : state_t::FALSE; }
    return value;
}

void Gate::signal() { scene->invoke_signal(output, get()); }

void Gate::increment()
{
    if (type == gate_t::NOT) { return; }
    max_in++;
    inputs.reserve(max_in);
    inputs.push_back(0);
    signal();
}

void Gate::decrement()
{
    if (type == gate_t::NOT || max_in == 2) { return; }
    if (inputs[inputs.size() - 1] != 0) {
        scene->disconnect(inputs[inputs.size() - 1]);
    }
    inputs.pop_back();
    signal();
}

std::ostream& operator<<(std::ostream& os, const Gate& g)
{

    os << g.id << "{" << gate_to_str(g.type) << g.value << "}";
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
