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

        std::string to_dependency_string(void) const;
        error_t load_dependencies(void);
        error_t load_dependencies(const std::string& self_dependency);

        /* Serializable interface */
        Json::Value to_json(void) const override;
        error_t from_json(const Json::Value&) override;

        std::string name;
        std::string description;
        std::string author;
        int version;
        std::vector<std::string> dependencies;
    };

    /**
     * Attempts to obtain a component. If it hasn't been loaded yet, also
     * loads the component and returns it's handle.
     *
     * @param name component name
     * @returns - error on failure.
     */
    error_t verify_component(const std::string& name);

    /**
     * Parse a component from given string data, if it's a valid component
     * insert it to _loaded_components
     *
     * @param name to save as
     * @param data to parse
     * @returns - error on failure
     */
    error_t load_component(const std::string name, std::string data);

    /**
     * Executes the component with given id with provided input, returning
     * it's result.
     *
     * @param name component name
     * @param input binary encoded input value
     * @returns - binary encoded result
     */
    uint64_t run_component(const std::string& name, uint64_t input);

    /**
     * Returns a reference to a dependency. Dependency has to be loaded to
     * use this method.
     * @param name of the component
     * @returns Constant reference to a component or nullptr
     */
    NRef<const Scene> get_dependency(const std::string& name);

} // namespace sys

/** id type for socket, sock_t = 0 means disconnected */
typedef uint16_t sockid;

/** Relationship identifier. Id is a non-zero identifier. */
typedef uint32_t relid;

/** Type of a node in a scene. */
enum node_t : uint8_t {
    /** Logic gate. */
    GATE,
    /** A component that has been loaded externally. */
    COMPONENT,
    /** An input node that can be switched manually or by a clock. */
    INPUT,
    /** An output node. */
    OUTPUT,
    /** Input slot of a component. NOTE: Only available for Component Scenes. */
    COMPONENT_INPUT,
    /** Output slot of a component. NOTE: Only available for Component Scenes.
     */
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

    friend std::ostream& operator<<(std::ostream& os, const node& r);
    bool operator<(const node& n) const { return this->id < n.id; }

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    uint32_t id : 24;
    node_t type : 8;
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

/** Direction of the node, defaults to direction_t::RIGHT. */
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

/** Position of a node or a curve in the surface */
struct point_t final : public parse::Serializable {
    point_t(int _x = 0, int _y = 0)
        : x { _x }
        , y { _y } { };

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    int x;
    int y;
};

class BaseNode : public parse::Serializable {
public:
    explicit BaseNode(Scene*, node, direction_t _dir = direction_t::RIGHT,
        point_t _p = { 0, 0 });
    BaseNode(const BaseNode&)            = default;
    BaseNode(BaseNode&&)                 = default;
    BaseNode& operator=(BaseNode&&)      = default;
    BaseNode& operator=(const BaseNode&) = default;
    virtual ~BaseNode()                  = default;
    friend std::ostream& operator<<(std::ostream& os, const BaseNode& r);

    BaseNode* get_base(void) { return dynamic_cast<BaseNode*>(this); };
    inline node id(void) const { return _id; }

    /**
     * Updates the internal data and send out signals to all connected nodes.
     */
    virtual void on_signal(void) = 0;
    /** Returns whether all nodes are connected */
    virtual bool is_connected(void) const = 0;
    /** Get the current status */
    virtual state_t get(sockid slot = 0) const = 0;

    /* Serializable interface */
    Json::Value to_json(void) const override;

    direction_t dir;
    point_t point;

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

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    relid id;
    node from_node;
    node to_node;
    sockid from_sock;
    sockid to_sock;
    state_t value;
    std::vector<point_t> curve_points;
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

    /** Adds a new input socket */
    bool increment(void);
    /** Removes an input socket */
    bool decrement(void);
    inline gate_t type(void) const { return _type; };

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;
    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    /** max_in number of inputs, denoted with relation id */
    std::vector<relid> inputs;
    /** Connected nodes to a single output. */
    std::vector<relid> output;

private:
    /** Gate specific calculation function */
    bool (*_apply)(const std::vector<bool>&);

    gate_t _type;
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

    /** Assigns configurations of given component to this node. */
    error_t set_component(const std::string& path);

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;
    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    std::vector<relid> inputs;
    std::map<sockid, std::vector<relid>> outputs;
    std::string path;

private:
    bool _is_disabled;
    uint64_t _output_value;
};

/**
 * An input node where it's value can be arbitrarily changed. If InputNode::freq
 * is defined, it will be toggled automatically.  */
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

    /** Set the value, and notify connected nodes.
     * @param value to set
     **/
    void set(bool value);
    /** If the InputNode is set up as a timer updates the frequency.
     * @param freq new frequency
     */
    void set_freq(uint32_t freq);
    /** Inverts the value and notifies connected nodes. */
    void toggle(void);

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;
    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    std::vector<relid> output;

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

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    state_t get(sockid slot = 0) const override;
    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    relid input;

private:
    state_t _value;
};

/**
 * A component scene contains the ComponentContext, Component Context can
 * execute a scene with given parameters.
 * NOTE: ComponentContext internally pushes ids by one to run shift
 * calculations correctly. However to be compatible with ComponentNodes it is
 * accessed via get_input(sockid) and get_output(sockid) methods where it is
 * virtually increased by one.
 */
struct ComponentContext final : public parse::Serializable {
    ComponentContext(Scene* parent, sockid input_s = 0, sockid output_s = 0);

    /**
     * Grows or shrinks input or output's size
     * @param input_s new input size
     * @param output_s new output size
     */
    void setup(sockid input_s, sockid output_s);

    /**
     * Execute a scene using the given input.
     * @param input binary encoded input. Starting from the lowest bit
     * values are assigned to each input slot.
     * @returns binary encoded result
     *
     **/
    uint64_t run(uint64_t input);

    /** Get node id for given input socket */
    node get_input(sockid id) const;
    /** Get node id for given output socket */
    node get_output(sockid id) const;
    /** Get value of the given node socket */
    state_t get_value(node id) const;
    /** Update the value of an output slot */
    void set_value(sockid sock, state_t value);

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    std::map<sockid, std::vector<relid>> inputs;
    std::map<sockid, relid> outputs;

private:
    /** Temporarily used input value */
    uint64_t _execution_input;
    /** Temporarily used output value */
    uint64_t _execution_output;
    Scene* _parent;
};

class Scene final : public parse::Serializable {
public:
    Scene(const std::string& name = "", const std::string& author = "",
        const std::string& description = "");
    Scene(ComponentContext ctx, const std::string& name = "",
        const std::string& author = "", const std::string& description = "");
    Scene(Scene&&)                 = default;
    Scene(const Scene&)            = delete;
    Scene& operator=(Scene&&)      = default;
    Scene& operator=(const Scene&) = delete;
    ~Scene()                       = default;
    friend std::ostream& operator<<(std::ostream& os, const Scene&);

    /** Creates a node in a scene with given type. Passes arguments to
     * the constructor similar to emplace methods.
     * @param args to pass
     * @returns node identifier of newly created node.
     */
    template <class T, class... Args> node add_node(Args&&... args)
    {
        constexpr bool is_gate      = std::is_same<T, GateNode>::value;
        constexpr bool is_component = std::is_same<T, ComponentNode>::value;
        constexpr bool is_input     = std::is_same<T, InputNode>::value;
        constexpr bool is_output    = std::is_same<T, OutputNode>::value;

        if constexpr (is_gate) {
            _last_node[node_t::GATE].id++;
            return gates
                .emplace(_last_node[node_t::GATE],
                    GateNode { this, _last_node[node_t::GATE].id, args... })
                .first->first;
        } else if constexpr (is_component) {
            _last_node[node_t::COMPONENT].id++;
            return components
                .emplace(_last_node[node_t::COMPONENT],
                    ComponentNode {
                        this, _last_node[node_t::COMPONENT].id, args... })
                .first->first;
        } else if constexpr (is_input) {
            _last_node[node_t::INPUT].id++;
            return inputs
                .emplace(_last_node[node_t::INPUT],
                    InputNode { this, _last_node[node_t::INPUT].id, args... })
                .first->first;
        } else if constexpr (is_output) {
            _last_node[node_t::OUTPUT].id++;
            return outputs
                .emplace(_last_node[node_t::OUTPUT],
                    OutputNode { this, _last_node[node_t::OUTPUT].id, args... })
                .first->first;
        }
        return { 0, node_t::GATE };
    }

    /** Safely removes given node from the scene.
     * @param id node to remove
     */
    void remove_node(node id);

    /** Obtain a temporary node reference from the scene.
     *
     * @param id non-zero node id
     * @returns T* | nullptr
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
     * Returns a temporary node reference to the abstract class that all nodes
     * implement
     *
     * @param id non-zero node id
     * @returns BaseNode* || nullptr
     */
    NRef<BaseNode> get_base(node id);

    /**
     * Obtain a relationship between nodes by its id.
     * @param id non-zero rel id
     * @returns Relation | nullptr
     */
    NRef<Rel> get_rel(relid id);

    /**
     * Attempts to connect two nodes from their given sockets. Id of such
     * relation will be automatically generated and returned on success.
     *
     * NOTE: Leave from_sock empty if from is not a Component.
     * NOTE: For more detailed constructor refer to Scene::_connect_with_id.
     *
     * @param to_node target node
     * @param to_sock socket of the target node
     * @param from_node source node
     * @param from_sock socket of the source node
     * @returns relid on success, 0 on failure.
     */
    relid connect(
        node to_node, sockid to_sock, node from_node, sockid from_sock = 0);

    /**
     * Safely disconnects a node relationship.
     * @param id relid to disconnect
     * @returns error on failure
     */
    error_t disconnect(relid id);

    /**
     * Trigger a signal for the given relation while updating it's value
     * @param id relationship id
     * @param value to set
     */
    void signal(relid id, state_t value);

    /* Serializable interface */
    Json::Value to_json(void) const override;
    error_t from_json(const Json::Value&) override;

    // [id][period] map for timer inputs
    std::map<node, int> timer_sub_vec;
    std::map<node, GateNode> gates;
    std::map<node, ComponentNode> components;
    std::map<node, InputNode> inputs;
    std::map<node, OutputNode> outputs;
    std::map<relid, Rel> rel;
    sys::Metadata meta;
    std::optional<ComponentContext> component_context;

private:
    error_t _connect_with_id(relid id, node to_node, sockid to_sock,
        node from_node, sockid from_sock = 0);

    node _last_node[node_t::NODE_S];
    relid _last_rel;
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
