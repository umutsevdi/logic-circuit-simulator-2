#include "core.h"
#include <iostream>
#include <string>

namespace lcs {

ComponentContext::ComponentContext(relid input_s, relid output_s)
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
    , path { _path }
{
}

bool ComponentNode::is_connected() const { return false; }

state_t ComponentNode::get() { return state_t::DISABLED; }

void ComponentNode::signal()
{
    L_INFO(CLASS << "Unimplemented");
    throw "Unimplemented";
}

std::ostream& operator<<(std::ostream& os, const ComponentNode& g)
{
    os << "COMP [" << g.id << "]{inputs:";
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
