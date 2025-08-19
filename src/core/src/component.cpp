#include "common.h"
#include "core.h"
#include <algorithm>
#include <bitset>
#include <cstdint>

namespace lcs {

ComponentContext::ComponentContext(
    Scene* parent, sockid input_s, sockid output_s)
    : _execution_input { 0 }
    , _execution_output { 0 }
    , _parent { parent }
{
    for (relid i = 0; i < input_s; i++) {
        inputs.push_back({});
    }
    for (relid i = 0; i < output_s; i++) {
        outputs.push_back(0);
    }
}

void ComponentContext::setup(sockid input_s, sockid output_s)
{
    if (inputs.size() > input_s) {
        for (size_t i = input_s; i < inputs.size(); i++) {
            for (relid& id : inputs[i]) {
                _parent->disconnect(id);
                id = 0;
            }
        }
        inputs.resize(input_s);
    } else if (inputs.size() < input_s) {
        inputs.resize(input_s);
    }
    if (outputs.size() > output_s) {
        for (size_t i = output_s; i < outputs.size(); i++) {
            if (outputs[i] != 0) {
                _parent->disconnect(outputs[i]);
            }
        }
        outputs.resize(output_s);
    } else if (outputs.size() < output_s) {
        outputs.resize(output_s);
    }
    _execution_input  = 0;
    _execution_output = 0;
}

Node ComponentContext::get_input(sockid id) const
{
    if (id < inputs.size()) {
        return Node { static_cast<uint16_t>(id + 1),
            Node::Type::COMPONENT_INPUT };
    }
    return {};
}

Node ComponentContext::get_output(sockid id) const
{
    if (id < outputs.size()) {
        return Node { static_cast<uint16_t>(id + 1),
            Node::Type::COMPONENT_OUTPUT };
    }
    return {};
}

State ComponentContext::get_value(Node id) const
{
    id.index -= 1;
    if (id.type == Node::Type::COMPONENT_INPUT) {
        if (id.index < inputs.size() && !inputs[id.index].empty()) {
            return _execution_input[id.index] ? State::TRUE : State::FALSE;
        }
    } else {
        if (id.index < outputs.size() && outputs[id.index] != 0) {
            return _execution_output[id.index] ? State::TRUE : State::FALSE;
        }
    }
    return State::DISABLED;
}

void ComponentContext::set_value(Node id, State value)
{
    if (id.index > 0 && id.index < 64) {
        if (id.type == Node::Type::COMPONENT_INPUT) {
            _execution_input[id.index - 1] = value == State::TRUE;
            run();
        } else {
            _execution_output[id.index - 1] = value == State::TRUE;
        }
    }
}

uint64_t ComponentContext::run()
{
    L_INFO("Execute component: %b", _execution_input.to_ullong());
    _execution_output = 0;
    for (size_t i = 0; i < inputs.size(); i++) {
        State result = _execution_input[i] ? State::TRUE : State::FALSE;
        for (relid in : inputs[i]) {
            _parent->signal(in, result);
        }
    }

    for (size_t i = 0; i < outputs.size(); i++) {
        if (outputs[i] != 0) {
            _execution_output.set(
                i, _parent->get_rel(outputs[i])->value == State::TRUE);
        }
    }
    L_INFO("Execute output: %b", _execution_output.to_ullong());
    return _execution_output.to_ullong();
}

uint64_t ComponentContext::run(uint64_t input, size_t frame)
{
    size_t old_frame = _parent->frame_s;
    _parent->frame_s = frame;
    _execution_input = input;
    uint64_t result  = run();
    _parent->frame_s = old_frame;
    return result;
}

Component::Component(Scene* _s)
    : BaseNode { _s }
    , dep_idx { UINT8_MAX }
    , _output_value { 0 }
{
}

Error Component::set_component(uint8_t _dep_idx)
{
    if (_dep_idx >= _parent->dependencies().size()) {
        return ERROR(Error::COMPONENT_NOT_FOUND);
    }
    auto& ref = _parent->dependencies()[_dep_idx];
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

    for (size_t i = 0; i < ref.component_context->inputs.size(); i++) {
        inputs.push_back(0);
    }
    for (size_t i = 0; i < ref.component_context->outputs.size(); i++) {
        outputs[i] = {};
    }
    dep_idx = _dep_idx;
    return OK;
}

bool Component::is_connected() const
{
    return std::all_of(
        inputs.begin(), inputs.end(), [&](relid i) { return i != 0; });
}

State Component::get(sockid id) const
{
    return _output_value & (1 << id) ? TRUE : FALSE;
}

void Component::clean(void)
{
    for (auto s : outputs) {
        for (auto r : s.second) {
            if (r != 0) {
                _parent->disconnect(r);
            }
        }
    }
    for (auto r : inputs) {
        if (r != 0) {
            _parent->disconnect(r);
        }
    }
}

void Component::on_signal(void)
{
    if (is_connected()) {
        uint64_t input = 0;
        for (relid in : inputs) {
            auto rel = _parent->get_rel(in);
            lcs_assert(rel != nullptr);
            input <<= 1;
            if (rel->value == TRUE) {
                input++;
            }
        }
        _output_value = _parent->run_dependency(dep_idx, input);
        for (auto sock : outputs) {
            for (relid out : sock.second) {
                L_DEBUG("Sending %s signal to rel@%d",
                    to_str<State>(get(sock.first)), out);
                _parent->signal(out, get(sock.first));
            }
        }
    }
}

} // namespace lcs
