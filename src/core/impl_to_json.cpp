#include "core.h"
#include "io.h"
#include <json/value.h>
#include <string>

namespace lcs {

template <typename T> Json::Value _to_json(const std::map<Node, T>& m)
{
    Json::Value doc { Json::objectValue };
    for (const auto& c : m) {
        std::string id = std::to_string(c.first.id);
        doc[id]        = c.second.to_json();
        if (c.second.point.x != 0 || c.second.point.y != 0) {
            doc[id]["pos"] = c.second.point.to_json();
        }
    }
    return doc;
}

Json::Value Node::to_json() const
{
    Json::Value out;
    out["id"]
        = std::string { NodeType_to_str(type) } + "@" + std::to_string(id);
    return out;
}

Json::Value Point::to_json() const
{
    Json::Value out { Json::objectValue };
    out["x"] = x;
    out["y"] = y;
    return out;
}

Json::Value Rel::to_json() const
{
    Json::Value out { Json::objectValue };
    Json::Value from = from_node.to_json();
    if (from_sock) {
        from["sock"] = from_sock;
    }
    Json::Value to = to_node.to_json();
    if (to_sock) {
        to["sock"] = to_sock;
    }
    out["from"] = from;
    out["to"]   = to;
    return out;
}

Json::Value GateNode::to_json() const
{
    Json::Value out;
    out["type"] = GateType_to_str(type());
    if (type() != GateType::NOT && inputs.size() != 2) {
        out["size"] = inputs.size();
    }
    return out;
}

Json::Value InputNode::to_json() const
{
    Json::Value out;
    if (_freq.has_value()) {
        out["freq"] = _freq.value();
    } else {
        out["data"] = get() == State::TRUE ? true : false;
    }
    return out;
}

Json::Value ComponentNode::to_json() const
{
    Json::Value out;
    out["use"] = path;
    return out;
}

Json::Value OutputNode::to_json() const { return Json::Value {}; }

Json::Value ComponentContext::to_json() const
{
    Json::Value out { Json::objectValue };
    out["in"]  = inputs.size();
    out["out"] = outputs.size();
    return out;
}

Json::Value Scene::to_json() const
{
    Json::Value out { Json::objectValue };
    out["name"]   = name.data();
    out["author"] = author.data();
    if (!description.empty()) {
        out["description"] = description.data();
    }
    out["version"] = version;
    if (!dependencies.empty()) {
        Json::Value dep { Json::arrayValue };
        for (const auto& d : dependencies) {
            if (auto d_meta = io::component::get(d); d_meta != nullptr) {
                dep.append(d_meta->to_dependency());
            }
        }
        out["dependencies"] = dep;
    }
    out["nodes"] = { Json::objectValue };
    if (!_gates.empty()) {
        out["nodes"][NodeType_to_str(NodeType::GATE)]
            = _to_json<GateNode>(_gates);
    }
    if (!_inputs.empty()) {
        out["nodes"][NodeType_to_str(NodeType::INPUT)]
            = _to_json<InputNode>(_inputs);
    }
    if (!_outputs.empty()) {
        out["nodes"][NodeType_to_str(NodeType::OUTPUT)]
            = _to_json<OutputNode>(_outputs);
    }
    if (!_components.empty()) {
        out["nodes"][NodeType_to_str(NodeType::COMPONENT)]
            = _to_json<ComponentNode>(_components);
    }

    if (!_relations.empty()) {
        Json::Value doc { Json::objectValue };
        for (const auto& c : _relations) {
            doc[std::to_string(c.first)] = c.second.to_json();
        }
        out["rel"] = doc;
    }
    if (component_context.has_value()) {
        out["component"] = component_context.value().to_json();
    }
    return out;
}

} // namespace lcs
