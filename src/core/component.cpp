#include "common.h"
#include "core.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace lcs {

ComponentContext::ComponentContext(sockid input_s, sockid output_s)
{
    for (relid i = 1; i <= input_s; i++) {
        inputs[i] = {};
    }
    for (relid i = 1; i <= output_s; i++) {
        outputs[i] = 0;
    }
    execution_input  = 0;
    execution_output = 0;
}

node ComponentContext::get_input(uint32_t id) const
{
    if (id != 0 && id <= inputs.size()) {
        return node { id, node_t::COMPONENT_INPUT };
    }
    return { 0 };
}

node ComponentContext::get_output(uint32_t id) const
{
    if (id != 0 && id <= outputs.size()) {
        return node { id, node_t::COMPONENT_OUTPUT };
    }
    return { 0 };
}

void ComponentContext::set_value(sockid sock, state_t value)
{
    if (sock > 0 && sock < 64) {
        if (value == state_t::TRUE) {
            execution_output |= 1 << sock;
        } else {
            execution_output &= ~(1 << sock);
        }
    }
}

state_t ComponentContext::get_value(node id) const
{
    if (id.type == node_t::COMPONENT_INPUT) {
        if (auto in = inputs.find(id.id); in != inputs.end()) {
            return execution_input & (1 << id.id) ? state_t::TRUE
                                                  : state_t::FALSE;
        }
    } else {
        if (auto in = outputs.find(id.id); in != outputs.end()) {
            return execution_output & (1 << id.id) ? state_t::TRUE
                                                   : state_t::FALSE;
        }
    }
    return state_t::DISABLED;
}

uint64_t ComponentContext::run(Scene* s, uint64_t v)
{
    L_INFO("Execute component: " << v);
    v <<= 1;
    s->component_context->execution_input  = v;
    s->component_context->execution_output = 0;
    for (auto input : s->component_context->inputs) {
        s->invoke_signal(input.second,
            v & (1 << input.first) ? state_t::TRUE : state_t::FALSE);
    }

    for (auto output : s->component_context->outputs) {
        if (output.second != 0) {
            s->component_context->execution_output
                |= s->get_rel(output.second)->value ? (1 << output.first) : 0;
        }
    }
    return s->component_context->execution_output >> 1;
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
    auto ref = sys::get_dependency(_path);
    if (ref == nullptr) { return ERROR(error_t::COMPONENT_NOT_FOUND); }
    // TODO add disconnect by relid
    for (size_t i = 0; i < ref->component_context->inputs.size(); i++) {
        inputs.push_back(0);
    }
    // TODO add disconnect by relid
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

void ComponentNode::signal()
{
    if (is_connected()) {
        uint64_t input = 0;
        _is_disabled   = false;
        for (relid in : inputs) {
            if (in != 0) {
                auto rel = scene->get_rel(in);
                lcs_assert(rel != nullptr);
                if (rel->value == state_t::DISABLED) {
                    _is_disabled = true;
                    break;
                }
                input <<= 1;
                if (rel->value == TRUE) { input++; }
            }
        }
        if (!_is_disabled) { _output_value = sys::run_component(path, input); }
    } else {
        _is_disabled = true;
    }

    for (auto out : outputs) {
        L_INFO(CLASS "Sending " << state_t_str(get(out.first)) << " signal");
        scene->invoke_signal(out.second, get(out.first));
    }
}

std::ostream& operator<<(std::ostream& os, const ComponentNode& g)
{
    os << "COMP [" << g.id << "](" << g.path << "){inputs:";
    for (auto i : g.inputs) {
        os << i << ", ";
    }
    os << ", output:";
    for (auto i : g.outputs) {
        os << i.first << ", ";
    }
    os << " }";
    return os;
}

} // namespace lcs
