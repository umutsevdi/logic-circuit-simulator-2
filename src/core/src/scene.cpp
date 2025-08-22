#include "common.h"
#include "core.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <utility>

namespace lcs {

Scene::Scene(const std::string& _name, const std::string& _author,
        const std::string& _description, int _version) :
        version { _version }, component_context { std::nullopt }, frame_s{0},
    _last_node {
            Node { 0, Node::Type::GATE },
            Node { 0, Node::Type::COMPONENT },
            Node { 0, Node::Type::INPUT },
            Node { 0, Node::Type::OUTPUT },
        },
        _last_rel { 0 }
{
    std::strncpy(name.data(), _name.c_str(), name.size() - 1);
    std::strncpy(author.data(), _author.c_str(), author.size() - 1);
    std::strncpy(
        description.data(), _description.c_str(), description.size() - 1);
}

Scene::Scene(ComponentContext ctx, const std::string& _name,
        const std::string& _author, const std::string& _description, int _version) :
        version { _version }, component_context { ctx }, frame_s{0},
        _last_node {
            Node { 0, Node::Type::GATE },
            Node { 0, Node::Type::COMPONENT },
            Node { 0, Node::Type::INPUT },
            Node { 0, Node::Type::OUTPUT },
        },
        _last_rel { 0 }
{
    std::strncpy(name.data(), _name.c_str(), name.size() - 1);
    std::strncpy(author.data(), _author.c_str(), author.size() - 1);
    std::strncpy(
        description.data(), _description.c_str(), description.size() - 1);
}

Scene::Scene(Scene&& other)
{
    if (this != &other) {
        _move_from(std::move(other));
    }
}

Scene& Scene::operator=(Scene&& other)
{
    if (this != &other) {
        _move_from(std::move(other));
    }
    return *this;
}

void Scene::clone(const Scene& other)
{
    memcpy(name.data(), other.name.data(), name.size());
    memcpy(description.data(), other.description.data(), description.size());
    memcpy(author.data(), other.author.data(), author.size());
    version = other.version;
    for (const Scene& dep : other.dependencies()) {
        Scene s {};
        s.clone(dep);
        add_dependency(std::move(s));
    }

    frame_s           = other.frame_s;
    _gates            = other._gates;
    _components       = other._components;
    _inputs           = other._inputs;
    _outputs          = other._outputs;
    _relations        = other._relations;
    component_context = other.component_context;
    for (size_t i = 0; i < Node::Type::NODE_S; i++) {
        _last_node[i] = other._last_node[i];
    }
    _last_rel = other._last_rel;
    for (auto& gate : _gates) {
        gate.reload(this);
    }
    for (auto& comp : _components) {
        comp.reload(this);
    }
    for (auto& input : _inputs) {
        input.reload(this);
    }
    for (auto& output : _outputs) {
        output.reload(this);
    }
    if (component_context.has_value()) {
        component_context->reload(this);
    }
}

void Scene::_move_from(Scene&& other)
{
    name              = std::move(other.name);
    description       = std::move(other.description);
    author            = std::move(other.author);
    version           = other.version;
    _dependencies     = std::move(other._dependencies);
    frame_s           = other.frame_s;
    _gates            = std::move(other._gates);
    _components       = std::move(other._components);
    _inputs           = std::move(other._inputs);
    _outputs          = std::move(other._outputs);
    _relations        = std::move(other._relations);
    component_context = std::move(other.component_context);
    for (size_t i = 0; i < Node::Type::NODE_S; i++) {
        _last_node[i] = other._last_node[i];
    }
    _last_rel = other._last_rel;

    for (auto& gate : _gates) {
        gate.reload(this);
    }
    for (auto& comp : _components) {
        comp.reload(this);
    }
    for (auto& input : _inputs) {
        input.reload(this);
    }
    for (auto& output : _outputs) {
        output.reload(this);
    }
    for (auto& dep : _dependencies) {
        dep._parent = this;
    }
    if (component_context.has_value()) {
        component_context->reload(this);
    }
}

void Scene::remove_node(Node id)
{
    BaseNode* node = nullptr;
    switch (id.type) {
    case Node::Type::GATE: {
        lcs_assert(id.index < _gates.size());
        node = &_gates[id.index];
        break;
    }
    case Node::Type::COMPONENT: {
        lcs_assert(id.index < _components.size());
        node = &_components[id.index];
        break;
    }
    case Node::Type::INPUT: {
        lcs_assert(id.index < _inputs.size());
        node = &_inputs[id.index];
        break;
    }
    case Node::Type::OUTPUT: {
        lcs_assert(id.index < _outputs.size());
        node = &_outputs[id.index];
        break;
    }
    default: break;
    }
    lcs_assert(node != nullptr);
    node->clean();
    node->set_null();
    if (_last_node[id.type].index >= id.index) {
        _last_node[id.type].index = id.index;
    }
    L_DEBUG("%s was invalidated. Last node for %s was updated to %d",
        to_str<Node>(id), to_str<Node::Type>(id.type),
        _last_node[id.type].index);
}

NRef<Rel> Scene::get_rel(relid idx)
{
    if (idx == 0 || idx > _last_rel) {
        return nullptr;
    }
    auto n = _relations.find(idx);
    return n != _relations.end() ? &n->second
                                 : (S_ERROR("Rel not found", nullptr));
}

Error Scene::connect_with_id(
    relid id, Node to_node, sockid to_sock, Node from_node, sockid from_sock)
{
    if (from_node.type == Node::Type::OUTPUT
        || from_node.type == Node::Type::COMPONENT_OUTPUT) {
        return ERROR(Error::INVALID_FROM_TYPE);
    } else if (from_node.type == Node::Type::COMPONENT
        && from_sock >= dependencies()[get_node<Component>(from_node)->dep_idx]
                .component_context->outputs.size()) {
        return ERROR(Error::INVALID_NODEID);
    }
    if (!component_context.has_value()
        && (to_node.type == Node::Type::COMPONENT_OUTPUT
            || from_node.type == Node::Type::COMPONENT_INPUT)) {
        return ERROR(Error::NOT_A_COMPONENT);
    }

    bool is_connected = false;
    switch (to_node.type) {
    case Node::Type::GATE: {
        if (auto gate = get_node<Gate>(to_node);
            gate != nullptr && gate->inputs[to_sock] == 0) {
            gate->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case Node::Type::COMPONENT: {
        if (auto comp = get_node<Component>(to_node);
            comp != nullptr && comp->inputs[to_sock] == 0) {
            comp->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case Node::Type::OUTPUT: {
        if (auto out = get_node<Output>(to_node);
            out != nullptr && out->input == 0) {
            out->input   = id;
            is_connected = true;
        }
        break;
    }
    case Node::Type::COMPONENT_OUTPUT: {
        if (component_context->outputs.size() > to_node.index - 1) {
            relid& to_id = component_context->outputs[to_node.index - 1];
            if (to_id != 0) {
                return ERROR(Error::ALREADY_CONNECTED);
            }
            to_id        = id;
            is_connected = true;
        }
        break;
    }
    default: return ERROR(Error::INVALID_TO_TYPE);
    }
    if (!is_connected) {
        return ERROR(Error::ALREADY_CONNECTED);
    }
    _relations.emplace(id, Rel { id, from_node, to_node, from_sock, to_sock });

    switch (from_node.type) {
    case Node::Type::GATE:
        get_node<Gate>(from_node)->output.push_back(id);
        get_node<Gate>(from_node)->on_signal();
        break;
    case Node::Type::COMPONENT:
        get_node<Component>(from_node)->outputs[from_sock].push_back(id);
        get_node<Component>(from_node)->on_signal();
        break;
    case Node::Type::INPUT:
        get_node<Input>(from_node)->output.push_back(id);
        get_node<Input>(from_node)->on_signal();
        break;
    case Node::Type::COMPONENT_INPUT: /* Component input is not handled
                                     here. */
        lcs_assert(component_context.has_value());
        if (component_context->inputs.size() > from_node.index - 1) {
            std::vector<relid>& from_list
                = component_context->inputs[from_node.index - 1];
            for (auto _id : from_list) {
                if (_id == id) {
                    return ERROR(Error::ALREADY_CONNECTED);
                }
            }
            from_list.push_back(id);
        }
        component_context->run(0, 0);
        break;
    default: return ERROR(Error::INVALID_TO_TYPE);
    }
    L_DEBUG("Connect completed.");
    return OK;
}

relid Scene::connect(
    Node to_node, sockid to_sock, Node from_node, sockid from_sock)
{
    _last_rel++;
    relid id = _last_rel;
    return connect_with_id(id, to_node, to_sock, from_node, from_sock) ? 0 : id;
}

Error Scene::disconnect(relid id)
{
    if (id == 0 || id > _last_rel) {
        return ERROR(Error::INVALID_RELID);
    }
    auto remove_fn = [id](relid i) { return i == id; };
    auto r         = _relations.find(id);
    if (r == _relations.end()) {
        return ERROR(Error::REL_NOT_FOUND);
    }

    switch (r->second.from_node.type) {
    case Node::Type::GATE: {
        auto& v = get_node<Gate>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case Node::Type::COMPONENT: {
        auto& v = get_node<Component>(r->second.from_node)
                      ->outputs[r->second.from_sock];
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case Node::Type::INPUT: {
        auto& v = get_node<Input>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case Node::Type::COMPONENT_INPUT: {
        lcs_assert(component_context.has_value());
        if (component_context->inputs.size() > r->second.from_node.index - 1) {
            auto& v = component_context->inputs[r->second.from_node.index - 1];
            v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        } else {
            return ERROR(Error::NOT_CONNECTED);
        }
        break;
    }
    default: lcs_assert(r->second.from_node.type == Node::Type::OUTPUT); break;
    }

    switch (r->second.to_node.type) {
    case Node::Type::GATE: {
        auto g = get_node<Gate>(r->second.to_node);

        g->inputs[r->second.to_sock] = 0;
        g->on_signal();
        break;
    }
    case Node::Type::COMPONENT: {
        auto c = get_node<Component>(r->second.to_node);

        c->inputs[r->second.to_sock] = 0;
        c->on_signal();
        break;
    }
    case Node::Type::OUTPUT: {
        auto o   = get_node<Output>(r->second.to_node);
        o->input = 0;
        o->on_signal();
        break;
    }
    case Node::Type::COMPONENT_OUTPUT: {
        lcs_assert(component_context.has_value());
        if (component_context->outputs.size() > r->second.to_node.index - 1) {
            component_context->outputs[r->second.to_node.index - 1] = 0;
            component_context->run();
        } else {
            return ERROR(Error::NOT_CONNECTED);
        }
        break;
    }
    default: lcs_assert(r->second.from_node.type == Node::Type::INPUT); break;
    }
    _relations.erase(id);
    L_DEBUG("Disconnect completed.");
    return OK;
}

void Scene::signal(relid id, State value)
{
    lcs_assert(id != 0);
    auto r = get_rel(id);
    lcs_assert(r != nullptr);
    if (r->value != value || r->value == DISABLED) {
        r->value = value;
        L_INFO("%s:rel@%-2d %s:%d sent %s to %s:%d",
            _parent != nullptr ? name.begin() : "root", id,
            to_str<Node>(r->from_node), r->from_sock, to_str<State>(r->value),
            to_str<Node>(r->to_node), r->to_sock);
        if (r->to_node.type != Node::Type::COMPONENT_OUTPUT) {
            auto n = get_base(r->to_node);
            lcs_assert(n != nullptr);
            n->on_signal();
        } else {
            component_context->set_value(r->to_node.index, r->value);
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

NRef<BaseNode> Scene::get_base(Node id)
{
    switch (id.type) {
    case Node::Type::GATE: return get_node<Gate>(id)->base();
    case Node::Type::COMPONENT: return get_node<Component>(id)->base();
    case Node::Type::INPUT: return get_node<Input>(id)->base();
    case Node::Type::OUTPUT: return get_node<Output>(id)->base();
    default: break;
    }
    return nullptr;
}

std::string Scene::to_dependency() const
{
    std::stringstream dep_str {};
    std::string_view str_author { author.begin() };
    if (str_author.empty()) {
        dep_str << "local/";
    } else {
        dep_str << str_author << '/';
    }
    dep_str << std::string_view { name.begin() } << '/'
            << std::to_string(version);
    return dep_str.str();
}

void Scene::run(float delta)
{
    uint32_t frame_pre = frame_s * 10;
    frame_s += delta;
    uint32_t frame = frame_s * 10;
    if (frame != frame_pre) {
//        L_INFO("delta: %f frame:%d frame_pre:%d", delta, frame, frame_pre);
        for (auto& in : _inputs) {
            if (!in.is_null() && in.is_timer()) {
                if (frame % in._freq == 0) {
                    in.set(frame / 10 % 2 == 0);
                }
            }
        }
    }
}

} // namespace lcs
