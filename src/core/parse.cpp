#include "common/common.h"
#include "core/types.h"
#include "engine.h"
#include "yaml-cpp/emittermanip.h"
#include <algorithm>
#include <map>
#include <memory>
#define PAIR(_KEY, _VALUE) YAML::Key << _KEY << YAML::Value << _VALUE
namespace lcs {
namespace parser {
    std::unique_ptr<Scene> read_scene(const YAML::Node& yaml, error_t& err)
    {
        err = OK;
        if (!yaml.IsDefined() || yaml.IsNull() || !yaml.IsMap()) {
            err = error_t::INVALID_NODE;
            return nullptr;
        }
        if (!yaml["type"].IsDefined()
            || yaml["type"].as<std::string>() != "SCENE") {
            err = error_t::INVALID_SCENE;
            return nullptr;
        }
        auto scene = std::make_unique<Scene>();
        err        = scene->parse_scene(yaml);
        if (err != OK) { return nullptr; }
        err = OK;
        return scene;
    }

    error_t write_scene(Scene& s, LcsEmitter& yaml)
    {
        lcs_assert(VERSION == 1);
        yaml << YAML::BeginMap << PAIR("version", VERSION) << PAIR("scene", s)
             << YAML::EndMap;
        return yaml.good() ? OK : WRITE_FMT;
    }

    LcsEmitter::LcsEmitter()
        : YAML::Emitter {}
    {
        SetIndent(4);
        SetBoolFormat(YAML::EMITTER_MANIP::TrueFalseBool);
        SetNullFormat(YAML::EMITTER_MANIP::LowerNull);
    }
} // namespace parser

/******************************************************************************
                                Serialization
*****************************************************************************/

YAML::Emitter& operator<<(YAML::Emitter& out, const node& v)
{
    out << PAIR("id", v.id) << PAIR("type", node_to_str(v.type));
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const point_t& v)
{
    out << YAML::BeginMap << PAIR("x", v.x) << PAIR("y", v.y) << YAML::EndMap;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Rel& v)
{
    out << YAML::BeginMap
        << PAIR("from",
               YAML::BeginMap << v.from_node << PAIR("sock", v.from_sock)
                              << YAML::EndMap)
        <<

        PAIR("to",
            YAML::BeginMap << v.to_node << PAIR("sock", v.to_sock)
                           << YAML::EndMap);
    if (!v.curve_points.empty()) { out << PAIR("curve", v.curve_points); }
    out << YAML::EndMap;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const BaseNode& v)
{
    if (v.get_rotation() != direction_t::RIGHT) {
        out << PAIR("dir", direction_to_str(v.get_rotation()));
    }
    if (!v.get_position().is_zero()) { out << PAIR("point", v.get_position()); }
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Gate& v)
{
    out << YAML::BeginMap;
    if (!v.is_zero()) { out << *(v.get_cbase()); }
    out << PAIR("data", YAML::BeginMap << PAIR("type", gate_to_str(v.type)));
    if (v.type != gate_t::NOT && v.max_in != 2) {
        out << PAIR("size", v.max_in);
    }
    out << YAML::EndMap << YAML::EndMap;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Component& v)
{
    if (!v.is_zero()) {
        out << YAML::BeginMap << *(v.get_cbase()) << YAML::EndMap;
    }
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Input& v)
{
    out << YAML::BeginMap;
    if (!v.is_zero()) { out << *(v.get_cbase()); }
    out << PAIR("data", v.value) << YAML::EndMap;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Output& v)
{
    out << YAML::BeginMap;
    if (!v.is_zero()) { out << *(v.get_cbase()); }
    out << PAIR("data",
        (v.value == state_t::TRUE           ? "true"
                : v.value == state_t::FALSE ? "false"
                                            : "disabled"))
        << YAML::EndMap;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, Scene& v)
{
    out << YAML::BeginMap

        << PAIR("name", v.name) << PAIR("description", v.description)
        << PAIR("type", "SCENE") << PAIR("author", "person")
        << PAIR("version", 1.0) << PAIR("last_node_id", v.last_node)
        << PAIR("last_rel_id", v.last_rel)
        << PAIR("depends", YAML::BeginSeq << YAML::EndSeq);
    out << PAIR("nodes", YAML::BeginMap);
    if (!v.gates.empty()) {
        out << PAIR("gate", YAML::BeginMap);
        for (auto& c : v.gates) {
            out << PAIR(c.first.id, c.second);
        }
        out << YAML::EndMap;
    }

    if (!v.components.empty()) {
        out << PAIR("comp", YAML::BeginMap);
        for (auto& c : v.components) {
            out << PAIR(c.first.id, c.second);
        }
        out << YAML::EndMap;
    }
    if (!v.inputs.empty()) {
        out << PAIR("input", YAML::BeginMap);
        for (auto& c : v.inputs) {
            out << PAIR(c.first.id, c.second);
        }
        out << YAML::EndMap;
    }
    if (!v.outputs.empty()) {
        out << PAIR("output", YAML::BeginMap);
        for (auto& c : v.outputs) {
            out << PAIR(c.first.id, c.second);
        }
        out << YAML::EndMap;
    };
    out << YAML::EndMap;
    if (!v.rel.empty()) out << PAIR("rel", v.rel);
    out << YAML::EndMap;
    return out;
}

/******************************************************************************
                                Deserialization
*****************************************************************************/

static inline void deserialize_base(BaseNode& b, const YAML::Node& node)
{
    if (node["dir"].IsDefined()) {
        std::string dir = node["dir"].as<std::string>();
        b.set_rotation(str_to_dir(dir));
    }
    if (node["point"].IsDefined() && node["point"].IsMap()) {
        b.set_position({
            node["point"]["x"].as<int>(),
            node["point"]["y"].as<int>(),
        });
    }
}

static inline Gate deserialize_gate(
    Scene* s, const node id, const YAML::Node& node)
{
    std::string type = node["type"].as<std::string>();

    gate_t g = str_to_gate(type);

    sockid max_in = 2;
    if (node["size"].IsDefined() && node["size"].IsScalar()) {
        max_in = node["size"].as<sockid>();
    }
    Gate gate { s, id, g, max_in };
    return gate;
}

static inline Component deserialize_comp(
    Scene* s, const node id, const YAML::Node& node)
{
    return Component { s, id, node["path"].as<std::string>() };
}

parser::error_t Scene::parse_scene(const YAML::Node& node)
{
    if (!(node["name"].IsDefined() && node["description"].IsDefined()
            && node["author"].IsDefined() && node["version"].IsScalar()
            && node["nodes"].IsMap())) {
        return S_ERROR(parser::PARSE_ERROR);
    } else {
        name         = node["name"].as<std::string>();
        description  = node["description"].as<std::string>();
        author       = node["name"].as<std::string>();
        version      = node["version"].as<int>();
        dependencies = node["depends"].as<std::vector<std::string>>();
        if (node["nodes"]["gate"].IsDefined()
            && node["nodes"]["gate"].IsMap()) {
            for (auto iter = node["nodes"]["gate"].begin();
                iter != node["nodes"]["gate"].end(); iter++) {
                const struct node id { iter->first.as<uint32_t>(),
                    node_t::GATE };
                if (!(iter->second["data"].IsMap()
                        && iter->second["data"]["type"].IsDefined())) {
                    return S_ERROR(parser::PARSE_ERROR);
                }
                Gate g = deserialize_gate(this, id, iter->second["data"]);
                deserialize_base(g, iter->second);
                gates.emplace(id, g);
                if (last_node[node_t::GATE] < id.id) {
                    last_node[node_t::GATE] = id.id;
                }
            }
        }
        if (node["nodes"]["comp"].IsDefined()
            && node["nodes"]["comp"].IsMap()) {
            for (auto iter = node["nodes"]["comp"].begin();
                iter != node["nodes"]["comp"].end(); iter++) {
                const struct node id { iter->first.as<uint32_t>(),
                    node_t::COMPONENT };
                if (!iter->second["data"].IsMap()) {
                    return S_ERROR(parser::PARSE_ERROR);
                }
                Component g = deserialize_comp(this, id, iter->second["data"]);
                deserialize_base(g, iter->second);
                components.emplace(id, g);
                if (last_node[node_t::COMPONENT] < id.id) {
                    last_node[node_t::COMPONENT] = id.id;
                }
            }
        }
        if (node["nodes"]["input"].IsDefined()
            && node["nodes"]["input"].IsMap()) {
            for (auto iter = node["nodes"]["input"].begin();
                iter != node["nodes"]["input"].end(); iter++) {
                const struct node id { iter->first.as<uint32_t>(),
                    node_t::INPUT };
                Input i { this, id };
                i.set(iter->second["data"].IsDefined()
                        ? iter->second["data"].as<bool>()
                        : false);
                deserialize_base(i, iter->second);
                inputs.emplace(id, i);
                if (last_node[node_t::INPUT] < id.id) {
                    last_node[node_t::INPUT] = id.id;
                }
            }
        }
        if (node["nodes"]["output"].IsDefined()
            && node["nodes"]["output"].IsMap()) {
            for (auto iter = node["nodes"]["output"].begin();
                iter != node["nodes"]["output"].end(); iter++) {
                const struct node id { iter->first.as<uint32_t>(),
                    node_t::OUTPUT };
                Output g { this, id };
                deserialize_base(g, iter->second);
                outputs.emplace(id, g);
                if (last_node[node_t::OUTPUT] < id.id) {
                    last_node[node_t::OUTPUT] = id.id;
                }
            }
        }
    }
    if (node["rel"].IsDefined() && node["rel"].IsMap()) {
        for (auto iter = node["rel"].begin(); iter != node["rel"].end();
            iter++) {
            relid id = iter->first.as<relid>();
            if (!(iter->second.IsMap() && iter->second["from"].IsMap()
                    && iter->second["to"].IsMap()
                    && iter->second["from"]["id"].IsScalar()
                    && iter->second["from"]["type"].IsDefined()
                    && iter->second["to"]["id"].IsScalar()
                    && iter->second["to"]["sock"].IsScalar()
                    && iter->second["from"]["type"].IsDefined())) {
                return S_ERROR(parser::PARSE_ERROR);
            }

            const struct node to_node { iter->second["to"]["id"].as<uint32_t>(),
                str_to_node(iter->second["to"]["type"].as<std::string>()) };
            const sockid to_sock = iter->second["to"]["sock"].as<sockid>();
            const struct node from_node {
                iter->second["from"]["id"].as<uint32_t>(),
                str_to_node(iter->second["from"]["type"].as<std::string>())
            };
            const sockid from_sock = iter->second["from"]["sock"].as<sockid>();

            if (get_base(from_node) == nullptr
                || get_base(to_node) == nullptr) {
                lcs_assert(0 && "fucky vucky");
                return S_ERROR(parser::NODE_NOT_FOUND);
            }

            if (!connect_with_id(id, to_node, to_sock, from_node, from_sock)) {
                return S_ERROR(parser::REL_CONNECT_ERROR);
            }
            if (last_rel < id) { last_rel = id; }
        }
    }
    return S_ERROR(parser::OK);
}

} // namespace lcs
