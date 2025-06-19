#include "common.h"
#include "core.h"
#include "io.h"
#include <json/json.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <optional>

namespace lcs {
/** Reads a JSON document and writes its values to a map */
template <typename T>
static Error _json_to_map(
    Scene* s, const Json::Value& doc, std::map<Node, T>& m, Node& last_node);

static NodeType _str_to_node(const std::string&);
GateType _str_to_gate(const std::string&);

Error Scene::from_json(const Json::Value& doc)
{
    static constexpr const char* _component
        = NodeType_to_str(NodeType::COMPONENT);
    static constexpr const char* _gate   = NodeType_to_str(NodeType::GATE);
    static constexpr const char* _input  = NodeType_to_str(NodeType::INPUT);
    static constexpr const char* _output = NodeType_to_str(NodeType::OUTPUT);
    if (!(doc.isObject() && doc["nodes"].isObject() && doc["name"].isString()
            && doc["author"].isString() && doc["version"].isInt())) {
        return ERROR(Error::INVALID_SCENE);
    }
    std::string buffer = doc["name"].asString();
    if (buffer.size() >= name.max_size()) {
        return ERROR(Error::INVALID_SCENE_NAME);
    }
    std::copy(buffer.begin(), buffer.end(), name.begin());
    buffer = doc["author"].asString();
    if (buffer.size() >= author.max_size()) {
        return ERROR(Error::INVALID_AUTHOR_NAME);
    }
    std::copy(buffer.begin(), buffer.end(), author.begin());
    if (doc["description"].isString()) {
        buffer = doc["description"].asString();
        if (buffer.size() >= description.max_size()) {
            return ERROR(Error::INVALID_DESCRIPTION);
        }
        std::copy(buffer.begin(), buffer.end(), description.begin());
    }
    version = doc["version"].asInt();
    if (doc["dependencies"].isArray()) {
        return Error::OK;
        for (const auto& j : doc["dependencies"]) {
            Error err = io::component::fetch(j.asString());
            if (err) {
                return err;
            }
            dependencies.push_back(j.asString());
        }
    }

    Error err = Error::OK;
    if (doc["component"].isObject()) {
        component_context = ComponentContext { this };
        err               = component_context->from_json(doc["component"]);
        if (err) {
            return err;
        }
    }
    const Json::Value& nodes = doc["nodes"];
    if (nodes[_gate].isObject()) {
        err = _json_to_map<GateNode>(
            this, nodes[_gate], _gates, _last_node[NodeType::GATE]);
        if (err) {
            return err;
        }
    }
    if (nodes[_component].isObject()) {
        err = _json_to_map<ComponentNode>(this, nodes[_component], _components,
            _last_node[NodeType::COMPONENT]);
        if (err) {
            return err;
        }
    }
    if (nodes[_input].isObject()) {
        err = _json_to_map<InputNode>(
            this, nodes[_input], _inputs, _last_node[NodeType::INPUT]);
        if (err) {
            return err;
        }
    }
    if (nodes[_output].isObject()) {
        err = _json_to_map<OutputNode>(
            this, nodes[_output], _outputs, _last_node[NodeType::OUTPUT]);
        if (err) {
            return err;
        }
    }
    if (doc["rel"].isObject()) {
        for (Json::Value::const_iterator iter = doc["rel"].begin();
            iter != doc["rel"].end(); iter++) {
            relid id = std::atol(iter.key().asCString());
            Rel r;
            r.id      = id;
            Error err = r.from_json(*iter);
            if (err) {
                return err;
            }
            if (id > _last_rel) {
                _last_rel = id;
            }
            if (_connect_with_id(
                    r.id, r.to_node, r.to_sock, r.from_node, r.from_sock)) {
                return ERROR(Error::REL_CONNECT_ERROR);
            }
        }
        if (err) {
            return err;
        }
    }

    for (auto& comp : _components) {
        bool found = false;
        for (const auto& dep : dependencies) {
            if (comp.second.path == dep) {
                found = true;
                break;
            }
        }
        if (!found) {
            return ERROR(Error::UNDEFINED_DEPENDENCY);
        }
    }
    return Error::OK;
}

Error Point::from_json(const Json::Value& doc)
{
    x = doc["x"].isInt() ? doc["x"].asInt() : 0;
    y = doc["y"].isInt() ? doc["y"].asInt() : 0;
    return Error::OK;
}

Error Node::from_json(const Json::Value& doc)
{
    if (doc["id"].isString()) {
        std::string idstr = doc["id"].asString();
        size_t at_idx     = idstr.find("@");
        if (at_idx == std::string::npos) {
            return ERROR(Error::INVALID_NODE);
        }
        type = _str_to_node(std::string(idstr.begin(), idstr.begin() + at_idx));
        id   = std::atol(
            std::string { idstr.begin() + at_idx + 1, idstr.end() }.c_str());
    } else if (doc["id"].isInt()) {
        id = doc["id"].asInt();
    } else {
        return ERROR(Error::INVALID_NODE);
    }
    if (type >= NodeType::NODE_S) {
        return ERROR(Error::INVALID_NODE);
    }
    return Error::OK;
}

Error Rel::from_json(const Json::Value& doc)
{
    if (!(doc["from"].isObject() && doc["to"].isObject())) {
        return Error::INVALID_NODE;
    }
    Error err = from_node.from_json(doc["from"]);
    if (err) {
        return err;
    }

    err = to_node.from_json(doc["to"]);
    if (err) {
        return err;
    }

    from_sock = doc["from"].get("sock", 0).asInt();
    to_sock   = doc["to"].get("sock", 0).asInt();

    if (doc["curve"].isArray()) {
        for (const auto& j : doc["curve"]) {
            Point p;
            err = p.from_json(j);
            if (err) {
                return err;
            }
            curve_points.push_back(p);
        }
    }
    return Error::OK;
}

Error GateNode::from_json(const Json::Value& doc)
{
    if (doc["size"].isInt()) {
        int size  = doc["size"].asInt();
        bool pass = true;
        while (_max_in < size && pass) {
            pass = increment();
        }
    }

    return Error::OK;
}

Error ComponentNode::from_json(const Json::Value& doc)
{
    if (!doc["use"].isString()) {
        return ERROR(Error::INVALID_COMPONENT);
    }
    set_component(doc["use"].asString());
    return Error::OK;
}

Error InputNode::from_json(const Json::Value& doc)
{
    if (doc["freq"].isNumeric()) {
        _freq = std::max(0.25f,
            std::min(std::round(doc["freq"].asFloat() * 4.0f) / 4.0f, 10.0f));
        if (_freq <= 0) {
            _freq = std::nullopt;
        }
    } else if (doc["data"].isBool()) {
        set(doc["data"].asBool());
    } else {
        return ERROR(Error::INVALID_INPUT);
    }
    return Error::OK;
}

Error OutputNode::from_json(const Json::Value&) { return Error::OK; }

Error ComponentContext::from_json(const Json::Value& doc)
{
    if (!(doc["in"].isInt() && doc["out"].isInt())) {
        return ERROR(Error::INVALID_COMPONENT);
    }
    setup(doc["in"].asInt(), doc["out"].asInt());
    return Error::OK;
}

template <typename T>
Error _json_to_map(
    Scene* s, const Json::Value& doc, std::map<Node, T>& m, Node& last_node)
{
    Node last_id = 0;
    Error err    = Error::OK;

    for (Json::Value::const_iterator iter = doc.begin(); iter != doc.end();
        iter++) {
        const Json::Value& value = *iter;
        Node id { static_cast<uint16_t>(std::atoi(iter.key().asCString())),
            as_node_type<T>() };
        if (id.id > last_id.id) {
            last_id = id;
        }
        Point point { 0, 0 };
        if (value["pos"].isObject()) {
            err = point.from_json(value["pos"]);
        }
        if (err) {
            return err;
        }
        if constexpr (std::is_same<T, GateNode>::value) {
            if (!value["type"].isString()) {
                return ERROR(Error::INVALID_GATE);
            }
            T t { s, id, _str_to_gate(value["type"].asString()) };
            err = t.from_json(value);
            if (err) {
                return err;
            }
            t.point = point;
            m.emplace(id, t);
        } else {
            T t { s, id };
            err = t.from_json(value);
            if (err) {
                return err;
            }
            t.point = point;
            m.emplace(id, t);
        }

        if constexpr (std::is_same<T, InputNode>::value) {
            if (auto t = m.find(id); t != m.end() && t->second.is_timer()) {
                s->_timerlist.emplace(id, 0);
            }
        }
    }
    last_node = last_id;
    return Error::OK;
}

static NodeType _str_to_node(const std::string& n)
{

    if (n == "Gate") {
        return NodeType::GATE;
    } else if (n == "Comp") {
        return NodeType::COMPONENT;
    } else if (n == "In") {
        return NodeType::INPUT;
    } else if (n == "Out") {
        return NodeType::OUTPUT;
    } else if (n == "Cin") {
        return NodeType::COMPONENT_INPUT;
    } else if (n == "Cout") {
        return NodeType::COMPONENT_OUTPUT;
    }
    return NodeType::NODE_S;
}

GateType _str_to_gate(const std::string& type)
{
    if (type == "NOT")
        return NOT;
    else if (type == "AND")
        return AND;
    else if (type == "OR")
        return OR;
    else if (type == "XOR")
        return XOR;
    else if (type == "NAND")
        return NAND;
    else if (type == "NOR")
        return NOR;
    else
        return XNOR;
}

namespace parse {
    Error load_scene(const std::string& str, Scene& scene)
    {
        Json::Reader reader {};
        Json::Value root;
        bool ok = reader.parse(str, root);
        if (!ok) {
            return ERROR(Error::INVALID_JSON_FORMAT);
        }
        return scene.from_json(root);
    }

} // namespace parse
} // namespace lcs
