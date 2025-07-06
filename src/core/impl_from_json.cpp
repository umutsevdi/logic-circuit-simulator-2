#include "core.h"
#include "io.h"
#include <json/json.h>
#include <cmath>

namespace lcs {
/** Reads a JSON document and writes its values to a map */
template <typename T>
Error static _json_to_map(
    Scene* s, const Json::Value& doc, std::map<Node, T>& m, Node& last_node);

static NodeType _str_to_node(const std::string&);
GateType _str_to_gate(const std::string&);

template <> LCS_ERROR from_json<Point>(const Json::Value& doc, Point& point)
{
    point.x = doc["x"].isInt() ? doc["x"].asInt() : 0;
    point.y = doc["y"].isInt() ? doc["y"].asInt() : 0;
    return OK;
}
template <> LCS_ERROR from_json<Node>(const Json::Value& doc, Node& node)
{
    if (doc["id"].isString()) {
        std::string idstr = doc["id"].asString();
        size_t at_idx     = idstr.find("@");
        if (at_idx == std::string::npos) {
            return ERROR(Error::INVALID_NODE);
        }
        node.type
            = _str_to_node(std::string(idstr.begin(), idstr.begin() + at_idx));
        node.id = std::atol(
            std::string { idstr.begin() + at_idx + 1, idstr.end() }.c_str());
    } else if (doc["id"].isInt()) {
        node.id = doc["id"].asInt();
    } else {
        return ERROR(Error::INVALID_NODE);
    }
    if (node.type >= NodeType::NODE_S) {
        return ERROR(Error::INVALID_NODE);
    }
    return OK;
}
template <> LCS_ERROR from_json<Rel>(const Json::Value& doc, Rel& rel)
{
    if (!(doc["from"].isObject() && doc["to"].isObject())) {
        return INVALID_NODE;
    }
    Error err = from_json<Node>(doc["from"], rel.from_node);
    if (err) {
        return err;
    }

    err = from_json(doc["to"], rel.to_node);
    if (err) {
        return err;
    }

    rel.from_sock = doc["from"].get("sock", 0).asInt();
    rel.to_sock   = doc["to"].get("sock", 0).asInt();
    return OK;
}
template <>
LCS_ERROR from_json<GateNode>(const Json::Value& doc, GateNode& node)
{
    if (doc["size"].isInt()) {
        size_t size = doc["size"].asInt();
        bool pass   = true;
        while (node.inputs.size() < size && pass) {
            pass = node.increment();
        }
    }

    return OK;
}
template <>
LCS_ERROR from_json<ComponentNode>(const Json::Value& doc, ComponentNode& node)
{
    if (!doc["use"].isString()) {
        return ERROR(Error::INVALID_COMPONENT);
    }
    return node.set_component(doc["use"].asString());
}
template <>
LCS_ERROR from_json<InputNode>(const Json::Value& doc, InputNode& node)
{
    if (doc["freq"].isNumeric()) {
        node._freq = std::max(0.1f,
            std::min(std::round(doc["freq"].asFloat() * 10) / 10.0f, 5.0f));
        if (node._freq <= 0) {
            node._freq = std::nullopt;
        }
    } else if (doc["data"].isBool()) {
        node.set(doc["data"].asBool());
    } else {
        return ERROR(Error::INVALID_INPUT);
    }
    return OK;
}
template <> LCS_ERROR from_json<OutputNode>(const Json::Value&, OutputNode&)
{
    return OK;
}
template <>
LCS_ERROR from_json<ComponentContext>(
    const Json::Value& doc, ComponentContext& ctx)
{
    if (!(doc["in"].isInt() && doc["out"].isInt())) {
        return ERROR(Error::INVALID_COMPONENT);
    }
    ctx.setup(doc["in"].asInt(), doc["out"].asInt());
    return OK;
}

template <> LCS_ERROR from_json<Scene>(const Json::Value& doc, Scene& scene)
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
    if (buffer.size() >= scene.name.max_size()) {
        return ERROR(Error::INVALID_SCENE_NAME);
    }
    std::copy(buffer.begin(), buffer.end(), scene.name.begin());
    buffer = doc["author"].asString();
    if (buffer.size() >= scene.author.max_size()) {
        return ERROR(Error::INVALID_AUTHOR_NAME);
    }
    std::copy(buffer.begin(), buffer.end(), scene.author.begin());
    if (doc["description"].isString()) {
        buffer = doc["description"].asString();
        if (buffer.size() >= scene.description.max_size()) {
            return ERROR(Error::INVALID_DESCRIPTION);
        }
        std::copy(buffer.begin(), buffer.end(), scene.description.begin());
    }
    scene.version = doc["version"].asInt();
    if (doc["dependencies"].isArray()) {
        return OK;
        for (const auto& j : doc["dependencies"]) {
            Error err = io::component::fetch(j.asString());
            if (err) {
                return err;
            }
            scene.dependencies.push_back(j.asString());
        }
    }

    Error err = OK;
    if (doc["component"].isObject()) {
        scene.component_context = ComponentContext { &scene };
        err                     = from_json<ComponentContext>(
            doc["component"], scene.component_context.value());
        if (err) {
            return err;
        }
    }
    const Json::Value& nodes = doc["nodes"];
    if (nodes[_gate].isObject()) {
        err = _json_to_map<GateNode>(&scene, nodes[_gate], scene._gates,
            scene._last_node[NodeType::GATE]);
        if (err) {
            return err;
        }
    }
    if (nodes[_component].isObject()) {
        err = _json_to_map<ComponentNode>(&scene, nodes[_component],
            scene._components, scene._last_node[NodeType::COMPONENT]);
        if (err) {
            return err;
        }
    }
    if (nodes[_input].isObject()) {
        err = _json_to_map<InputNode>(&scene, nodes[_input], scene._inputs,
            scene._last_node[NodeType::INPUT]);
        if (err) {
            return err;
        }
    }
    if (nodes[_output].isObject()) {
        err = _json_to_map<OutputNode>(&scene, nodes[_output], scene._outputs,
            scene._last_node[NodeType::OUTPUT]);
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
            Error err = from_json<Rel>(*iter, r);
            if (err) {
                return err;
            }
            if (id > scene._last_rel) {
                scene._last_rel = id;
            }
            if (scene._connect_with_id(
                    r.id, r.to_node, r.to_sock, r.from_node, r.from_sock)) {
                return ERROR(Error::REL_CONNECT_ERROR);
            }
        }
        if (err) {
            return err;
        }
    }

    for (auto& comp : scene._components) {
        bool found = false;
        for (const auto& dep : scene.dependencies) {
            if (comp.second.path == dep) {
                found = true;
                break;
            }
        }
        if (!found) {
            return ERROR(Error::UNDEFINED_DEPENDENCY);
        }
    }
    return OK;
}

template <typename T>
Error _json_to_map(
    Scene* s, const Json::Value& doc, std::map<Node, T>& m, Node& last_node)
{
    Node last_id = 0;
    Error err    = OK;

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
            err = from_json<Point>(value["pos"], point);
        }
        if (err) {
            return err;
        }
        if constexpr (std::is_same<T, GateNode>::value) {
            if (!value["type"].isString()) {
                return ERROR(Error::INVALID_GATE);
            }
            T t { s, id, _str_to_gate(value["type"].asString()) };
            err = from_json<T>(value, t);
            if (err) {
                return err;
            }
            t.point = point;
            m.emplace(id, t);
        } else {
            T t { s, id };
            err = from_json<T>(value, t);
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
    return OK;
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
    if (type == "NOT") {
        return NOT;
    } else if (type == "AND") {
        return AND;
    } else if (type == "OR") {
        return OR;
    } else if (type == "XOR") {
        return XOR;
    } else if (type == "NAND") {
        return NAND;
    } else if (type == "NOR") {
        return NOR;
    } else {
        return XNOR;
    }
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
        return from_json<Scene>(root, scene);
    }

} // namespace parse
} // namespace lcs
  //
