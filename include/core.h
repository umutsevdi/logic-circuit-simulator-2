#pragma once
/*******************************************************************************
 * \file
 * File: core.h
 * Created: 01/18/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "common.h"
#include "parse.h"
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace lcs {
class Scene;
namespace sys {

    struct Metadata final : parse::Serializable {
        Metadata(std::string _name, std::string _description,
            std::string _author, int _version = VERSION)
            : name { _name }
            , description { _description }
            , author { _author }
            , version { _version } { };
        Metadata() { };

        std::string name;
        std::string description;
        std::string author;
        int version;
        std::vector<std::string> dependencies;

        std::string to_dependency_string(void) const;
        error_t load_dependencies(void);
        error_t load_dependencies(const std::string& self_dependency);

        /* Serializable interface */
        Json::Value to_json(void) const override;
        error_t from_json(const Json::Value&) override;
    };

    /**
     * Attempts to obtain a component. If it hasn't been loaded yet, also
     * loads the component and returns it's handle.
     *
     * @param name - component name
     * @returns - error on failure.
     */
    error_t verify_component(const std::string& name);

    /**
     * Parse a component from given string data, if it's a valid component
     * insert it to _loaded_components
     *
     * @param name - to save as
     * @param data - to parse
     * @returns - error on failure
     */
    error_t load_component(const std::string name, std::string data);

    /**
     * Executes the component with given id with provided input, returning
     * it's result.
     *
     * @param name - component name
     * @param input - binary encoded input value
     * @returns - binary encoded result
     */
    uint64_t run_component(const std::string& name, uint64_t input);

    NRef<const Scene> get_dependency(const std::string& name);

} // namespace sys

/** id type for socket, sock_t = 0 means disconnected */
typedef uint16_t sockid;

/** Relationship identifier */
typedef uint32_t relid;

enum node_t : uint8_t {
    GATE,
    COMPONENT,
    INPUT,
    OUTPUT,
    COMPONENT_INPUT,
    COMPONENT_OUTPUT,

    NODE_S
};

constexpr const char* node_to_str(node_t s)
{
    switch (s) {
    case node_t::GATE: return "GATE";
    case node_t::COMPONENT: return "COMPONENT";
    case node_t::INPUT: return "INPUT";
    case node_t::OUTPUT: return "OUTPUT";
    case node_t::COMPONENT_INPUT: return "COMPIN";
    case node_t::COMPONENT_OUTPUT: return "COMPOUT";
    default: return "UNKNOWN";
    }
}

node_t str_to_node(const std::string&);

/**
 * Node is a handler that represents the index.
 * id is a non-zero identifier. Together with the type, represents a unique
 * node.
 * */
struct node final : public parse::Serializable {
    node(uint32_t _id = 0, node_t _type = GATE);
    node(node&&)                 = default;
    node(const node&)            = default;
    node& operator=(node&&)      = default;
    node& operator=(const node&) = default;
    /** unique identifier within the same type */
    uint32_t id : 24;
    node_t type : 8;
    friend std::ostream& operator<<(std::ostream& os, const node& r);
    bool operator<(const node& n) const { return this->id < n.id; }

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;
};

enum state_t {
    /** Socket evaluated to false. */
    FALSE,
    /** Socket evaluated to true. */
    TRUE,
    /** The socket provides no valuable information
     * due to the disconnection of at least one prior socket.
     */
    DISABLED,
};
constexpr const char* state_t_str(state_t s)
{
    return (s) == state_t::DISABLED ? "DISABLED"
        : (s) == state_t::TRUE      ? "TRUE"
                                    : "FALSE";
}
state_t str_to_state(const std::string&);

enum direction_t { RIGHT, DOWN, LEFT, UP };
constexpr const char* direction_to_str(direction_t s)
{
    return s == RIGHT ? "r" : s == DOWN ? "d" : s == LEFT ? "l" : "d";
}
direction_t str_to_dir(const std::string&);

enum gate_t {
    NOT,
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    XNOR,

    GATE_S
};
constexpr const char* gate_to_str(gate_t g)
{
    return g == NOT ? "NOT"
        : g == AND  ? "AND"
        : g == OR   ? "OR"
        : g == XOR  ? "XOR"
        : g == NAND ? "NAND"
        : g == NOR  ? "NOR"
        : g == XNOR ? "XNOR"
                    : "NaN";
}
gate_t str_to_gate(const std::string&);

struct point_t final : public parse::Serializable {
    point_t(int _x = 0, int _y = 0)
        : x { _x }
        , y { _y } { };
    int x;
    int y;

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;
};

class Scene;

class BaseNode : public parse::Serializable {
public:
    explicit BaseNode(Scene*, node, direction_t _dir = direction_t::RIGHT,
        point_t _p = { 0, 0 });
    BaseNode(const BaseNode&)            = default;
    BaseNode(BaseNode&&)                 = default;
    BaseNode& operator=(BaseNode&&)      = default;
    BaseNode& operator=(const BaseNode&) = default;
    virtual ~BaseNode()                  = default;
    BaseNode* get_base(void) { return dynamic_cast<BaseNode*>(this); };
    friend std::ostream& operator<<(std::ostream& os, const BaseNode& r);

    inline node id(void) const { return _id; }

    /**
     * Updates the internal data and send out signals to all connected nodes.
     */
    virtual void on_signal(void) = 0;
    /** Returns whether all nodes are connected */
    virtual bool is_connected(void) const = 0;
    /** Get the current status */
    virtual state_t get(sockid slot = 0) const = 0;

    direction_t dir;
    point_t point;

    /* Serializable interface */
    Json::Value to_json(void) const override;

protected:
    Scene* _scene;
    node _id;
};

/**
 * Describes a relation between two nodes.
 *
 * NOTE: from_sock is set to non-zero only when it
 * the connected output is a Component, since components
 * can have multiple output sockets.
 */
struct Rel final : public parse::Serializable {
    explicit Rel(relid _id, node _from_node, node _to_node, sockid _from_sock,
        sockid _to_sock);
    Rel();
    ~Rel() = default;
    friend std::ostream& operator<<(std::ostream& os, const Rel& r);

    relid id;
    node from_node;
    node to_node;
    sockid from_sock;
    sockid to_sock;
    state_t value;
    std::vector<point_t> curve_points;

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;
};

/**
 * Describes a single logic gate.
 */
class GateNode final : public BaseNode {
public:
    explicit GateNode(Scene*, node, gate_t type, sockid max_in = 2);
    GateNode(const GateNode&)            = default;
    GateNode(GateNode&&)                 = default;
    GateNode& operator=(GateNode&&)      = default;
    GateNode& operator=(const GateNode&) = default;
    ~GateNode()                          = default;

    friend std::ostream& operator<<(std::ostream& os, const GateNode& g);

    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;

    /** max_in number of inputs, denoted with relation id */
    std::vector<relid> inputs;
    /** Connected nodes to a single output. */
    std::vector<relid> output;

    /** Adds a new input socket */
    bool increment(void);
    /** Removes an input socket */
    bool decrement(void);

    inline gate_t type(void) const { return _type; };

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

private:
    gate_t _type;
    /** Gate specific calculation function */
    bool (*_apply)(const std::vector<bool>&);
    state_t _value;
    bool _is_disabled;
    sockid _max_in;
};

class ComponentNode final : public BaseNode {
public:
    explicit ComponentNode(Scene*, node, const std::string& path = "");
    ComponentNode(const ComponentNode&)            = default;
    ComponentNode(ComponentNode&&)                 = default;
    ComponentNode& operator=(ComponentNode&&)      = default;
    ComponentNode& operator=(const ComponentNode&) = default;
    ~ComponentNode()                               = default;
    friend std::ostream& operator<<(std::ostream& os, const ComponentNode& g);

    error_t set_component(const std::string& path);

    std::map<sockid, std::vector<relid>> outputs;
    std::vector<relid> inputs;

    std::string path;

    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

private:
    bool _is_disabled;
    uint64_t _output_value;
};

/**
 * An input node where it's value can be arbitrarily
 * changed. */
class InputNode : public BaseNode {
public:
    InputNode(
        Scene* _scene, node _id, std::optional<uint32_t> freq = std::nullopt);
    InputNode(const InputNode&)            = default;
    InputNode(InputNode&&)                 = default;
    InputNode& operator=(InputNode&&)      = default;
    InputNode& operator=(const InputNode&) = default;
    ~InputNode()                           = default;
    friend std::ostream& operator<<(std::ostream& os, const InputNode& g);

    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;

    /**
     * Set the value, and notify connected nodes.
     * @param value - to set
     **/
    void set(bool value);
    void set_freq(uint32_t freq);
    /** Inverts the value and notifies connected nodes. */
    void toggle(void);

    std::vector<relid> output;

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

private:
    bool _value;
    std::optional<uint32_t> _freq;
};

/** An output node that displays the result */
class OutputNode final : public BaseNode {
public:
    OutputNode(Scene* _scene, node _id);
    OutputNode(const OutputNode&)            = default;
    OutputNode(OutputNode&&)                 = default;
    OutputNode& operator=(OutputNode&&)      = default;
    OutputNode& operator=(const OutputNode&) = default;
    ~OutputNode()                            = default;
    friend std::ostream& operator<<(std::ostream& os, const OutputNode& g);

    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;

    relid input;
    state_t value;

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;
};

/**
 * A component scene contains the ComponentContext, Component Context can
 * execute a scene with given parameters.
 */
struct ComponentContext final : public parse::Serializable {
    ComponentContext(sockid input_s = 0, sockid output_s = 0);
    std::map<sockid, std::vector<relid>> inputs;
    std::map<sockid, relid> outputs;

    void setup(sockid input_s, sockid output_s);
    /**
     * Execute a scene using the given input.
     * @param s to run
     * @param input binary encoded input. Starting from the lowest bit
     * values are assigned to each input slot. Before running the method
     * @returns binary encoded result
     *
     **/
    uint64_t run(Scene* s, uint64_t input);
    node get_input(uint32_t id) const;
    node get_output(uint32_t id) const;
    state_t get_value(node id) const;
    void set_value(sockid sock, state_t value);

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

private:
    /** Temporarily used input value */
    uint64_t _execution_input;
    /** Temporarily used output value */
    uint64_t _execution_output;
};

class Scene final : public parse::Serializable {
public:
    Scene(const std::string& name = "", const std::string& author = "",
        const std::string& description = "");
    Scene(ComponentContext ctx, const std::string& name = "",
        const std::string& author = "", const std::string& description = "");
    Scene(Scene&&)                 = default;
    Scene(const Scene&)            = delete;
    Scene& operator=(Scene&&)      = delete;
    Scene& operator=(const Scene&) = delete;
    ~Scene()                       = default;
    friend std::ostream& operator<<(std::ostream& os, const Scene&);

    template <class T, class... Args> node add_node(Args&&... args)
    {
        constexpr bool is_gate      = std::is_same<T, GateNode>::value;
        constexpr bool is_component = std::is_same<T, ComponentNode>::value;
        constexpr bool is_input     = std::is_same<T, InputNode>::value;
        constexpr bool is_output    = std::is_same<T, OutputNode>::value;

        if constexpr (is_gate) {
            last_node[node_t::GATE].id++;
            return gates
                .emplace(last_node[node_t::GATE],
                    GateNode { this, last_node[node_t::GATE].id, args... })
                .first->first;
        } else if constexpr (is_component) {
            last_node[node_t::COMPONENT].id++;
            return components
                .emplace(last_node[node_t::COMPONENT],
                    ComponentNode {
                        this, last_node[node_t::COMPONENT].id, args... })
                .first->first;
        } else if constexpr (is_input) {
            last_node[node_t::INPUT].id++;
            return inputs
                .emplace(last_node[node_t::INPUT],
                    InputNode { this, last_node[node_t::INPUT].id, args... })
                .first->first;
        } else if constexpr (is_output) {
            last_node[node_t::OUTPUT].id++;
            return outputs
                .emplace(last_node[node_t::OUTPUT],
                    OutputNode { this, last_node[node_t::OUTPUT].id, args... })
                .first->first;
        }
        //    case node_t::DISPLAY:
        return { 0, node_t::GATE };
    }
    void remove_node(node);

    /**
     * Obtain a temporary node reference from the scene.
     *
     * @param id - non-zero node id
     * @returns T* || nullptr
     *
     */
    template <typename T> NRef<T> get_node(node id)
    {
        constexpr bool is_gate      = std::is_same<T, GateNode>::value;
        constexpr bool is_component = std::is_same<T, ComponentNode>::value;
        constexpr bool is_input     = std::is_same<T, InputNode>::value;
        constexpr bool is_output    = std::is_same<T, OutputNode>::value;
        if constexpr (is_gate) {
            auto g = gates.find(id);
            lcs_assert(
                id.id != 0 && g != gates.end() && id.type == node_t::GATE);
            return &g->second;
        } else if constexpr (is_component) {
            auto g = components.find(id);
            lcs_assert(id.id != 0 && g != components.end()
                && id.type == node_t::COMPONENT);
            return &g->second;
        } else if constexpr (is_input) {
            auto g = inputs.find(id);
            lcs_assert(
                id.id != 0 && g != inputs.end() && id.type == node_t::INPUT);
            return &g->second;
        } else if constexpr (is_output) {
            auto g = outputs.find(id);
            lcs_assert(
                id.id != 0 && g != outputs.end() && id.type == node_t::OUTPUT);
            return &g->second;
        }
        return nullptr;
    }

    /**
     * Returns a temporary node reference to the abstract class all nodes
     * implement
     *
     * @param id - non-zero node id
     * @returns BaseNode* || nullptr
     *
     */
    NRef<BaseNode> get_base(node id);

    /**
     * Obtain a relationship between nodes by its id.
     * @param id - non-zero rel id
     * @returns T* | nullptr
     *
     */
    NRef<Rel> get_rel(relid id);

    /** Leave from_sock empty if from is not a Component. */
    relid connect(node to_node, sockid to_sock, node from_node,
        sockid from_sock = 0) noexcept;
    error_t disconnect(relid);

    /**
     * Update a series relationship to the value, and trigger signal for each
     * connected node
     * @param id - relationship id
     * @param value - to set
     */
    void signal(relid id, state_t value);

    void set_position(node, point_t);

    error_t connect_with_id(relid id, node to_node, sockid to_sock,
        node from_node, sockid from_sock = 0) noexcept;

    // [id][period] map for timer inputs
    std::map<node, int> timer_sub_vec;
    std::map<node, GateNode> gates;
    std::map<node, ComponentNode> components;
    std::map<node, InputNode> inputs;
    std::map<node, OutputNode> outputs;
    std::map<relid, Rel> rel;
    node last_node[node_t::NODE_S];
    relid last_rel;

    sys::Metadata meta;
    std::optional<ComponentContext> component_context;

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;
};

/** Class to node_t conversion */
template <typename T> constexpr node_t to_node_type(void)
{
    if constexpr (std::is_same<T, GateNode>::value) { return node_t::GATE; }
    if constexpr (std::is_same<T, ComponentNode>::value) {
        return node_t::COMPONENT;
    }
    if constexpr (std::is_same<T, InputNode>::value) { return node_t::INPUT; }
    if constexpr (std::is_same<T, OutputNode>::value) { return node_t::OUTPUT; }
    return node_t::NODE_S;
}

} // namespace lcs
