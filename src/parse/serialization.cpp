#include "core.h"
#include "parse.h"
#include "json/value.h"
#include <string>
namespace lcs {

template <typename T> Json::Value _to_json(const std::map<node, T>& m)
{
    Json::Value doc { Json::objectValue };
    for (const auto& c : m) {
        std::string id = std::to_string(c.first.id);
        doc[id]        = c.second.to_json();
    }
    return doc;
}

Json::Value Scene::to_json(void) const
{
    Json::Value out { Json::objectValue };
    out["meta"] = meta.to_json();
    if (!gates.empty()) { out["nodes"]["gates"] = _to_json<GateNode>(gates); }
    if (!inputs.empty()) {
        out["nodes"]["inputs"] = _to_json<InputNode>(inputs);
    }
    if (!outputs.empty()) {
        out["nodes"]["outputs"] = _to_json<OutputNode>(outputs);
    }
    if (!components.empty()) {
        out["nodes"]["comp"] = _to_json<ComponentNode>(components);
    }

    if (!rel.empty()) {
        Json::Value doc { Json::objectValue };
        for (const auto& c : rel) {
            doc[std::to_string(c.first)] = c.second.to_json();
        }
        out["rel"] = doc;
    }
    if (component_context.has_value()) {
        out["component"] = component_context->to_json();
    }
    return out;
}

Json::Value node::to_json(void) const
{
    Json::Value out { Json::objectValue };
    out["id"]   = id;
    out["type"] = node_to_str(type);
    return out;
}

Json::Value point_t::to_json(void) const
{
    Json::Value out { Json::objectValue };
    out["x"] = x;
    out["y"] = y;
    return out;
}

Json::Value Rel::to_json(void) const
{
    Json::Value out { Json::objectValue };
    Json::Value from = from_node.to_json();
    if (from_sock) { from["sock"] = from_sock; }
    Json::Value to = to_node.to_json();
    if (to_sock) { to["sock"] = to_sock; }
    out["from"] = from;
    out["to"]   = to;

    if (!curve_points.empty()) {
        Json::Value out_curve { Json::arrayValue };
        for (const auto& c : curve_points) {
            out_curve.append(c.to_json());
        }
        out["curve"] = out_curve;
    }
    return out;
}

Json::Value GateNode::to_json(void) const
{
    Json::Value out = this->BaseNode::to_json();
    out["gate"]     = gate_to_str(type);
    if (type != gate_t::NOT && max_in != 2) { out["size"] = max_in; }
    return out;
}

Json::Value InputNode::to_json(void) const
{
    Json::Value out = this->BaseNode::to_json();
    if (freq.has_value()) {
        out["freq"] = freq.value();
    } else {
        out["data"] = value;
    }
    return out;
}
Json::Value ComponentNode::to_json(void) const
{
    Json::Value out = this->BaseNode::to_json();
    out["depends"]  = path;
    return out;
}

Json::Value OutputNode::to_json(void) const
{
    return this->BaseNode::to_json();
}

Json::Value sys::Metadata::to_json(void) const
{
    Json::Value out { Json::objectValue };
    out["name"]   = name;
    out["author"] = author;
    if (description != "") { out["description"] = description; }
    out["version"] = version;
    if (!dependencies.empty()) {
        Json::Value dep { Json::arrayValue };
        for (const auto& d : dependencies) {
            if (auto d_meta = sys::get_dependency(d); d_meta != nullptr) {
                dep.append(d_meta->meta.to_dependency_string());
            } else {
                return ERROR(error_t::INVALID_COMPONENT);
            }
        }
        out["dependencies"] = dep;
    }
    return out;
}

Json::Value ComponentContext::to_json(void) const
{
    Json::Value out { Json::objectValue };
    out["in"]  = inputs.size();
    out["out"] = outputs.size();
    return out;
}

Json::Value BaseNode::to_json(void) const
{
    Json::Value out { Json::objectValue };
    if (dir != direction_t::RIGHT) { out["dir"] = direction_to_str(dir); }
    if (point.x != 0 || point.y != 0) { out["pos"] = point.to_json(); }
    return out;
}

} // namespace lcs
