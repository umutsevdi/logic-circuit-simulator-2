#include "core.h"
#include "parse.h"
#include <json/json.h>
#include <map>

namespace lcs {
namespace parse {
    /** Reads a JSON document and writes values to a point_t object */
    static error_t _from_json(const Json::Value& doc, point_t& v);
    /** Reads a JSON document and writes values to a node object */
    static error_t _from_json(const Json::Value& doc, node& v);
    /** Reads a JSON document and writes values to a Rel object */
    static error_t _from_json(const Json::Value& doc, relid id, Rel& v);
    /** Reads a JSON document and writes values to a GateNode object */
    static error_t _from_json(const Json::Value& doc, GateNode& v);
    /** Reads a JSON document and writes values to a Value object */
    static error_t _from_json(const Json::Value&, ComponentNode&);
    /** Reads a JSON document and writes values to a InputNode object */
    static error_t _from_json(const Json::Value& doc, InputNode& v);
    /** Reads a JSON document and writes values to a Value object */
    static error_t _from_json(const Json::Value&, OutputNode&);
    /** Reads a JSON document and writes values to a Metadata object */
    static error_t _from_json(const Json::Value& doc, sys::Metadata& v);
    /** Reads a JSON document and writes values to a ComponentNode Context
     * object */
    static error_t _from_json(
        const Json::Value& doc, std::optional<ComponentContext>& v);
    /** Reads a JSON document and writes its values to a map */
    template <typename T>
    static error_t _json_to_map(
        Scene* s, const Json::Value& doc, std::map<node, T>& m);
    /** Connects existing nodes to each other according to the document */
    static error_t _connect_all(Scene* s, const Json::Value& rel);

    error_t from_json(const Json::Value& doc, Scene& v)
    {
        if (!doc["meta"].isObject() || !doc["nodes"].isObject()) {
            return ERROR(error_t::INVALID_SCENE);
        }
        error_t err = _from_json(doc["meta"], v.meta);
        if (err) { return err; }

        if (doc["meta"]["type"].isString()
            && doc["meta"]["type"].asString() == "component") {
            err = _from_json(doc, v.component_context);
            if (err) { return err; }
        }
        const Json::Value& nodes = doc["nodes"];
        if (nodes["gates"].isObject()) {
            err = _json_to_map<GateNode>(&v, nodes["gates"], v.gates);
            if (err) { return err; }
        }
        if (nodes["comp"].isObject()) {
            err = _json_to_map<ComponentNode>(&v, nodes["comp"], v.components);
            if (err) { return err; }
        }
        if (nodes["inputs"].isObject()) {
            err = _json_to_map<InputNode>(&v, nodes["inputs"], v.inputs);
            if (err) { return err; }
        }
        if (nodes["outputs"].isObject()) {
            err = _json_to_map<OutputNode>(&v, nodes["outputs"], v.outputs);
            if (err) { return err; }
        }
        if (doc["rel"].isObject()) {
            err = _connect_all(&v, doc["rel"]);
            if (err) { return err; }
        }

        for (auto& comp : v.components) {
            bool found = false;
            for (const auto& dep : v.meta.dependencies) {
                if (comp.second.path == dep) {
                    found = true;
                    break;
                }
            }
            if (!found) { return ERROR(error_t::UNDEFINED_DEPENDENCY); }
        }
        return error_t::OK;
    }

    error_t _from_json(const Json::Value& doc, point_t& v)
    {
        v.x = doc["x"].isInt() ? doc["x"].asInt() : 0;
        v.y = doc["y"].isInt() ? doc["y"].asInt() : 0;
        return error_t::OK;
    }

    error_t _from_json(const Json::Value& doc, node& v)
    {

        if (doc["id"].isString()) {
            v.id = std::atol(doc["id"].asCString());
        } else if (doc["id"].isInt()) {
            v.id = doc["id"].asInt();
        }
        if (doc["type"].isString()) {
            v.type = str_to_node(doc["type"].asString());
        }
        if (v.type >= node_t::NODE_S) { return ERROR(error_t::INVALID_NODE); }
        return error_t::OK;
    }

    error_t _from_json(const Json::Value& doc, relid id, Rel& v)
    {
        v.id = id;
        if (!(doc["from"].isObject() && doc["to"].isObject())) {
            return error_t::INVALID_NODE;
        }
        error_t err = _from_json(doc["from"], v.from_node);
        if (err) { return err; }

        err = _from_json(doc["to"], v.to_node);
        if (err) { return err; }

        v.from_sock = doc["from"].get("sock", 0).asInt();
        v.to_sock   = doc["to"].get("sock", 0).asInt();

        if (doc["curve"].isArray()) {
            for (const auto& j : doc["curve"]) {
                point_t p;
                err = _from_json(j, p);
                if (err) { return err; }
                v.curve_points.push_back(p);
            }
        }
        return error_t::OK;
    }

    error_t _from_json(const Json::Value& doc, GateNode& v)
    {
        if (doc["size"].isInt()) {
            int max_in = doc["size"].asInt();
            bool pass  = true;
            while (v.max_in < max_in && pass) {
                pass = v.increment();
            }
        }

        return error_t::OK;
    }

    error_t _from_json(const Json::Value& doc, ComponentNode& v)
    {
        if (!doc["depends"].isString()) {
            return ERROR(error_t::INVALID_COMPONENT);
        }
        v.set_component(doc["depends"].asString());
        return error_t::OK;
    }

    error_t _from_json(const Json::Value& doc, InputNode& v)
    {
        if (doc["freq"].isInt()) {
            v.set_freq(doc["freq"].isInt());
        } else if (doc["data"].isBool()) {
            v.set(doc["data"].asBool());
        } else {
            return ERROR(error_t::INVALID_INPUT);
        }
        return error_t::OK;
    }

    error_t _from_json(const Json::Value&, OutputNode&) { return error_t::OK; }

    error_t _from_json(const Json::Value& doc, sys::Metadata& v)
    {
        if (!(doc["name"].isString() && doc["author"].isString()
                && doc["version"].isInt())) {
            return ERROR(error_t::INVALID_SCENE);
        }
        v.name   = doc["name"].asString();
        v.author = doc["author"].asString();
        if (doc["description"].isString()) {
            v.description = doc["description"].asString();
        }
        v.version = doc["version"].asInt();
        if (doc["dependencies"].isArray()) {
            return error_t::OK;
            for (const auto& j : doc["dependencies"]) {
                error_t err = sys::verify_component(j.asString());
                if (err) { return err; }
                v.dependencies.push_back(j.asString());
            }
        }
        return error_t::OK;
    }

    static error_t _from_json(
        const Json::Value& doc, std::optional<ComponentContext>& v)
    {
        if (!(doc["size_in"].isInt() && doc["size_out"].isInt())) {
            return ERROR(error_t::INVALID_COMPONENT);
        }
        v.emplace(doc["size_in"].asInt(), doc["size_out"].asInt());
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
            if (value["pos"].isObject()) {
                err = _from_json(value["pos"], point);
            }
            if (err) { return err; }

            if constexpr (std::is_same<T, GateNode>::value) {
                if (!value["gate"].isString()) {
                    return ERROR(error_t::INVALID_GATE);
                }
                T t { s, id, str_to_gate(value["gate"].asString()) };
                err = _from_json(value, t);
                if (err) { return err; }
                t.point = point;
                t.dir   = dir;
                m.emplace(id, t);
            } else {
                T t { s, id };
                err = _from_json(value, t);
                if (err) { return err; }
                t.point = point;
                t.dir   = dir;
                m.emplace(id, t);
            }
        }
        s->last_node[last_id.type] = last_id.id;
        return error_t::OK;
    }

    error_t _connect_all(Scene* s, const Json::Value& rel)
    {
        for (Json::Value::const_iterator iter = rel.begin(); iter != rel.end();
            iter++) {
            relid id = std::atol(iter.key().asCString());
            Rel r;
            error_t err = _from_json(*iter, id, r);
            if (err) { return err; }
            if (id > s->last_rel) { s->last_rel = id; }
            if (s->connect_with_id(
                    r.id, r.to_node, r.to_sock, r.from_node, r.from_sock)) {
                return ERROR(error_t::REL_CONNECT_ERROR);
            }
        }
        return error_t::OK;
    }

    error_t from_json(const std::string& doc, Scene& s)
    {
        Json::Reader reader {};
        Json::Value root;
        bool ok = reader.parse(doc, root);
        if (!ok) { return ERROR(error_t::INVALID_JSON_FORMAT); }
        error_t err = parse::from_json(root, s);
        return err;
    }
} // namespace parse
} // namespace lcs
