#include "parse.h"
#include "core/engine.h"
#include "core/types.h"
#include <map>
#include <string>

namespace lcs {
namespace parse {

    /***************************************************************************
                                    Serialization
    ***************************************************************************/

    Json::Value _base_to_json(const BaseNode& v);
    template <typename T> Json::Value _map_to_json(const std::map<node, T>& m);

    Json::Value to_json(const node& v)
    {
        Json::Value out { Json::objectValue };
        out["id"]   = v.id;
        out["type"] = node_to_str(v.type);
        return out;
    }

    Json::Value to_json(const point_t& v)
    {
        Json::Value out { Json::objectValue };
        out["x"] = v.x;
        out["y"] = v.y;
        return out;
    }

    Json::Value to_json(const Rel& v)
    {
        Json::Value out { Json::objectValue };
        Json::Value from { Json::objectValue };
        from["sock"] = v.from_sock;
        from["node"] = to_json(v.from_node);
        Json::Value to { Json::objectValue };
        to["sock"]  = v.to_sock;
        to["node"]  = to_json(v.to_node);
        out["from"] = from;
        out["to"]   = to;

        if (!v.curve_points.empty()) {
            Json::Value curve_points { Json::arrayValue };
            for (const auto& c : v.curve_points) {
                curve_points.append(to_json(c));
            }
            out["curve"] = curve_points;
        }
        return out;
    }

    Json::Value to_json(const Gate& v)
    {
        Json::Value out = _base_to_json(v);
        out["gate"]     = gate_to_str(v.type);
        if (v.type != gate_t::NOT && v.max_in != 2) { out["size"] = v.max_in; }
        return out;
    }

    Json::Value to_json(const Input& v)
    {
        Json::Value out = _base_to_json(v);
        out["data"]     = v.value;
        return out;
    }

    Json::Value to_json(const Output& v)
    {
        Json::Value out = _base_to_json(v);
        out["data"]     = v.value == state_t::TRUE ? "true"
                : v.value == state_t::FALSE        ? "false"
                                                   : "disabled";
        return out;
    }

    Json::Value to_json(const Metadata& v)
    {
        Json::Value out { Json::objectValue };
        //  out << YAML::BeginMap
        //      << PAIR("name", v.name) << PAIR("description", v.description)
        //      << PAIR("type", "SCENE") << PAIR("author", "person")
        //      << PAIR("version", 1.0) << PAIR("last_node_id", v.last_node)
        //      << PAIR("last_rel_id", v.last_rel)
        //      << PAIR("depends", YAML::BeginSeq << YAML::EndSeq);
        //  out << PAIR("nodes", YAML::BeginMap);
        return out;
    }

    Json::Value to_json(Scene& s)
    {
        Json::Value out { Json::objectValue };
        out["meta"] = to_json(s.meta);
        if (!s.gates.empty()) { out["gates"] = _map_to_json<Gate>(s.gates); }
        if (!s.inputs.empty()) {
            out["inputs"] = _map_to_json<Input>(s.inputs);
        }
        if (!s.outputs.empty()) {
            out["outputs"] = _map_to_json<Output>(s.outputs);
        }

        if (!s.rel.empty()) {
            Json::Value doc { Json::objectValue };
            for (const auto& c : s.rel) {
                doc[std::to_string(c.first)] = to_json(c.second);
            }
            out["rel"] = doc;
        }
        return out;
    }

    template <typename T> Json::Value _map_to_json(const std::map<node, T>& m)
    {
        Json::Value doc { Json::objectValue };
        for (const auto& c : m) {
            std::string id  = std::to_string(c.first.id);
            doc[id]         = to_json(c.second);
            doc[id]["type"] = node_to_str(c.first.type);
        }
        return doc;
    }

    Json::Value _base_to_json(const BaseNode& v)
    {
        Json::Value out { Json::objectValue };
        if (v.dir != direction_t::RIGHT) {
            out["dir"] = direction_to_str(v.dir);
        }
        if (v.point.x != 0 || v.point.y != 0) { out["pos"] = to_json(v.point); }
        return out;
    }

    /******************************************************************************
                                    Deserialization
    *****************************************************************************/

    template <typename T>
    error_t _json_to_map(
        Scene* s, const Json::Value& doc, std::map<node, T>& m);
    error_t _json_to_map(
        Scene* s, const Json::Value& rel, std::map<relid, Rel>& m);

    error_t from_json(const Json::Value& doc, point_t& v)
    {
        v.x = doc["x"].isInt() ? doc["x"].asInt() : 0;
        v.y = doc["y"].isInt() ? doc["y"].asInt() : 0;
        return error_t::OK;
    }

    error_t from_json(const Json::Value& doc, node& v)
    {
        if (!doc["id"].isString() || !doc["type"].isString()) {
            return S_ERROR(error_t::INVALID_NODE);
        }
        v.id   = std::atol(doc["id"].asCString());
        v.type = str_to_node(doc["type"].asString());
        if (v.type >= node_t::NODE_S) { return S_ERROR(error_t::INVALID_NODE); }
        return error_t::OK;
    }

    error_t from_json(const Json::Value& doc, relid id, Rel& v)
    {
        v.id = id;
        if (!(doc["from"].isObject() && doc["to"].isObject()
                && doc["from"]["node"].isObject() && doc["from"]["sock"].isInt()
                && doc["to"]["node"].isObject() && doc["to"]["sock"].isInt())) {
            return parse::error_t::INVALID_NODE;
        }
        error_t err = from_json(doc["from"]["node"], v.from_node);
        if (!err) { return err; }

        err = from_json(doc["to"]["node"], v.to_node);
        if (!err) { return err; }

        v.from_sock = doc["from"]["sock"].asInt();
        v.to_sock   = doc["to"]["sock"].asInt();

        if (doc["curve"].isArray()) {
            for (const auto& j : doc["curve"]) {
                point_t p;
                err = from_json(j, p);
                if (!err) { return err; }
                v.curve_points.push_back(p);
            }
        }
        return error_t::OK;
    }

    error_t from_json(const Json::Value& doc, Gate& v) { return error_t::OK; }
    error_t from_json(const Json::Value& doc, Component& v)
    {
        return error_t::OK;
    }
    error_t from_json(const Json::Value& doc, Input& v) { return error_t::OK; }
    error_t from_json(const Json::Value& doc, Output& v) { return error_t::OK; }
    error_t from_json(const Json::Value& doc, Metadata& v)
    {
        return error_t::OK;
    }

    error_t from_json(const Json::Value& doc, Scene& v)
    {
        if (!doc["meta"].isObject()) { return S_ERROR(error_t::INVALID_SCENE); }

        error_t err = from_json(doc["meta"], v.meta);
        if (!err) { return err; }

        if (doc["gates"].isObject()) {
            err = _json_to_map<Gate>(&v, doc["gates"], v.gates);
            if (!err) { return err; }
        }
        if (doc["comp"].isObject()) {
            err = _json_to_map<Component>(&v, doc["comp"], v.components);
            if (!err) { return err; }
        }
        if (doc["inputs"].isObject()) {
            err = _json_to_map<Input>(&v, doc["inputs"], v.inputs);
            if (!err) { return err; }
        }
        if (doc["outputs"].isObject()) {
            err = _json_to_map<Output>(&v, doc["outputs"], v.outputs);
            if (!err) { return err; }
        }
        if (doc["rel"].isObject()) {
            err = _json_to_map(&v, doc["rel"], v.rel);
            if (!err) { return err; }
        }

        return error_t::OK;
    }

    template <typename T>
    error_t _json_to_map(Scene* s, const Json::Value& doc, std::map<node, T>& m)
    {
        node last_id = 0;
        for (Json::Value::const_iterator iter = doc.begin(); iter != doc.end();
            iter++) {
            node id;
            error_t err = from_json(iter.key(), id);
            if (id.id > last_id.id) { last_id = id; }
            if (!err) { return err; }

            if (!(*iter)["dir"].isString() || !(*iter)["pos"].isObject()) {
                return S_ERROR(error_t::INVALID_NODE);
            }
            direction_t dir = str_to_dir(doc["dir"].asString());
            point_t point;
            err = from_json(doc["pos"], point);
            if (!err) { return err; }

            if constexpr (std::is_same<T, Gate>::value) {
                T t { s, id, str_to_gate(doc["gate"].asString()) };
                err = from_json(*iter, t);
                if (!err) { return err; }
                t.point = point;
                t.dir   = dir;
                m.emplace(id, t);

            } else if constexpr (std::is_same<T, Component>::value) {
                T t { s, id, "" };
                t.point = point;
                t.dir   = dir;
            } else {
                T t { s, id };
                err = from_json(*iter, t);
                if (!err) { return err; }
                t.point = point;
                t.dir   = dir;
                m.emplace(id, t);
            }

            return error_t::OK;
        }
        s->last_node[last_id.type] = last_id.id;
        return error_t::OK;
    }

    error_t _json_to_map(
        Scene* s, const Json::Value& rel, std::map<relid, Rel>& m)
    {
        relid last_rel = 0;
        for (Json::Value::const_iterator iter = rel.begin(); iter != rel.end();
            iter++) {
            relid id = std::atol(iter.key().asCString());
            Rel r;
            error_t err = from_json(*iter, id, r);
            if (!err) { return err; }
            m.emplace(id, r);
            if (id > last_rel) { last_rel = id; }
        }
        s->last_rel = last_rel;
        return error_t::OK;
    }

} // namespace parse
} // namespace lcs
