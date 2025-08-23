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

#include <array>
#include <bitset>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include "common.h"

namespace lcs {

class Scene;
/** id type for socket, sock_t = 0 means disconnected */
using sockid = uint8_t;

/** Relationship identifier. Id is a non-zero identifier. */
using relid = uint32_t;

/**
 * Node is a handler that represents the index. Together with the type,
 * represents a unique node.
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

    Node(uint16_t _id = UINT16_MAX, Node::Type _type = GATE)
        : index { _id }
        , type { _type }
    {
    }

    Node(Node&&)                 = default;
    Node(const Node&)            = default;
    Node& operator=(Node&&)      = default;
    Node& operator=(const Node&) = default;

    bool operator<(const Node& n) const { return this->index < n.index; }

    inline uint32_t numeric(void) const { return index | (type << 16); }

    uint16_t index : 16;
    Type type : 4;
};

/**
 * ┌────────────── 16 bits ────────────────┐
 * │ node.index (bits 0‑15)                │
 * ├─────────────────── 4 bits ────────────┤
 * │ node.type  (bits 16‑19)               │
 * ├─────────────────── 8 bits ────────────┤
 * │ sock       (bits 20‑27)               │
 * ├─────────────────── 1 bit  ────────────┤
 * │ is_out     (bit  29)                  │
 * └─────────────────────── remaining bits ┘
 *
 * Note: bit 30 stays unused – it can be repurposed later if needed.
 */
int encode_pair(Node node, sockid sock, bool is_out);
Node decode_pair(int pair_code, sockid* sock = nullptr, bool* is_out = nullptr);

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
    Point(int16_t _x = 0, int16_t _y = 0)
        : x { _x }
        , y { _y } { };

    int16_t x;
    int16_t y;
};

class BaseNode {
public:
    BaseNode(Scene* scene, Point _p = {})
        : point { _p }
        , _parent { scene }
    {
    }

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
    Rel(relid _id, Node _from_node, Node _to_node, sockid _from_sock,
        sockid _to_sock)
        : id { _id }
        , from_node { _from_node }
        , to_node { _to_node }
        , from_sock { _from_sock }
        , to_sock { _to_sock }
        , value { DISABLED }
    {
    }

    Rel()
        : id { 0 }
        , from_sock { 0 }
        , to_sock { 0 }
        , value { DISABLED }
    {
    }

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
class Gate final : public BaseNode {
public:
    enum Type : uint8_t { NOT, AND, OR, XOR, NAND, NOR, XNOR };
    explicit Gate(Scene*, Type type = Type::AND, sockid max_in = 2);
    Gate(const Gate&)            = default;
    Gate(Gate&&)                 = default;
    Gate& operator=(Gate&&)      = default;
    Gate& operator=(const Gate&) = default;
    ~Gate()                      = default;

    /** Get gate type. */
    Type type(void) const { return _type; };
    /** Adds a new input socket. */
    bool increment(void);
    /** Removes an input socket. */
    bool decrement(void);

    /* BaseNode */
    virtual bool is_connected(void) const override;
    virtual State get(sockid slot = 0) const override;
    virtual void on_signal(void) override;
    virtual void clean(void) override;

    /** max_in number of inputs, denoted with relation id */
    std::vector<relid> inputs;
    /** Connected nodes to a single output. */
    std::vector<relid> output;

private:
    /** Gate specific calculation function */
    Type _type;
    State _value;
};

class Component final : public BaseNode {
public:
    explicit Component(Scene*);
    Component(const Component&)            = default;
    Component(Component&&)                 = default;
    Component& operator=(Component&&)      = default;
    Component& operator=(const Component&) = default;
    ~Component()                           = default;

    /** Assigns configurations of given component to this node. */
    LCS_ERROR set_component(uint8_t dep_idx);

    /* BaseNode */
    virtual void on_signal(void) override;
    virtual bool is_connected(void) const override;
    virtual State get(sockid slot = 0) const override;
    virtual void clean(void) override;

    std::vector<relid> inputs;
    std::map<sockid, std::vector<relid>> outputs;
    uint8_t dep_idx;

private:
    uint64_t _output_value;
};

/**
 * An input node where it's value can be arbitrarily changed. If
 * Input::_freq is defined, it will be toggled automatically.
 */
class Input final : public BaseNode {
public:
    enum Type : uint8_t { DEFAULT, TIMER };
    Input(Scene* _scene, uint8_t freq = 0)
        : BaseNode { _scene }
        , _freq { freq }
    {
    }

    Input(const Input&)            = default;
    Input(Input&&)                 = default;
    Input& operator=(Input&&)      = default;
    Input& operator=(const Input&) = default;
    ~Input()                       = default;

    bool is_timer(void) const { return _freq != 0; }
    /** Set the value, and notify connected nodes.
     * @param value to set
     **/
    void set(bool value);
    /** Inverts the value and notifies connected nodes. */
    void toggle(void);

    /* BaseNode */
    virtual void on_signal(void) override;
    virtual bool is_connected(void) const override;
    virtual State get(sockid slot = 0) const override;
    virtual void clean(void) override;

    std::vector<relid> output;

    /** _freq is a non-zero value for timers.
     * On an active scene Scene::run_timers is 10 times per
     * second.
     *
     * * _freq = 1   once every 10 seconds.
     * * _freq = 10  once per second.
     * * _freq = 20  twice per second.
     */
    uint8_t _freq;

private:
    bool _value = false;
};

/** An output node that displays the result */
class Output final : public BaseNode {
public:
    explicit Output(Scene* _scene)
        : BaseNode { _scene }
    {
    }

    Output(const Output&)            = default;
    Output(Output&&)                 = default;
    Output& operator=(Output&&)      = default;
    Output& operator=(const Output&) = default;
    ~Output()                        = default;

    /* BaseNode */
    virtual void on_signal(void) override;
    virtual bool is_connected(void) const override;
    virtual State get(sockid slot = 0) const override;
    virtual void clean(void) override;

    relid input = 0;

private:
    State _value = State::DISABLED;
};

/**
 * A component scene contains the ComponentContext, Component Context can
 * execute a scene with given parameters.
 * NOTE: ComponentContext internally pushes ids by one to run shift
 * calculations correctly. However to be compatible with Component it is
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
     * @param frame_s time to calculate. Intended to inherit parent scene's
     * time.
     * @returns binary encoded result
     */
    uint64_t run(uint64_t input, size_t frame_s = 0);

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
    if constexpr (std::is_same<T, Gate>::value) {
        return Node::Type::GATE;
    }
    if constexpr (std::is_same<T, Component>::value) {
        return Node::Type::COMPONENT;
    }
    if constexpr (std::is_same<T, Input>::value) {
        return Node::Type::INPUT;
    }
    if constexpr (std::is_same<T, Output>::value) {
        return Node::Type::OUTPUT;
    }
    return Node::Type::NODE_S;
}

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

    /** Explicit copy function. */
    void clone(const Scene&);

    /**
     * Run a frame for the scene.
     * @param delta time elapsed since last frame, in seconds.
     */
    void run(float delta);

    /** Creates a node in a scene with given type. Passes arguments to
     * the constructor similar to emplace methods.
     * @param args to pass
     * @returns node identifier of newly created node.
     */
    template <class T, class... Args> Node add_node(Args&&... args)
    {
        Node id;
        constexpr Node::Type node_type = as_node_type<T>();
        std::vector<T>& vec            = vector<T>();
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
        return id;
    }

    /**
     * Safely removes given node from the scene.
     *
     * @param id node to remove
     */
    void remove_node(Node id);

    /**
     * Obtain a temporary node reference from the scene.
     *
     * @param id non-zero node id
     * @returns T* | nullptr
     */
    template <typename T> NRef<T> get_node(Node id)
    {
        std::vector<T>& vec = vector<T>();
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
     *
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
    connect_with_id(relid id, Node to_node, sockid to_sock, Node from_node,
        sockid from_sock = 0);

    /**
     * Safely disconnects a node relationship.
     * @param id relid to disconnect
     * @returns Error on failure:
     *
     * - Error::INVALID_RELID
     * - Error::REL_NOT_FOUND
     * - Error::NOT_CONNECTED
     */
    Error disconnect(relid id);

    /**
     * Trigger a signal for the given relation while updating it's value.
     * @param id relationship id
     * @param value to set
     */
    void signal(relid id, State value);

    /** Get a reference to the desired Node::Type vector. */
    template <class T> constexpr std::vector<T>& vector()
    {
        if constexpr (std::is_same<T, Gate>()) {
            return _gates;
        } else if constexpr (std::is_same<T, Component>()) {
            return _components;
        } else if constexpr (std::is_same<T, Input>()) {
            return _inputs;
        } else if constexpr (std::is_same<T, Output>()) {
            return _outputs;
        }
    }

    /**
     * Serializes given scene.
     * @param buffer to write into
     * @returns Error on failure
     */
    LCS_ERROR write_to(std::vector<uint8_t>& buffer) const;

    /**
     * Deserializes given scene.
     * @param buffer to read from
     * @returns Error on failure
     */
    LCS_ERROR read_from(const std::vector<uint8_t>& buffer);

    /** Returns a dependency string. */
    std::string to_dependency(void) const;

    std::array<char, 128> name {};
    std::array<char, 512> description {};
    /** GitHub user names are limited to 40 characters. */
    std::array<char, 60> author {}; //
    int version;
    std::optional<ComponentContext> component_context;

    std::vector<Gate> _gates;
    std::vector<Component> _components;
    std::vector<Input> _inputs;
    std::vector<Output> _outputs;
    std::map<relid, Rel> _relations;
    /** Delta counter in seconds. */
    float frame_s;

    Node _last_node[Node::Type::NODE_S];
    relid _last_rel;
    Scene* _parent = nullptr;

    void add_dependency(Scene&& scene)
    {
        _dependencies.emplace_back(std::move(scene));
        _dependencies.back()._parent = this;
    }

    void remove_dependency(size_t idx)
    {
        size_t i = 0;
        while (i < _components.size()) {
            if (_components[i].dep_idx == idx) {
                remove_node(i);
            } else {
                i++;
            }
        }
    }

    inline uint64_t run_dependency(size_t idx, uint64_t input)
    {
        return _dependencies[idx].component_context->run(input, frame_s);
    }

    inline const std::vector<Scene>& dependencies(void) const
    {
        return _dependencies;
    }

private:
    std::vector<Scene> _dependencies;
    /** The helper method for move constructor and move assignment */
    void _move_from(Scene&&);
};

namespace tabs {
    /**
     * Creates an empty scene with given name
     * @param name Scene name
     * @param author Scene author
     * @param description Scene description
     * @param version Scene version
     * @returns scene index
     */
    size_t create(const std::string& name, const std::string& author,
        const std::string& description, int version);

    /**
     * Reads scene data from given file.
     * @param path to file
     * @param idx on successful scene_open calls idx will be updated to a
     * valid idx to access scene.
     * @returns Error on failure:
     *
     * - Error::NOT_FOUND
     * - deserialize
     */
    LCS_ERROR open(const std::filesystem::path& path, size_t& idx);

    /**
     * Closes the scene with selected path, erasing from memory.
     * @param idx index of the scene, active scene if not provided
     * @returns Error::OK
     */
    LCS_ERROR close(size_t idx = SIZE_MAX);

    /**
     * Updates the contents of given scene.
     * @param idx index of the scene, active scene if not provided
     * @returns Error on failure:
     *
     * - Error::NO_SAVE_PATH_DEFINED
     */
    LCS_ERROR save(size_t idx = SIZE_MAX);

    /**
     * Updates the contents of given scene.
     * @param new_path to save
     * @param idx index of the scene, active scene if not provided
     * @returns Error on failure:
     *
     * - Error::NO_SAVE_PATH_DEFINED
     */
    LCS_ERROR save_as(
        const std::filesystem::path& new_path, size_t idx = SIZE_MAX);

    /**
     * Alerts the changes in a scene.
     * @param idx to update
     */
    void notify(size_t idx = SIZE_MAX);
    /**
     * Selects a scene as current.
     * @param idx to select
     * @returns reference to scene
     */
    NRef<Scene> active(size_t idx = SIZE_MAX);

    bool is_saved(size_t idx = SIZE_MAX);

    /**
     * Iterate over each active scene.
     * @param run method to execute.
     *       - name: name of the scene
     *       - path: path of the scene
     *       - is_saved: whether the scene is saved.
     *       - is_active: [Only in UI] If the scene is selected..
     *
     */
    void for_each(std::function<bool(std::string_view name,
            const std::filesystem::path& path, bool is_saved, bool is_active)>
            run);

    /**
     * Get if there are any changes that have been done to the scene
     * outside the Node editor. Node editor uses this method to update
     * itself.
     *
     * NOTE: Has changes assumes, after calling this method required changes
     * are made. So calling this method again would return always false.
     *
     * @returns whether there are changes within the scene
     */
    bool is_changed(void);
} // namespace tabs

/**
 * Loads the given component. If the component does not
 * exist in file system attempts to pull it from an available mirror.
 * @param name component name
 * @param scene to update
 * @returns Error on failure:
 *
 * - Error::NOT_A_COMPONENT
 * - Error::INVALID_DEPENDENCY_FORMAT
 * - Error::COMPONENT_NOT_FOUND
 * - Error::NOT_A_COMPONENT
 * - deserialize
 * - net::get_request
 */
Error load_dependency(const std::string& name, Scene& scene);

} // namespace lcs
