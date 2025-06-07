#include <algorithm>
#include <bitset>

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
            NodeType::COMPONENT_INPUT };
    }
    return {};
}

Node ComponentContext::get_output(sockid id) const
{
    if (id < outputs.size()) {
        return Node { static_cast<uint16_t>(id + 1),
            NodeType::COMPONENT_OUTPUT };
    }
    return {};
}

State ComponentContext::get_value(Node id) const
{
    id.id -= 1;
    if (id.type == NodeType::COMPONENT_INPUT) {
        if (id.id < inputs.size() && !inputs[id.id].empty()) {
            return _execution_input[id.id] ? State::TRUE : State::FALSE;
        }
    } else {
        if (id.id < outputs.size() && outputs[id.id] != 0) {
            return _execution_output[id.id] ? State::TRUE : State::FALSE;
        }
    }
    return State::DISABLED;
}

void ComponentContext::set_value(Node id, State value)
{
    if (id.id > 0 && id.id < 64) {
        if (id.type == NodeType::COMPONENT_INPUT) {
            _execution_input[id.id - 1] = value == State::TRUE;
        } else {
            _execution_output[id.id - 1] = value == State::TRUE;
        }
    }
}

uint64_t ComponentContext::run()
{
    L_INFO("Execute component: " << _execution_input);
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
    L_INFO("Component result: " << _execution_output);
    return _execution_output.to_ullong();
}

uint64_t ComponentContext::run(uint64_t v)
{
    _execution_input = v;
    return run();
}

ComponentNode::ComponentNode(Scene* _s, Node _id, const std::string& _path)
    : BaseNode { _s, Node { _id.id, NodeType::COMPONENT } }
    , _output_value { 0 }
{
    if (_path != "") {
        set_component(_path);
    }
}

Error ComponentNode::set_component(const std::string& _path)
{
    if (_path == "") {
        return ERROR(Error::INVALID_DEPENDENCY_FORMAT);
    }
    auto ref = io::component::get(_path);
    if (ref == nullptr) {
        return ERROR(Error::COMPONENT_NOT_FOUND);
    }
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
    return Error::OK;
}

bool ComponentNode::is_connected() const
{
    return std::all_of(
        inputs.begin(), inputs.end(), [&](relid i) { return i != 0; });
}

State ComponentNode::get(sockid id) const
{
    if (_is_disabled) {
        return State::DISABLED;
    }
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
                if (rel->value == State::DISABLED) {
                    _is_disabled = true;
                    break;
                }
                input <<= 1;
                if (rel->value == TRUE) {
                    input++;
                }
            }
        }
        if (!_is_disabled) {
            _output_value = io::component::run(path, input);
        }
    } else {
        _is_disabled = true;
    }

    for (auto sock : outputs) {
        for (relid out : sock.second) {
            L_INFO(CLASS "Sending " << State_to_str(get(sock.first))
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
