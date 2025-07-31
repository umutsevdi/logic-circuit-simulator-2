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
#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace lcs {
/** id type for socket, sock_t = 0 means disconnected */
typedef uint8_t sockid;

/** Relationship identifier. Id is a non-zero identifier. */
typedef uint32_t relid;

/**
 * Node is a handler that represents the index.
 * id is a non-zero identifier. Together with the type, represents a unique
 * node.
 */
struct Node {
    enum Type : uint8_t {
        /** Logic gate. */
        GATE,
        /** A component that has been loaded externally. */
        COMPONENT,
        /** An input node that can be switched manually or by a clock. */
        INPUT,
        /** An output node. */
        OUTPUT,
        LABEL,
        /** Input slot of a component. NOTE: Only available in Component Scenes.
         */
        COMPONENT_INPUT,
        /** Output slot of a component. NOTE: Only available in Component
           Scenes. */
        COMPONENT_OUTPUT,

        NODE_S
    };
    Node(uint16_t _id = UINT16_MAX, Type _type = GATE);
    Node(Node&&)                 = default;
    Node(const Node&)            = default;
    Node& operator=(Node&&)      = default;
    Node& operator=(const Node&) = default;

    bool operator<(const Node& n) const { return this->index < n.index; }

    inline uint32_t numeric(void) const { return index | (type << 16); }

    uint16_t index : 16;
    Type type : 4;
};
template <> const char* to_str<Node::Type>(Node::Type s)
{
    switch (s) {
    case Node::Type::GATE: return "Gate";
    case Node::Type::COMPONENT: return "Component";
    case Node::Type::INPUT: return "Input";
    case Node::Type::OUTPUT: return "Output";
    case Node::Type::COMPONENT_INPUT: return "Component Input";
    case Node::Type::COMPONENT_OUTPUT: return "Component Output";
    default: return "Unknown";
    }
}

int encode_pair(Node node, sockid sock, bool is_out);
Node decode_pair(int pair_code, sockid* sock = nullptr, bool* is_out = nullptr);

class Scene;

enum State {
    /** Socket evaluated to false. */
    FALSE,
    /** Socket evaluated to true. */
    TRUE,
    /** The socket provides no valuable information
     * due to the disconnection of at least one prior socket. */
    DISABLED,
};

/** Position of a node or a curve in the surface */
struct Point {
    Point(int _x = 0, int _y = 0)
        : x { _x }
        , y { _y } { };

    int x;
    int y;
};

class BaseNode {
public:
    explicit BaseNode(Scene*, Point _p = { 0, 0 });
    BaseNode(const BaseNode&)            = default;
    BaseNode(BaseNode&&)                 = default;
    BaseNode& operator=(BaseNode&&)      = default;
    BaseNode& operator=(const BaseNode&) = default;
    virtual ~BaseNode()                  = default;

    inline BaseNode* base(void) { return dynamic_cast<BaseNode*>(this); };
    inline void reload(Scene* s) { _parent = s; }
    inline bool is_null() const { return _parent == nullptr; }
    inline void set_null() { _parent = nullptr; }

    /** Returns whether all nodes are connected */
    bool virtual is_connected(void) const = 0;
    /** Get the current status */
    State virtual get(sockid slot = 0) const = 0;
    /** Executes the node and sends computed value to all connected nodes. */
    void virtual on_signal(void) = 0;
    /** Disconnects from all connected nodes and invalidates its Node id. */
    virtual void clean(void) = 0;

    Point point;

protected:
    Scene* _parent;
};

/**
 * Describes a relation between two nodes.
 *
 * NOTE: from_sock is set to non-zero only when it
 * the connected output is a Component, since components
 * can have multiple output sockets.
 */
struct Rel {
    explicit Rel(relid _id, Node _from_node, Node _to_node, sockid _from_sock,
        sockid _to_sock);
    Rel();
    ~Rel() = default;

    relid id;
    Node from_node;
    Node to_node;
    sockid from_sock;
    sockid to_sock;
    State value;
};

/**
 * Describes a single logic gate.
 */
class GateNode final : public BaseNode {
public:
    enum Type : uint8_t { NOT, AND, OR, XOR, NAND, NOR, XNOR };
    explicit GateNode(Scene*, Type type = Type::AND, sockid max_in = 2);
    GateNode(const GateNode&)            = default;
    GateNode(GateNode&&)                 = default;
    GateNode& operator=(GateNode&&)      = default;
    GateNode& operator=(const GateNode&) = default;
    ~GateNode()                          = default;

    /** Adds a new input socket */
    bool increment(void);
    /** Removes an input socket */
    bool decrement(void);
    inline Type type(void) const { return _type; };

    /* BaseNode */
    bool is_connected(void) const override;
    State get(sockid slot = 0) const override;
    void on_signal(void) override;
    void clean(void) override;

    /** max_in number of inputs, denoted with relation id */
    std::vector<relid> inputs;
    /** Connected nodes to a single output. */
    std::vector<relid> output;

private:
    /** Gate specific calculation function */
    bool (*_apply)(const std::vector<bool>&);

    Type _type;
    State _value;
};

class ComponentNode final : public BaseNode {
public:
    explicit ComponentNode(Scene*, const std::string& path = "");
    ComponentNode(const ComponentNode&)            = default;
    ComponentNode(ComponentNode&&)                 = default;
    ComponentNode& operator=(ComponentNode&&)      = default;
    ComponentNode& operator=(const ComponentNode&) = default;
    ~ComponentNode()                               = default;

    /** Assigns configurations of given component to this node. */
    LCS_ERROR set_component(const std::string& path);

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    State get(sockid slot = 0) const override;
    void clean(void) override;

    std::vector<relid> inputs;
    std::map<sockid, std::vector<relid>> outputs;
    std::string path;

private:
    uint64_t _output_value;
};

/**
 * An input node where it's value can be arbitrarily changed. If
 * InputNode::_freq is defined, it will be toggled automatically.
 */
class InputNode final : public BaseNode {
public:
    enum Type : uint8_t { DEFAULT, TIMER };
    InputNode(Scene* _scene, std::optional<float> freq = std::nullopt);
    InputNode(const InputNode&)            = default;
    InputNode(InputNode&&)                 = default;
    InputNode& operator=(InputNode&&)      = default;
    InputNode& operator=(const InputNode&) = default;
    ~InputNode()                           = default;

    bool is_timer(void) const { return _freq.has_value(); }
    /** Set the value, and notify connected nodes.
     * @param value to set
     **/
    void set(bool value);
    /** Inverts the value and notifies connected nodes. */
    void toggle(void);

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    State get(sockid slot = 0) const override;
    void clean(void) override;

    std::vector<relid> output;

    struct TimeData {
        Node node;
        float freq;
        float current;

        bool operator<(const TimeData& other) const
        {
            return node.index < other.node.index;
        }
    };
    std::optional<float> _freq;

private:
    bool _value;
};

/** An output node that displays the result */
class OutputNode final : public BaseNode {
public:
    enum Type : uint8_t { SINGLE, SEG7 };
    OutputNode(Scene* _scene, Type variant = Type::SINGLE);
    OutputNode(const OutputNode&)            = default;
    OutputNode(OutputNode&&)                 = default;
    OutputNode& operator=(OutputNode&&)      = default;
    OutputNode& operator=(const OutputNode&) = default;
    ~OutputNode()                            = default;

    /* BaseNode */
    void on_signal(void) override;
    bool is_connected(void) const override;
    State get(sockid slot = 0) const override;
    void clean(void) override;

    relid input;

private:
    State value;
};

/**
 * A component scene contains the ComponentContext, Component Context can
 * execute a scene with given parameters.
 * NOTE: ComponentContext internally pushes ids by one to run shift
 * calculations correctly. However to be compatible with ComponentNodes it is
 * accessed via ComponentContext::get_input(sockid) and
 * ComponentContext::get_output(sockid) methods where it is
 * virtually increased by one.
 */
struct ComponentContext {
    ComponentContext(Scene* parent, sockid input_s = 0, sockid output_s = 0);

    inline void reload(Scene* parent) { _parent = parent; }

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
     */
    uint64_t run(uint64_t input);

    /**
     * Execute a scene using the existing state.
     * @returns binary encoded result
     */
    uint64_t run();

    /** Get node id for given input socket */
    Node get_input(sockid id) const;

    /** Get node id for given output socket */
    Node get_output(sockid id) const;

    /** Get value of the given node socket */
    State get_value(Node id) const;

    /** Update the value of an output slot */
    void set_value(Node id, State value);

    std::vector<std::vector<relid>> inputs;
    std::vector<relid> outputs;

private:
    /** Temporarily used input value */
    std::bitset<64> _execution_input;
    /** Temporarily used output value */
    std::bitset<64> _execution_output;
    Scene* _parent;
};

/** Class to Node::Type conversion */
template <typename T> constexpr Node::Type as_node_type(void)
{
    if constexpr (std::is_same<T, GateNode>::value) {
        return Node::Type::GATE;
    }
    if constexpr (std::is_same<T, ComponentNode>::value) {
        return Node::Type::COMPONENT;
    }
    if constexpr (std::is_same<T, InputNode>::value) {
        return Node::Type::INPUT;
    }
    if constexpr (std::is_same<T, OutputNode>::value) {
        return Node::Type::OUTPUT;
    }
    return Node::Type::NODE_S;
}

struct TimeData {
    Node node;
    float freq;
    float current;

    bool operator<(const TimeData& other) const
    {
        return node.index < other.node.index;
    }
};

class Scene {
public:
    Scene(const std::string& name = "", const std::string& author = "",
        const std::string& description = "", int _version = 1);
    Scene(ComponentContext ctx, const std::string& name = "",
        const std::string& author = "", const std::string& description = "",
        int _version = 1);
    Scene(Scene&&);
    Scene& operator=(Scene&&);
    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
    ~Scene()                       = default;

    /** Run a frame for the scene. */
    void run_timers(void);

    /** Creates a node in a scene with given type. Passes arguments to
     * the constructor similar to emplace methods.
     * @param args to pass
     * @returns node identifier of newly created node.
     */
    template <class T, class... Args> Node add_node(Args&&... args)
    {
        Node id;
        constexpr Node::Type node_type = as_node_type<T>();
        std::vector<T>& vec            = _get_vec<T>();
        id = { _last_node[node_type].index, node_type };
        if (id.index == vec.size()) {
            vec.emplace_back(this, args...);
            _last_node[node_type].index = vec.size();
        } else {
            vec[id.index] = T { this, args... };
            // Start from next item and find next empty spot
            size_t i = _last_node[node_type].index + 1;
            while (i < vec.size()) {
                if (vec[i].is_null()) {
                    break;
                }
                i++;
            }
            // If an empty spot is found i will store its' index. Else
            // it will be equal to the size of vector.
            _last_node[node_type].index = i;
            L_DEBUG("Last node found at %zu/%zu for %s", i, vec.size(),
                to_str<Node::Type>(node_type));
        }
        if constexpr (std::is_same<T, InputNode>()) {
            // FIXME add timer if timer
            get_first<float>(args...);
            //          if (_freq.has_value()) {
            //              _timerlist.emplace(id, 0);
            //          }
        }

        return id;
    }

    /** Safely removes given node from the scene.
     * @param id node to remove
     */
    void remove_node(Node id);

    /** Obtain a temporary node reference from the scene.
     *
     * @param id non-zero node id
     * @returns T* | nullptr
     */
    template <typename T> NRef<T> get_node(Node id)
    {
        std::vector<T>& vec = _get_vec<T>();
        lcs_assert(id.type == as_node_type<T>());
        return id.index < vec.size() ? &vec[id.index] : nullptr;
    }

    /**
     * Returns a temporary node reference to the abstract class that all nodes
     * implement
     *
     * @param id non-zero node id
     * @returns BaseNode* || nullptr
     */
    NRef<BaseNode> get_base(Node id);

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
        Node to_node, sockid to_sock, Node from_node, sockid from_sock = 0);

    /**
     * Safely disconnects a node relationship.
     * @param id relid to disconnect
     * @returns Error on failure:
     *
     * - Error::INVALID_RELID
     * - Error::REL_NOT_FOUND
     * - Error::NOT_CONNECTED
     */
    LCS_ERROR disconnect(relid id);

    /**
     * Trigger a signal for the given relation while updating it's value.
     * @param id relationship id
     * @param value to set
     */
    void signal(relid id, State value);

    /** Returns a dependency string. */
    std::string to_dependency(void) const;
    /** Returns file path to save. */
    std::string to_filepath(void) const;
    /**
     * Loads dependencies according to the internal list.
     * @returns Error on failure:
     *
     * - io::component::fetch
     *
     */
    LCS_ERROR load_dependencies(void);

    std::array<char, 128> name;
    std::array<char, 512> description;
    /** GitHub user names are limited to 40 characters. */
    std::array<char, 60> author; //
    int version;
    std::vector<std::string> dependencies;
    std::optional<ComponentContext> component_context;

    std::vector<GateNode> _gates;
    std::vector<ComponentNode> _components;
    std::vector<InputNode> _inputs;
    std::vector<OutputNode> _outputs;
    std::map<relid, Rel> _relations;
    /** key = Node::id, value: internal clock counter */
    std::set<TimeData> _timerlist;

    /**
     * Attempts to connect two nodes from their given sockets. On success
     * relationship with given id is inserted.
     *
     * NOTE: Leave from_sock empty if from is not a Component.
     *
     * @param id of relationship
     * @param to_node target node
     * @param to_sock socket of the target node
     * @param from_node source node
     * @param from_sock socket of the source node
     * @returns Error on failure:
     *
     * - Error::INVALID_FROM_TYPE
     * - Error::INVALID_NODEID
     * - Error::NOT_A_COMPONENT
     * - Error::ALREADY_CONNECTED
     * - Error::INVALID_TO_TYPE
     */
    LCS_ERROR
    _connect_with_id(relid id, Node to_node, sockid to_sock, Node from_node,
        sockid from_sock = 0);

    Node _last_node[Node::Type::NODE_S];
    relid _last_rel;

private:
    /** The helper method for move constructor and move assignment */
    void _move_from(Scene&&);

    template <class T> inline std::vector<T>& _get_vec()
    {
        constexpr bool is_gate      = std::is_same<T, GateNode>::value;
        constexpr bool is_component = std::is_same<T, ComponentNode>::value;
        constexpr bool is_input     = std::is_same<T, InputNode>::value;
        constexpr bool is_output    = std::is_same<T, OutputNode>::value;

        if constexpr (is_gate) {
            return _gates;
        } else if constexpr (is_component) {
            return _components;
        } else if constexpr (is_input) {
            return _inputs;
        } else if constexpr (is_output) {
            return _outputs;
        }
    }
};

/**
 * Serializes given scene.
 * @param scene to serialize
 * @param buffer to write into
 * @returns Error on failure
 *
 */
Error serialize(const Scene& scene, std::vector<uint8_t>& buffer);

/**
 * Deserializes given scene.
 * @param buffer to read from
 * @param scene to write into
 * @returns Error on failure
 */
Error deserialize(const std::vector<uint8_t>& buffer, Scene& scene);

template <> const char* to_str<State>(State s)
{
    switch (s) {
    case State::TRUE: return "TRUE";
    case State::FALSE: return "FALSE";
    default: return "DISABLED";
    }
}
template <> const char* to_str<GateNode::Type>(GateNode::Type s)
{
    switch (s) {
    case GateNode::Type::NOT: return "NOT";
    case GateNode::Type::AND: return "AND";
    case GateNode::Type::OR: return "OR";
    case GateNode::Type::XOR: return "XOR";
    case GateNode::Type::NAND: return "NAND";
    case GateNode::Type::NOR: return "NOR";
    case GateNode::Type::XNOR: return "XNOR";
    default: return "null";
    }
}
namespace io {

}

} // namespace lcs
