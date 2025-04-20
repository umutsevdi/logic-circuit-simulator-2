#pragma once
/*******************************************************************************
 * \file
 * File: engine/engine.h
 * Created: 01/18/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "common.h"
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace lcs {
namespace sys {

    enum error_t {
        OK,
        COMPONENT_NOT_FOUND

    };

    typedef uint32_t component_handle_t;

    struct Metadata {
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
        std::vector<sys::component_handle_t> dependencies;

        std::string to_dependency_string() const;
    };

    /**
     * Attempts to obtain a component. If it hasn't been loaded yet, also
     * loads the component and returns it's handle.
     *
     * @param name - component name
     * @param id id to assign the handle
     * @returns - error on failure.
     *
     */
    error_t get_component(const std::string& name, component_handle_t& id);

    /**
     * Executes the component with given id with provided input, returning
     * it's result.
     *
     * @param id - component handle
     * @param input - binary encoded input value
     * @returns - binary encoded result
     */
    uint64_t run_component(component_handle_t id, uint64_t input);

    std::optional<Metadata> get_dependency_info(component_handle_t id);

} // namespace sys

enum error_t {
    OK,
    INVALID_NODEID,
    INVALID_RELID,
    REL_NOT_FOUND,
    INVALID_FROM_TYPE,
    NOT_A_COMPONENT,
    INVALID_TO_TYPE,
    ALREADY_CONNECTED,
    NOT_CONNECTED,

    COMPONENT_NOT_FOUND

};

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
struct node {
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

struct point_t {
    int x;
    int y;
};

class Scene;

class BaseNode {
public:
    explicit BaseNode(Scene*, node, direction_t _dir = direction_t::RIGHT,
        point_t _p = { 0, 0 });
    BaseNode(const BaseNode&)            = default;
    BaseNode(BaseNode&&)                 = default;
    BaseNode& operator=(BaseNode&&)      = default;
    BaseNode& operator=(const BaseNode&) = default;
    virtual ~BaseNode()                  = default;
    BaseNode* get_base() { return dynamic_cast<BaseNode*>(this); };

    /**
     * Updates the internal data and send out signals to all connected nodes.
     */
    virtual void signal() = 0;
    /** Returns whether all nodes are connected */
    virtual bool is_connected() const = 0;
    /** Get the current status */
    virtual state_t get() = 0;

    inline node get_node() const { return id; }

    node id;
    direction_t dir;
    point_t point;

protected:
    Scene* scene;
};

/**
 * Describes a relation between two nodes.
 *
 * NOTE: from_sock is set to non-zero only when it
 * the connected output is a Component, since components
 * can have multiple output sockets.
 */
struct Rel {
    explicit Rel(relid _id, node _from_node, node _to_node, sockid _from_sock,
        sockid _to_sock);
    Rel() { }
    ~Rel() = default;
    friend std::ostream& operator<<(std::ostream& os, const Rel& r);

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

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    /** max_in number of inputs, denoted with relation id */
    std::vector<relid> inputs;
    /** Connected nodes to a single output. */
    std::vector<relid> output;

    /** Adds a new input socket */
    bool increment();
    /** Removes an input socket */
    bool decrement();

    gate_t type;
    sockid max_in;

private:
    /** Gate specific calculation function */
    bool (*_apply)(const std::vector<bool>&);
    state_t value;
};

class ComponentNode final : public BaseNode {
public:
    explicit ComponentNode(Scene*, node, const std::string& path);
    ComponentNode(const ComponentNode&)            = default;
    ComponentNode(ComponentNode&&)                 = default;
    ComponentNode& operator=(ComponentNode&&)      = default;
    ComponentNode& operator=(const ComponentNode&) = default;
    ~ComponentNode()                               = default;
    friend std::ostream& operator<<(std::ostream& os, const ComponentNode& g);

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    std::map<sockid, std::vector<relid>> outputs;
    std::vector<relid> inputs;

private:
    std::string path;
    sockid max_in;
    sockid max_out;

    bool apply(std::vector<relid>&);
};

/**
 * An input node where it's value can be arbitrarily
 * changed. */
class InputNode : public BaseNode {
public:
    InputNode(Scene* _scene, node _id);
    InputNode(const InputNode&)            = default;
    InputNode(InputNode&&)                 = default;
    InputNode& operator=(InputNode&&)      = default;
    InputNode& operator=(const InputNode&) = default;
    ~InputNode()                           = default;
    friend std::ostream& operator<<(std::ostream& os, const InputNode& g);

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    /**
     * Set the value, and notify connected nodes.
     * @param value - to set
     **/
    void set(bool value);
    void set_freq(uint32_t freq);
    /** Inverts the value and notifies connected nodes. */
    void toggle();

    std::vector<relid> output;

    bool value;
    std::optional<uint32_t> freq;
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

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    relid input;
    state_t value;
};

/**
 * A component scene contains the ComponentContext, Component Context can
 * execute a scene with given parameters.
 */
struct ComponentContext {
    ComponentContext() = default;
    ComponentContext(relid input_s, relid output_s);
    std::map<sockid, std::vector<relid>> inputs;
    std::map<sockid, relid> outputs;

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

private:
    /** Temporarily used input value */
    uint64_t execution_input;
    /** Temporarily used output value */
    uint64_t execution_output;
};

class Scene {
public:
    Scene(const std::string& name = "", const std::string& author = "",
        const std::string& description = "");
    Scene(ComponentContext ctx, const std::string& name = "",
        const std::string& author = "", const std::string& description = "");
    Scene(Scene&&)                 = default;
    Scene(const Scene&)            = default;
    Scene& operator=(Scene&&)      = default;
    Scene& operator=(const Scene&) = default;
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
     * @param ids - to update
     * @param value - to set
     */
    void invoke_signal(const std::vector<relid>& ids, state_t value);

    void set_position(node, point_t);

    error_t connect_with_id(relid& id, node to_node, sockid to_sock,
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

private:
    sys::error_t add_dependency(const std::string& name);
};

/** Class to node_t conversion */
template <typename T> constexpr node_t to_node_type()
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
