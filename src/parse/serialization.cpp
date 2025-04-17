#include "parse.h"
namespace lcs {
namespace parse {

    /** Converts a node object to a JSON value */
    static Json::Value _to_json(const node& v);
    /** Converts a point_t object to a JSON value */
    static Json::Value _to_json(const point_t& v);
    /** Converts a Rel object to a JSON value */
    static Json::Value _to_json(const Rel& v);
    /** Converts a Gate object to a JSON value */
    static Json::Value _to_json(const Gate& v);
    /** Converts a Input object to a JSON value */
    static Json::Value _to_json(const Input& v);
    /** Converts a Output object to a JSON value */
    static Json::Value _to_json(const Output& v);
    /** Converts a Metadata object to a JSON value */
    static Json::Value _to_json(const Metadata& v);
    /** Converts the base node class to a JSON value */
    static Json::Value _to_json_base(const BaseNode& v);
    /** Converts all elements in the map to json */
    template <typename T>
    static Json::Value _to_json(const std::map<node, T>& m);

    Json::Value to_json(Scene& s)
    {
        Json::Value out { Json::objectValue };
        out["meta"] = _to_json(s.meta);
        if (!s.gates.empty()) {
            out["nodes"]["gates"] = _to_json<Gate>(s.gates);
        }
        if (!s.inputs.empty()) {
            out["nodes"]["inputs"] = _to_json<Input>(s.inputs);
        }
        if (!s.outputs.empty()) {
            out["nodes"]["outputs"] = _to_json<Output>(s.outputs);
        }

        if (!s.rel.empty()) {
            Json::Value doc { Json::objectValue };
            for (const auto& c : s.rel) {
                doc[std::to_string(c.first)] = _to_json(c.second);
            }
            out["rel"] = doc;
        }
        return out;
    }

    Json::Value _to_json(const node& v)
    {
        Json::Value out { Json::objectValue };
        out["id"]   = v.id;
        out["type"] = node_to_str(v.type);
        return out;
    }

    Json::Value _to_json(const point_t& v)
    {
        Json::Value out { Json::objectValue };
        out["x"] = v.x;
        out["y"] = v.y;
        return out;
    }

    Json::Value _to_json(const Rel& v)
    {
        Json::Value out { Json::objectValue };
        Json::Value from = _to_json(v.from_node);
        if (v.from_sock) { from["sock"] = v.from_sock; }
        Json::Value to = _to_json(v.to_node);
        if (v.to_sock) { to["sock"] = v.to_sock; }
        out["from"] = from;
        out["to"]   = to;

        if (!v.curve_points.empty()) {
            Json::Value curve_points { Json::arrayValue };
            for (const auto& c : v.curve_points) {
                curve_points.append(_to_json(c));
            }
            out["curve"] = curve_points;
        }
        return out;
    }

    Json::Value _to_json(const Gate& v)
    {
        Json::Value out = _to_json_base(v);
        out["gate"]     = gate_to_str(v.type);
        if (v.type != gate_t::NOT && v.max_in != 2) { out["size"] = v.max_in; }
        return out;
    }

    Json::Value _to_json(const Input& v)
    {
        Json::Value out = _to_json_base(v);
        if (v.type) {
            out["freq"] = v.freq;
        } else {
            out["data"] = v.value;
        }
        return out;
    }

    Json::Value _to_json(const Output& v) { return _to_json_base(v); }

    Json::Value _to_json(const Metadata& v)
    {
        Json::Value out { Json::objectValue };
        out["name"]   = v.name;
        out["author"] = v.author;
        if (v.description != "") { out["description"] = v.description; }
        out["version"] = v.version;
        if (!v.dependencies.empty()) {
            Json::Value dep { Json::arrayValue };
            for (const auto& d : v.dependencies) {
                dep.append(d);
            }
            out["dependencies"] = dep;
        }
        return out;
    }

    template <typename T> Json::Value _to_json(const std::map<node, T>& m)
    {
        Json::Value doc { Json::objectValue };
        for (const auto& c : m) {
            std::string id = std::to_string(c.first.id);
            doc[id]        = _to_json(c.second);
        }
        return doc;
    }

    Json::Value _to_json_base(const BaseNode& v)
    {
        Json::Value out { Json::objectValue };
        if (v.dir != direction_t::RIGHT) {
            out["dir"] = direction_to_str(v.dir);
        }
        if (v.point.x != 0 || v.point.y != 0) {
            out["pos"] = _to_json(v.point);
        }
        return out;
    }

} // namespace parse
} // namespace lcs
