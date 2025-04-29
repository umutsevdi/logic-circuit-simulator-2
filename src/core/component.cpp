#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "common.h"
#include "core.h"

namespace lcs {

ComponentContext::ComponentContext(
    Scene* parent, sockid input_s, sockid output_s)
    : inputs {}
    , outputs {}
    , _execution_input { 0 }
    , _execution_output { 0 }
    , _parent { parent }
{
    for (relid i = 1; i <= input_s; i++) {
        inputs[i] = {};
    }
    for (relid i = 1; i <= output_s; i++) {
        outputs[i] = 0;
    }
}

void ComponentContext::setup(sockid input_s, sockid output_s)
{
    if (inputs.size() > input_s) {
        for (auto& i : inputs) {
            if (i.first > input_s) {
                for (relid& id : i.second) {
                    _parent->disconnect(id);
                    id = 0;
                }
            }
        }
        inputs.erase(inputs.find(input_s), inputs.end());
    } else if (inputs.size() < input_s) {
        for (sockid i = inputs.size() + 1; i <= input_s; i++) {
            inputs[i] = {};
        }
    }
    if (outputs.size() > output_s) {
        for (auto& id : outputs) {
            if (id.first > output_s) {
                _parent->disconnect(id.second);
                id.second = 0;
            }
        }
        outputs.erase(outputs.find(output_s), outputs.end());
    } else if (outputs.size() < output_s) {
        for (sockid i = outputs.size() + 1; i <= output_s; i++) {
            outputs[i] = 0;
        }
    }

    _execution_input  = 0;
    _execution_output = 0;
}

node ComponentContext::get_input(sockid id) const
{
    if (id < inputs.size()) {
        return node { static_cast<uint32_t>(id + 1), node_t::COMPONENT_INPUT };
    }
    return {};
}

node ComponentContext::get_output(sockid id) const
{
    if (id < outputs.size()) {
        return node { static_cast<uint32_t>(id + 1), node_t::COMPONENT_OUTPUT };
    }
    return {};
}

void ComponentContext::set_value(sockid sock, state_t value)
{
    if (sock > 0 && sock < 64) {
        if (value == state_t::TRUE) {
            _execution_output |= 1 << sock;
        } else {
            _execution_output &= ~(1 << sock);
        }
    }
}

state_t ComponentContext::get_value(node id) const
{
    if (id.type == node_t::COMPONENT_INPUT) {
        if (auto in = inputs.find(id.id); in != inputs.end()) {
            return _execution_input & (1 << id.id) ? state_t::TRUE
                                                   : state_t::FALSE;
        }
    } else {
        if (auto in = outputs.find(id.id); in != outputs.end()) {
            return _execution_output & (1 << id.id) ? state_t::TRUE
                                                    : state_t::FALSE;
        }
    }
    return state_t::DISABLED;
}

uint64_t ComponentContext::run(uint64_t v)
{
    L_INFO("Execute component: " << v);
    v <<= 1;
    _execution_input  = v;
    _execution_output = 0;
    for (auto input : inputs) {
        state_t result
            = v & (1 << input.first) ? state_t::TRUE : state_t::FALSE;
        for (relid in : input.second) {
            _parent->signal(in, result);
        }
    }

    for (auto output : outputs) {
        if (output.second != 0) {
            _execution_output |= _parent->get_rel(output.second)->value
                ? (1 << output.first)
                : 0;
        }
    }
    L_INFO("Component result: " << (_execution_output >> 1));
    return _execution_output >> 1;
}

ComponentNode::ComponentNode(Scene* _s, node _id, const std::string& _path)
    : BaseNode { _s, node { _id.id, node_t::COMPONENT } }
    , _output_value { 0 }
{
    if (_path != "") { set_component(_path); }
}

error_t ComponentNode::set_component(const std::string& _path)
{
    if (_path == "") { return ERROR(error_t::INVALID_DEPENDENCY_FORMAT); }
    auto ref = io::component::get(_path);
    if (ref == nullptr) { return ERROR(error_t::COMPONENT_NOT_FOUND); }
    for (relid id : inputs) {
        _parent->disconnect(id);
    }
    for (auto out : outputs) {
        for (relid id : out.second) {
            _parent->disconnect(id);
        }
    }
    inputs.clear();
    outputs.clear();

    for (size_t i = 0; i < ref->component_context->inputs.size(); i++) {
        inputs.push_back(0);
    }
    for (size_t i = 0; i < ref->component_context->outputs.size(); i++) {
        outputs[i] = {};
    }
    path = _path;
    return error_t::OK;
}

bool ComponentNode::is_connected() const
{
    return std::all_of(
        inputs.begin(), inputs.end(), [&](relid i) { return i != 0; });
}

state_t ComponentNode::get(sockid id) const
{
    if (_is_disabled) { return state_t::DISABLED; }
    return _output_value & (1 << id) ? TRUE : FALSE;
}

void ComponentNode::on_signal()
{
    if (is_connected()) {
        uint64_t input = 0;
        _is_disabled   = false;
        for (relid in : inputs) {
            if (in != 0) {
                auto rel = _parent->get_rel(in);
                lcs_assert(rel != nullptr);
                if (rel->value == state_t::DISABLED) {
                    _is_disabled = true;
                    break;
                }
                input <<= 1;
                if (rel->value == TRUE) { input++; }
            }
        }
        if (!_is_disabled) { _output_value = io::component::run(path, input); }
    } else {
        _is_disabled = true;
    }

    for (auto sock : outputs) {
        for (relid out : sock.second) {
            L_INFO(CLASS "Sending " << state_t_str(get(sock.first))
                                    << " signal to rel@" << out);
            _parent->signal(out, get(sock.first));
        }
    }
}

std::ostream& operator<<(std::ostream& os, const ComponentNode& g)
{
    os << g._id << "( " << strlimit(g.path, 15) << " )";
    return os;
}

} // namespace lcs
