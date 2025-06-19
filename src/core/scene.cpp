#include "common.h"
#include "core.h"
#include <base64.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <utility>

namespace lcs {

Scene::Scene(const std::string& _name, const std::string& _author,
        const std::string& _description, int _version) :
        version { _version }, component_context { std::nullopt }, _last_node {
            Node { 0, NodeType::GATE },
            Node { 0, NodeType::COMPONENT },
            Node { 0, NodeType::INPUT },
            Node { 0, NodeType::OUTPUT },
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
        version { _version }, component_context { ctx }, _last_node {
            Node { 0, NodeType::GATE },
            Node { 0, NodeType::COMPONENT },
            Node { 0, NodeType::INPUT },
            Node { 0, NodeType::OUTPUT },
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

void Scene::_move_from(Scene&& other)
{
    name              = std::move(other.name);
    description       = std::move(other.description);
    author            = std::move(other.author);
    version           = other.version;
    dependencies      = std::move(other.dependencies);
    _timerlist        = std::move(other._timerlist);
    _gates            = std::move(other._gates);
    _components       = std::move(other._components);
    _inputs           = std::move(other._inputs);
    _outputs          = std::move(other._outputs);
    _relations        = std::move(other._relations);
    component_context = std::move(other.component_context);
    for (size_t i = 0; i < NodeType::NODE_S; i++) {
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
    if (component_context.has_value()) {
        component_context->reload(this);
    }
}

void Scene::remove_node(Node id)
{
    if (id.id == 0) {
        lcs_assert(id.id != 0);
    }
    switch (id.type) {
    case NodeType::GATE: {
        lcs_assert(id.id <= _last_node[NodeType::GATE].id);
        auto g = _gates.find(id);
        lcs_assert(g != _gates.end());
        for (auto r : g->second.output) {
            if (r != 0) {
                disconnect(r);
            }
        }
        for (auto r : g->second.inputs) {
            if (r != 0) {
                disconnect(r);
            }
        }
        _gates.erase(_gates.find(id));
        break;
    }
    case NodeType::COMPONENT: {
        lcs_assert(id.id <= _last_node[NodeType::COMPONENT].id);
        auto c = _components.find(id);
        lcs_assert(c != _components.end());
        for (auto s : c->second.outputs) {
            for (auto r : s.second) {
                if (r != 0) {
                    disconnect(r);
                }
            }
        }
        for (auto r : c->second.inputs) {
            if (r != 0) {
                disconnect(r);
            }
        }
        _components.erase(_components.find(id));
        break;
    }
    case NodeType::INPUT: {
        lcs_assert(id.id <= _last_node[NodeType::INPUT].id);
        auto i = _inputs.find(id);
        lcs_assert(i != _inputs.end());
        if (i->second.is_timer()) {
            _timerlist.erase(i->first);
        }
        for (auto r : i->second.output) {
            if (r != 0) {
                disconnect(r);
            }
        }
        _inputs.erase(_inputs.find(id));
        break;
    }
    case NodeType::OUTPUT: {
        lcs_assert(id.id <= _last_node[NodeType::OUTPUT].id);
        auto o = _outputs.find(id);
        lcs_assert(o != _outputs.end());
        if (o->second.input != 0) {
            disconnect(o->second.input);
        }
        _outputs.erase(_outputs.find(id));
        break;
    }
    default: break;
    }
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

Error Scene::_connect_with_id(
    relid id, Node to_node, sockid to_sock, Node from_node, sockid from_sock)
{
    if (from_node.type == NodeType::OUTPUT
        || from_node.type == NodeType::COMPONENT_OUTPUT) {
        return ERROR(Error::INVALID_FROM_TYPE);
    } else if (from_node.type == NodeType::COMPONENT
        && from_sock
            >= io::component::get(get_node<ComponentNode>(from_node)->path)
                ->component_context->outputs.size()) {
        return ERROR(Error::INVALID_NODEID);
    }
    if (!component_context.has_value()
        && (to_node.type == NodeType::COMPONENT_OUTPUT
            || from_node.type == NodeType::COMPONENT_INPUT)) {
        return ERROR(Error::NOT_A_COMPONENT);
    }

    bool is_connected = false;
    switch (to_node.type) {
    case NodeType::GATE: {
        if (auto gate = get_node<GateNode>(to_node);
            gate != nullptr && gate->inputs[to_sock] == 0) {
            gate->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case NodeType::COMPONENT: {
        if (auto comp = get_node<ComponentNode>(to_node);
            comp != nullptr && comp->inputs[to_sock] == 0) {
            comp->inputs[to_sock] = id;
            is_connected          = true;
        }
        break;
    }
    case NodeType::OUTPUT: {
        if (auto out = get_node<OutputNode>(to_node);
            out != nullptr && out->input == 0) {
            out->input   = id;
            is_connected = true;
        }
        break;
    }
    case NodeType::COMPONENT_OUTPUT: {
        if (component_context->outputs.size() > to_node.id - 1) {
            relid& to_id = component_context->outputs[to_node.id - 1];
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
    case NodeType::GATE:
        get_node<GateNode>(from_node)->output.push_back(id);
        get_node<GateNode>(from_node)->on_signal();
        break;
    case NodeType::COMPONENT:
        get_node<ComponentNode>(from_node)->outputs[from_sock].push_back(id);
        get_node<ComponentNode>(from_node)->on_signal();
        break;
    case NodeType::INPUT:
        get_node<InputNode>(from_node)->output.push_back(id);
        get_node<InputNode>(from_node)->on_signal();
        break;
    case NodeType::COMPONENT_INPUT: /* Component input is not handled
                                     here. */
        lcs_assert(component_context.has_value());
        if (component_context->inputs.size() > from_node.id - 1) {
            std::vector<relid>& from_list
                = component_context->inputs[from_node.id - 1];
            for (auto _id : from_list) {
                if (_id == id) {
                    return ERROR(Error::ALREADY_CONNECTED);
                }
            }
            from_list.push_back(id);
        }
        component_context->run(0);
        break;
    default: return ERROR(Error::INVALID_TO_TYPE);
    }
    return Error::OK;
}

relid Scene::connect(
    Node to_node, sockid to_sock, Node from_node, sockid from_sock)
{
    _last_rel++;
    relid id = _last_rel;
    return _connect_with_id(id, to_node, to_sock, from_node, from_sock) ? 0
                                                                        : id;
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
    case NodeType::GATE: {
        auto& v = get_node<GateNode>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case NodeType::COMPONENT: {
        auto& v = get_node<ComponentNode>(r->second.from_node)
                      ->outputs[r->second.from_sock];
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case NodeType::INPUT: {
        auto& v = get_node<InputNode>(r->second.from_node)->output;
        v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        break;
    }
    case NodeType::COMPONENT_INPUT: {
        lcs_assert(component_context.has_value());
        if (component_context->inputs.size() > r->second.from_node.id - 1) {
            auto& v = component_context->inputs[r->second.from_node.id - 1];
            v.erase(std::remove_if(v.begin(), v.end(), remove_fn));
        } else {
            return ERROR(Error::NOT_CONNECTED);
        }
        break;
    }
    default: lcs_assert(r->second.from_node.type == NodeType::OUTPUT); break;
    }

    switch (r->second.to_node.type) {
    case NodeType::GATE: {
        auto g = get_node<GateNode>(r->second.to_node);

        g->inputs[r->second.to_sock] = 0;
        g->on_signal();
        break;
    }
    case NodeType::COMPONENT: {
        auto c = get_node<ComponentNode>(r->second.to_node);

        c->inputs[r->second.to_sock] = 0;
        c->on_signal();
        break;
    }
    case NodeType::OUTPUT: {
        auto o   = get_node<OutputNode>(r->second.to_node);
        o->input = 0;
        o->on_signal();
        break;
    }
    case NodeType::COMPONENT_OUTPUT: {
        lcs_assert(component_context.has_value());
        if (component_context->outputs.size() > r->second.to_node.id - 1) {
            component_context->outputs[r->second.to_node.id - 1] = 0;
            component_context->run();
        } else {
            return ERROR(Error::NOT_CONNECTED);
        }
        break;
    }
    default: lcs_assert(r->second.from_node.type == NodeType::INPUT); break;
    }
    _relations.erase(id);
    return Error::OK;
}

void Scene::signal(relid id, State value)
{
    if (id == 0) {
        return;
    }
    if (auto r = _relations.find(id); r != _relations.end()) {
        r->second.value = value;
        L_INFO(r->second);
        if (r->second.to_node.type != NodeType::COMPONENT_OUTPUT) {
            auto n = get_base(r->second.to_node);
            if (n != nullptr) {
                n->on_signal();
            }
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

NRef<BaseNode> Scene::get_base(Node id)
{
    switch (id.type) {
    case NodeType::GATE: return get_node<GateNode>(id)->base();
    case NodeType::COMPONENT: return get_node<ComponentNode>(id)->base();
    case NodeType::INPUT: return get_node<InputNode>(id)->base();
    case NodeType::OUTPUT: return get_node<OutputNode>(id)->base();
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

Error Scene::load_dependencies(void)
{
    for (const std::string& dep : dependencies) {
        Error err = io::component::fetch(dep);
        if (err) {
            return err;
        }
    }
    return Error::OK;
}

std::string Scene::to_filepath(void) const
{
    std::filesystem::path p;
    std::string_view str_author { author.begin() };
    std::string file_name { name.begin() };
    file_name = base64_encode(file_name + "/" + std::to_string(version), true)
        + ".json";
    if (str_author.empty() || str_author == "local") {
        p = io::LOCAL / file_name;
    } else {
        p = io::LIBRARY / str_author / file_name;
    }
    return p;
}

void Scene::run_timers(void)
{
    for (auto& timer : _timerlist) {
        NRef<InputNode> node = get_node<InputNode>(timer.first);
        timer.second += node->_freq.value_or(0) / 60;
        if (timer.second > 1) {
            timer.second--;
            node->toggle();
        }
    }
}

} // namespace lcs
