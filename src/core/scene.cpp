#include <base64.h>

#include <algorithm>
#include <utility>

#include "common.h"
#include "core.h"

namespace lcs {

Scene::Scene(const std::string& _name, const std::string& _author , const 
std::string& _description, int _version)
    :     name{_name},description{_description},author{_author}, 
version{_version}
    , component_context{std::nullopt} ,_last_node {
        node { 0, node_t::GATE },
        node { 0, node_t::COMPONENT },
        node { 0, node_t::INPUT },
        node { 0, node_t::OUTPUT },
    }  ,
        _last_rel{0}
{
}

Scene::Scene(ComponentContext ctx, const std::string& _name, const std::string& 
_author , const std::string& _description, int _version)
    :     name{_name},description{_description},author{_author}, 
version{_version}
    , component_context{ctx}
    , _last_node {
            node { 0, node_t::GATE },
            node { 0, node_t::COMPONENT },
            node { 0, node_t::INPUT },
            node { 0, node_t::OUTPUT },
      }
    , _last_rel{0}
{
}

Scene::Scene(Scene&& other)
{
    if (this != &other) { _move_from(std::move(other)); }
}

Scene& Scene::operator=(Scene&& other)
{
    if (this != &other) { _move_from(std::move(other)); }
    return *this;
}

void Scene::_move_from(Scene&& other)
{

    name              = std::move(other.name);
    description       = std::move(other.description);
    author            = std::move(other.author);
    version           = other.version;
    dependencies      = std::move(other.dependencies);
    timer_sub_vec     = std::move(other.timer_sub_vec);
    _gates            = std::move(other._gates);
    _components       = std::move(other._components);
    _inputs           = std::move(other._inputs);
    _outputs          = std::move(other._outputs);
    _relations        = std::move(other._relations);
    component_context = std::move(other.component_context);
    for (size_t i = 0; i < node_t::NODE_S; i++) {
        _last_node[i] = other._last_node[i];
    }
    _last_rel = other._last_rel;

    for (auto& gate : _gates) {
        gate.second.reload(this);
    }
    for (auto& comp : _components) {
        comp.second.reload(this);
    }
    for (auto& input : _inputs) {
        input.second.reload(this);
    }
    for (auto& output : _outputs) {
        output.second.reload(this);
    }
    if (component_context.has_value()) { component_context->reload(this); }
}

void Scene::remove_node(node id)
{
    if (id.id == 0) { lcs_assert(id.id != 0); }
    switch (id.type) {
    case node_t::GATE: {
        lcs_assert(id.id <= _last_node[node_t::GATE].id);
        auto g = _gates.find(id);
        lcs_assert(g != _gates.end());
        for (auto r : g->second.output) {
            if (r != 0) { disconnect(r); }
        }
        for (auto r : g->second.inputs) {
            if (r != 0) { disconnect(r); }
        }
        _gates.erase(_gates.find(id));
        break;
    }
    case node_t::COMPONENT: {
        lcs_assert(id.id <= _last_node[node_t::COMPONENT].id);
        auto c = _components.find(id);
        lcs_assert(c != _components.end());
        for (auto s : c->second.outputs) {
            for (auto r : s.second) {
                if (r != 0) { disconnect(r); }
            }
        }
        for (auto r : c->second.inputs) {
            if (r != 0) { disconnect(r); }
        }
        _components.erase(_components.find(id));
        break;
    }
    case node_t::INPUT: {
        lcs_assert(id.id <= _last_node[node_t::INPUT].id);
        auto i = _inputs.find(id);
        lcs_assert(i != _inputs.end());
        for (auto r : i->second.output) {
            if (r != 0) { disconnect(r); }
        }
        _inputs.erase(_inputs.find(id));
        break;
    }
    case node_t::OUTPUT: {
        lcs_assert(id.id <= _last_node[node_t::OUTPUT].id);
        auto o = _outputs.find(id);
        lcs_assert(o != _outputs.end());
        if (o->second.input != 0) { disconnect(o->second.input); }
        _outputs.erase(_outputs.find(id));
        break;
    }
    default: break;
    }
}

NRef<Rel> Scene::get_rel(relid idx)
{
    if (idx == 0 || idx > _last_rel) { return nullptr; }
    auto n = _relations.find(idx);
    return n != _relations.end() ? &n->second
                                 : (S_ERROR("Rel not found", nullptr));
}

error_t Scene::_connect_with_id(
    relid id, node to_node, sockid to_sock, node from_node, sockid from_sock)
{
    if (from_node.type == node_t::OUTPUT
        || from_node.type == node_t::COMPONENT_OUTPUT) {
        return ERROR(error_t::INVALID_FROM_TYPE);
    } else if (from_node.type == node_t::COMPONENT
        && from_sock
            >= io::component::get(get_node<ComponentNode>(from_node)->path)
                ->component_context->outputs.size()) {
        return ERROR(error_t::INVALID_NODEID);
    }
    if (!component_context.has_value()
        && (to_node.type == node_t::COMPONENT_OUTPUT
            || from_node.type == node_t::COMPONENT_INPUT)) {
        return ERROR(error_t::NOT_A_COMPONENT);
    }

    bool is_connected = false;
    switch (to_node.type) {
    case node_t::GATE: {
        if (auto gate = get_node<GateNode>(to_node);
            gate != nullptr && gate->inputs[to_sock] == 0) {
            gate->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case node_t::COMPONENT: {
        if (auto comp = get_node<ComponentNode>(to_node);
            comp != nullptr && comp->inputs[to_sock] == 0) {
            comp->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case node_t::OUTPUT: {
        if (auto out = get_node<OutputNode>(to_node);
            out != nullptr && out->input == 0) {
            out->input   = id;
            is_connected = true;
        }
        break;
    }
    case node_t::COMPONENT_OUTPUT: {
        if (auto compout = component_context->outputs.find(to_node.id);
            compout != component_context->outputs.end()) {
            if (compout->second != 0) {
                return ERROR(error_t::ALREADY_CONNECTED);
            }
            compout->second = id;
            is_connected    = true;
        }
        break;
    }
    default: return ERROR(error_t::INVALID_TO_TYPE);
    }
    if (!is_connected) { return ERROR(error_t::ALREADY_CONNECTED); }
    _relations.emplace(id, Rel { id, from_node, to_node, from_sock, to_sock });

    switch (from_node.type) {
    case node_t::GATE:
        get_node<GateNode>(from_node)->output.push_back(id);
        get_node<GateNode>(from_node)->on_signal();
        break;
    case node_t::COMPONENT:
        get_node<ComponentNode>(from_node)->outputs[from_sock].push_back(id);
        get_node<ComponentNode>(from_node)->on_signal();
        break;
    case node_t::INPUT:
        get_node<InputNode>(from_node)->output.push_back(id);
        get_node<InputNode>(from_node)->on_signal();
        break;
    case node_t::COMPONENT_INPUT: /* Component input is not handled
                                     here. */
        lcs_assert(component_context.has_value());
        if (auto compin = component_context->inputs.find(from_node.id);
            compin != component_context->inputs.end()) {
            for (auto _id : compin->second) {
                if (_id == id) { return ERROR(error_t::ALREADY_CONNECTED); }
            }
            compin->second.push_back(id);
        }
        component_context->run(0);
        break;
    default: return ERROR(error_t::INVALID_TO_TYPE);
    }
    return error_t::OK;
}

relid Scene::connect(
    node to_node, sockid to_sock, node from_node, sockid from_sock)
{
    _last_rel++;
    relid id = _last_rel;
    return _connect_with_id(id, to_node, to_sock, from_node, from_sock) ? 0
                                                                        : id;
}

error_t Scene::disconnect(relid id)
{
    if (id == 0 || id > _last_rel) { return ERROR(error_t::INVALID_RELID); }
    auto remove_fn = [id](relid i) { return i == id; };
    auto r         = _relations.find(id);
    if (r == _relations.end()) { return ERROR(error_t::REL_NOT_FOUND); }

    switch (r->second.from_node.type) {
    case node_t::GATE: {
        auto& v = get_node<GateNode>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case node_t::COMPONENT: {
        auto& v = get_node<ComponentNode>(r->second.from_node)
                      ->outputs[r->second.from_sock];
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case node_t::INPUT: {
        auto& v = get_node<InputNode>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case node_t::COMPONENT_INPUT: {
        if (auto inputitr
            = component_context->inputs.find(r->second.from_node.id);
            inputitr != component_context->inputs.end()) {
            auto& v = inputitr->second;
            v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        } else {
            return ERROR(error_t::NOT_CONNECTED);
        }
        break;
    }
    default: lcs_assert(r->second.from_node.type == node_t::OUTPUT); break;
    }

    switch (r->second.to_node.type) {
    case node_t::GATE: {
        auto g = get_node<GateNode>(r->second.to_node);

        g->inputs[r->second.to_sock] = 0;
        g->on_signal();
        break;
    }
    case node_t::COMPONENT: {
        auto c = get_node<ComponentNode>(r->second.to_node);

        c->inputs[r->second.to_sock] = 0;
        c->on_signal();
        break;
    }
    case node_t::OUTPUT: {
        auto o   = get_node<OutputNode>(r->second.to_node);
        o->input = 0;
        o->on_signal();
        break;
    }
    case node_t::COMPONENT_OUTPUT: {
        lcs_assert(component_context.has_value());
        if (auto outitr = component_context->outputs.find(r->second.to_node.id);
            outitr != component_context->outputs.end()) {
            outitr->second = 0;
            component_context->run(0);
        } else {
            return ERROR(error_t::NOT_CONNECTED);
        }
        break;
    }
    default: lcs_assert(r->second.from_node.type == node_t::INPUT); break;
    }
    _relations.erase(id);
    return error_t::OK;
}

void Scene::signal(relid id, state_t value)
{
    if (id == 0) { return; }
    if (auto r = _relations.find(id); r != _relations.end()) {
        r->second.value = value;
        L_INFO(r->second);
        if (r->second.to_node.type != node_t::COMPONENT_OUTPUT) {
            auto n = get_base(r->second.to_node);
            if (n != nullptr) { n->on_signal(); }
        } else {
            component_context->set_value(r->second.to_node.id, r->second.value);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Scene& s)
{
    std::string d_str {};
    os << "scene" << (s.component_context.has_value() ? "[C]" : "[S]") << "("
       << strlimit(s.to_dependency(), 15) << ")";
    if (s.component_context.has_value()) {
        os << "[" << s.component_context->inputs.size() << ", "
           << s.component_context->outputs.size() << "]" << "\t";
    }
    return os;
}

NRef<BaseNode> Scene::get_base(node id)
{
    switch (id.type) {
    case node_t::GATE: return get_node<GateNode>(id)->get_base();
    case node_t::COMPONENT: return get_node<ComponentNode>(id)->get_base();
    case node_t::INPUT: return get_node<InputNode>(id)->get_base();
    case node_t::OUTPUT: return get_node<OutputNode>(id)->get_base();
    default: break;
    }
    return nullptr;
}

std::string Scene::to_dependency() const
{
    std::string dep_str = "";
    if (author == "sys") {
        dep_str = '@';
    } else if (author == "") {
        dep_str = "local/";
    } else {
        dep_str = author + '/';
    }
    return dep_str + name + '/' + std::to_string(version);
}

error_t Scene::load_dependencies(void)
{
    for (const std::string& dep : dependencies) {
        error_t err = io::component::fetch(dep);
        if (err) { return err; }
    }
    return error_t::OK;
}

std::string Scene::to_filepath(void) const
{
    std::filesystem::path p;
    if (author == "" || author == "local") {
        p = io::LOCAL
            / (base64_encode(name + "/" + std::to_string(version), true)
                + ".json");
    } else {
        p = io::LIBRARY / author
            / (base64_encode(name + "/" + std::to_string(version), true)
                + ".json");
    }
    return p;
}

} // namespace lcs
