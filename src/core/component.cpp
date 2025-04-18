#include "core.h"
#include <iostream>

namespace lcs {

Component::Component(Scene* _s, node _id, const std::string& _path)
    : BaseNode { _s, node { _id.id, node_t::COMPONENT } }
    , path { _path }
{
}

bool Component::is_connected() const { return false; }

state_t Component::get() { return state_t::DISABLED; }

void Component::signal()
{
    L_INFO(CLASS << "Unimplemented");
    throw "Unimplemented";
}

std::ostream& operator<<(std::ostream& os, const Component& g)
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
