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
        doc[id]        = to_json<T>(c.second);
        if (c.second.point.x != 0 || c.second.point.y != 0) {
            doc[id]["pos"] = to_json<Point>(c.second.point);
        }
    }
    return doc;
}

template <> Json::Value to_json<Node>(const Node& node)
{
    Json::Value out;
    out["id"] = std::string { NodeType_to_str(node.type) } + "@"
        + std::to_string(node.id);
    return out;
}

template <> Json::Value to_json<Point>(const Point& point)
{
    Json::Value out { Json::objectValue };
    out["x"] = point.x;
    out["y"] = point.y;
    return out;
}

template <> Json::Value to_json<Rel>(const Rel& rel)
{
    Json::Value out { Json::objectValue };
    Json::Value from = to_json<Node>(rel.from_node);
    if (rel.from_sock) {
        from["sock"] = rel.from_sock;
    }
    Json::Value to = to_json<Node>(rel.to_node);
    if (rel.to_sock) {
        to["sock"] = rel.to_sock;
    }
    out["from"] = from;
    out["to"]   = to;
    return out;
}

template <> Json::Value to_json<GateNode>(const GateNode& node)
{
    Json::Value out;
    out["type"] = GateType_to_str(node.type());
    if (node.type() != GateType::NOT && node.inputs.size() != 2) {
        out["size"] = node.inputs.size();
    }
    return out;
}

template <> Json::Value to_json<InputNode>(const InputNode& node)
{
    Json::Value out;
    if (node._freq.has_value()) {
        out["freq"] = node._freq.value();
    } else {
        out["data"] = node.get() == State::TRUE ? true : false;
    }
    return out;
}

template <> Json::Value to_json<ComponentNode>(const ComponentNode& node)
{
    Json::Value out;
    out["use"] = node.path;
    return out;
}

template <> Json::Value to_json<OutputNode>(const OutputNode&)
{
    return Json::Value {};
}

template <> Json::Value to_json<ComponentContext>(const ComponentContext& ctx)
{
    Json::Value out { Json::objectValue };
    out["in"]  = ctx.inputs.size();
    out["out"] = ctx.outputs.size();
    return out;
}

template <> Json::Value to_json<Scene>(const Scene& scene)
{
    Json::Value out { Json::objectValue };
    out["name"]   = scene.name.data();
    out["author"] = scene.author.data();
    if (!scene.description.empty()) {
        out["description"] = scene.description.data();
    }
    out["version"] = scene.version;
    if (!scene.dependencies.empty()) {
        Json::Value dep { Json::arrayValue };
        for (const auto& d : scene.dependencies) {
            if (auto d_meta = io::component::get(d); d_meta != nullptr) {
                dep.append(d_meta->to_dependency());
            }
        }
        out["dependencies"] = dep;
    }
    out["nodes"] = { Json::objectValue };
    if (!scene._gates.empty()) {
        out["nodes"][NodeType_to_str(NodeType::GATE)]
            = _to_json<GateNode>(scene._gates);
    }
    if (!scene._inputs.empty()) {
        out["nodes"][NodeType_to_str(NodeType::INPUT)]
            = _to_json<InputNode>(scene._inputs);
    }
    if (!scene._outputs.empty()) {
        out["nodes"][NodeType_to_str(NodeType::OUTPUT)]
            = _to_json<OutputNode>(scene._outputs);
    }
    if (!scene._components.empty()) {
        out["nodes"][NodeType_to_str(NodeType::COMPONENT)]
            = _to_json<ComponentNode>(scene._components);
    }

    if (!scene._relations.empty()) {
        Json::Value doc { Json::objectValue };
        for (const auto& c : scene._relations) {
            doc[std::to_string(c.first)] = to_json<Rel>(c.second);
        }
        out["rel"] = doc;
    }
    if (scene.component_context.has_value()) {
        out["component"]
            = to_json<ComponentContext>(scene.component_context.value());
    }
    return out;
}

} // namespace lcs
