#include "core.h"
#include "parse.h"
#include <json/json.h>
#include <map>

namespace lcs {
/** Reads a JSON document and writes its values to a map */
template <typename T>
static error_t _json_to_map(
    Scene* s, const Json::Value& doc, std::map<node, T>& m);
/** Connects existing nodes to each other according to the document */
static error_t _connect_all(Scene* s, const Json::Value& rel);

error_t Scene::from_json(const Json::Value& doc)
{
    if (!doc["meta"].isObject() || !doc["nodes"].isObject()) {
        return ERROR(error_t::INVALID_SCENE);
    }
    error_t err = meta.from_json(doc["meta"]);
    if (err) { return err; }

    if (doc["component"].isObject()) {
        component_context = ComponentContext {};
        err = component_context->from_json(doc["component"]);
        if (err) { return err; }
    }
    const Json::Value& nodes = doc["nodes"];
    if (nodes["gates"].isObject()) {
        err = _json_to_map<GateNode>(this, nodes["gates"], gates);
        if (err) { return err; }
    }
    if (nodes["comp"].isObject()) {
        err = _json_to_map<ComponentNode>(this, nodes["comp"], components);
        if (err) { return err; }
    }
    if (nodes["inputs"].isObject()) {
        err = _json_to_map<InputNode>(this, nodes["inputs"], inputs);
        if (err) { return err; }
    }
    if (nodes["outputs"].isObject()) {
        err = _json_to_map<OutputNode>(this, nodes["outputs"], outputs);
        if (err) { return err; }
    }
    if (doc["rel"].isObject()) {
        err = _connect_all(this, doc["rel"]);
        if (err) { return err; }
    }

    for (auto& comp : components) {
        bool found = false;
        for (const auto& dep : meta.dependencies) {
            if (comp.second.path == dep) {
                found = true;
                break;
            }
        }
        if (!found) { return ERROR(error_t::UNDEFINED_DEPENDENCY); }
    }
    return error_t::OK;
}

error_t point_t::from_json(const Json::Value& doc)
{
    x = doc["x"].isInt() ? doc["x"].asInt() : 0;
    y = doc["y"].isInt() ? doc["y"].asInt() : 0;
    return error_t::OK;
}

error_t node::from_json(const Json::Value& doc)
{

    if (doc["id"].isString()) {
        id = std::atol(doc["id"].asCString());
    } else if (doc["id"].isInt()) {
        id = doc["id"].asInt();
    }
    if (doc["type"].isString()) { type = str_to_node(doc["type"].asString()); }
    if (type >= node_t::NODE_S) { return ERROR(error_t::INVALID_NODE); }
    return error_t::OK;
}

error_t Rel::from_json(const Json::Value& doc)
{
    if (!(doc["from"].isObject() && doc["to"].isObject())) {
        return error_t::INVALID_NODE;
    }
    error_t err = from_node.from_json(doc["from"]);
    if (err) { return err; }

    err = to_node.from_json(doc["to"]);
    if (err) { return err; }

    from_sock = doc["from"].get("sock", 0).asInt();
    to_sock   = doc["to"].get("sock", 0).asInt();

    if (doc["curve"].isArray()) {
        for (const auto& j : doc["curve"]) {
            point_t p;
            err = p.from_json(j);
            if (err) { return err; }
            curve_points.push_back(p);
        }
    }
    return error_t::OK;
}

error_t GateNode::from_json(const Json::Value& doc)
{
    if (doc["size"].isInt()) {
        int size  = doc["size"].asInt();
        bool pass = true;
        while (_max_in < size && pass) {
            pass = increment();
        }
    }

    return error_t::OK;
}

error_t ComponentNode::from_json(const Json::Value& doc)
{
    if (!doc["depends"].isString()) {
        return ERROR(error_t::INVALID_COMPONENT);
    }
    set_component(doc["depends"].asString());
    return error_t::OK;
}

error_t InputNode::from_json(const Json::Value& doc)
{
    if (doc["freq"].isInt()) {
        _freq = doc["freq"].isInt();
        if (_freq == 0) { _freq = std::nullopt; }
    } else if (doc["data"].isBool()) {
        set(doc["data"].asBool());
    } else {
        return ERROR(error_t::INVALID_INPUT);
    }
    return error_t::OK;
}

error_t OutputNode::from_json(const Json::Value&) { return error_t::OK; }

error_t sys::Metadata::from_json(const Json::Value& doc)
{
    if (!(doc["name"].isString() && doc["author"].isString()
            && doc["version"].isInt())) {
        return ERROR(error_t::INVALID_SCENE);
    }
    name   = doc["name"].asString();
    author = doc["author"].asString();
    if (doc["description"].isString()) {
        description = doc["description"].asString();
    }
    version = doc["version"].asInt();
    if (doc["dependencies"].isArray()) {
        return error_t::OK;
        for (const auto& j : doc["dependencies"]) {
            error_t err = sys::verify_component(j.asString());
            if (err) { return err; }
            dependencies.push_back(j.asString());
        }
    }
    return error_t::OK;
}

error_t ComponentContext::from_json(const Json::Value& doc)
{
    if (!(doc["in"].isInt() && doc["out"].isInt())) {
        return ERROR(error_t::INVALID_COMPONENT);
    }
    setup(doc["in"].asInt(), doc["out"].asInt());
    return error_t::OK;
}

template <typename T>
error_t _json_to_map(Scene* s, const Json::Value& doc, std::map<node, T>& m)
{
    node last_id = 0;
    error_t err  = error_t::OK;

    for (Json::Value::const_iterator iter = doc.begin(); iter != doc.end();
        iter++) {
        const Json::Value& value = *iter;
        node id { static_cast<uint32_t>(std::atoll(iter.key().asCString())),
            to_node_type<T>() };
        if (id.id > last_id.id) { last_id = id; }
        direction_t dir = str_to_dir(value.get("dir", "r").asString());
        point_t point { 0, 0 };
        if (value["pos"].isObject()) { err = point.from_json(value["pos"]); }
        if (err) { return err; }

        if constexpr (std::is_same<T, GateNode>::value) {
            if (!value["gate"].isString()) {
                return ERROR(error_t::INVALID_GATE);
            }
            T t { s, id, str_to_gate(value["gate"].asString()) };
            err = t.from_json(value);
            if (err) { return err; }
            t.point = point;
            t.dir   = dir;
            m.emplace(id, t);
        } else {
            T t { s, id };
            err = t.from_json(value);
            if (err) { return err; }
            t.point = point;
            t.dir   = dir;
            m.emplace(id, t);
        }
    }
    s->last_node[last_id.type] = last_id;
    return error_t::OK;
}

error_t _connect_all(Scene* s, const Json::Value& rel)
{
    for (Json::Value::const_iterator iter = rel.begin(); iter != rel.end();
        iter++) {
        relid id = std::atol(iter.key().asCString());
        Rel r;
        r.id        = id;
        error_t err = r.from_json(*iter);
        if (err) { return err; }
        if (id > s->last_rel) { s->last_rel = id; }
        if (s->connect_with_id(
                r.id, r.to_node, r.to_sock, r.from_node, r.from_sock)) {
            return ERROR(error_t::REL_CONNECT_ERROR);
        }
    }
    return error_t::OK;
}

namespace parse {
    error_t load_scene(const std::string& str, Scene& scene)
    {
        Json::Reader reader {};
        Json::Value root;
        bool ok = reader.parse(str, root);
        if (!ok) { return ERROR(error_t::INVALID_JSON_FORMAT); }
        return scene.from_json(root);
    }

} // namespace parse
} // namespace lcs
