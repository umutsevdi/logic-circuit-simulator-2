#include "core.h"
#include <algorithm>
#include <utility>

namespace lcs {

Scene::Scene(const std::string& _name, const std::string& _author , const std::string& _description)
    : last_node {
        node { 0, node_t::GATE },
        node { 0, node_t::COMPONENT },
        node { 0, node_t::INPUT },
        node { 0, node_t::OUTPUT },
    }, last_rel{0}
    , meta {_name,_author,_description}

{
}

void Scene::remove_node(node id)
{
    if (id.id == 0) { lcs_assert(id.id != 0); }
    switch (id.type) {
    case node_t::GATE: {
        lcs_assert(id.id <= last_node[node_t::GATE].id);
        auto g = gates.find(id);
        lcs_assert(g != gates.end());
        for (auto r : g->second.output) {
            if (r != 0) { disconnect(r); }
        }
        for (auto r : g->second.inputs) {
            if (r != 0) { disconnect(r); }
        }
        gates.erase(gates.find(id));
        break;
    }
    case node_t::COMPONENT: {
        lcs_assert(id.id <= last_node[node_t::COMPONENT].id);
        auto c = components.find(id);
        lcs_assert(c != components.end());
        for (auto s : c->second.outputs) {
            for (auto r : s.second) {
                if (r != 0) { disconnect(r); }
            }
        }
        for (auto r : c->second.inputs) {
            if (r != 0) { disconnect(r); }
        }
        components.erase(components.find(id));
        break;
    }
    case node_t::INPUT: {
        lcs_assert(id.id <= last_node[node_t::INPUT].id);
        auto i = inputs.find(id);
        lcs_assert(i != inputs.end());
        for (auto r : i->second.output) {
            if (r != 0) { disconnect(r); }
        }
        inputs.erase(inputs.find(id));
        break;
    }
    case node_t::OUTPUT: {
        lcs_assert(id.id <= last_node[node_t::OUTPUT].id);
        auto o = outputs.find(id);
        lcs_assert(o != outputs.end());
        if (o->second.input != 0) { disconnect(o->second.input); }
        outputs.erase(outputs.find(id));
        break;
    }
    default: break;
    }
}

NRef<Rel> Scene::get_rel(relid idx)
{
    if (idx == 0 || idx > last_rel) {
        return S_ERROR("Invalid relid", nullptr);
    }
    auto n = rel.find(idx);
    return n != rel.end() ? &n->second : (S_ERROR("Rel not found", nullptr));
}

bool Scene::connect_with_id(relid& id, node to_node, sockid to_sock,
    node from_node, sockid from_sock) noexcept
{
    if (id == 0) {
        last_rel++;
        id = last_rel;
    }
    if (from_node.type == node_t::OUTPUT) {
        return S_ERROR("from_node can not be OUTPUT", false);
    } else if (from_node.type == node_t::COMPONENT
        && from_sock >= get_node<Component>(from_node)->outputs.size()) {
        lcs_assert("pff" && 0);
        return false;
    }

    bool is_connected = false;

    switch (to_node.type) {
    case node_t::GATE: {
        if (auto gate = get_node<Gate>(to_node);
            gate != nullptr && gate->inputs[to_sock] == 0) {
            gate->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case node_t::COMPONENT: {
        if (auto comp = get_node<Component>(to_node);
            comp != nullptr && comp->inputs[to_sock] == 0) {
            comp->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case node_t::OUTPUT: {
        if (auto out = get_node<Output>(to_node);
            out != nullptr && out->input == 0) {
            out->input   = id;
            is_connected = true;
        }
        break;
    }
    default: return S_ERROR("Attempted to connect to INPUT.", false); ;
    }
    if (!is_connected) { return S_ERROR("Already connected", false); }
    rel.emplace(id, Rel { id, from_node, to_node, from_sock, to_sock });

    switch (from_node.type) {
    case node_t::GATE:
        get_node<Gate>(from_node)->output.push_back(id);
        get_node<Gate>(from_node)->signal();
        break;
    case node_t::COMPONENT:
        get_node<Component>(from_node)->outputs[from_sock].push_back(id);
        get_node<Component>(from_node)->signal();
        break;
    case node_t::INPUT:
        get_node<Input>(from_node)->output.push_back(id);
        get_node<Input>(from_node)->signal();
        break;
    default: return S_ERROR("Attempted to signal from OUTPUT", false);
    }
    return true;
}

relid Scene::connect(
    node to_node, sockid to_sock, node from_node, sockid from_sock) noexcept
{
    relid id = 0;
    return connect_with_id(id, to_node, to_sock, from_node, from_sock) ? id : 0;
}

void Scene::disconnect(relid id)
{
    if (id == 0 || id > last_rel) {
        L_ERROR("Invalid rel id");
        return;
    }
    auto remove_fn = [id](relid i) { return i == id; };
    auto r         = rel.find(id);
    if (r == rel.end()) {
        L_ERROR("Rel not found.");
        return;
    }

    switch (r->second.from_node.type) {
    case node_t::GATE: {
        auto& v = get_node<Gate>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case node_t::COMPONENT: {
        auto& v = get_node<Component>(r->second.from_node)
                      ->outputs[r->second.from_sock];
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case node_t::INPUT: {
        auto& v = get_node<Input>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    default: lcs_assert(r->second.from_node.type == node_t::OUTPUT); break;
    }

    switch (r->second.to_node.type) {
    case node_t::GATE: {
        auto g = get_node<Gate>(r->second.to_node);

        g->inputs[r->second.to_sock] = 0;
        g->signal();
        break;
    }
    case node_t::COMPONENT: {
        auto c = get_node<Component>(r->second.to_node);

        c->inputs[r->second.to_sock] = 0;
        c->signal();
        break;
    }
    case node_t::OUTPUT: {
        auto o   = get_node<Output>(r->second.to_node);
        o->input = 0;
        o->signal();
        break;
    }
    default: lcs_assert(r->second.from_node.type == node_t::INPUT); break;
    }
    rel.erase(id);
}

void Scene::set_position(node n, point_t p) { get_base(n)->point = p; }

void Scene::invoke_signal(const std::vector<relid>& output, state_t value)
{
    for (relid id : output) {
        if (id == 0) { continue; }
        if (auto r = rel.find(id); r != rel.end()) {
            r->second.value = value;
            auto n          = get_base(r->second.to_node);
            if (n != nullptr) { n->signal(); }
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Scene&)
{
    os << "Scene[]";
    return os;
}

NRef<BaseNode> Scene::get_base(node id)
{
    switch (id.type) {
    case node_t::GATE: return get_node<Gate>(id)->get_base();
    case node_t::COMPONENT: return get_node<Component>(id)->get_base();
    case node_t::INPUT: return get_node<Input>(id)->get_base();
    case node_t::OUTPUT: return get_node<Output>(id)->get_base();
    default: break;
    }
    return nullptr;
}

} // namespace lcs
