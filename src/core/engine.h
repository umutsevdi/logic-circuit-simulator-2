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

#include "common/common.h"
#include "types.h"
#include <map>
#include <vector>

namespace lcs {
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
class Gate final : public BaseNode {
public:
    explicit Gate(Scene*, node, gate_t type, sockid max_in = 2);
    Gate(const Gate&)            = default;
    Gate(Gate&&)                 = default;
    Gate& operator=(Gate&&)      = default;
    Gate& operator=(const Gate&) = default;
    ~Gate()                      = default;

    friend std::ostream& operator<<(std::ostream& os, const Gate& g);

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    /** max_in number of inputs, denoted with relation id */
    std::vector<relid> inputs;
    /** Connected nodes to a single output. */
    std::vector<relid> output;

    /** Adds a new input socket */
    void increment();
    /** Removes an input socket */
    void decrement();

    gate_t type;
    sockid max_in;

private:
    /** Gate specific calculation function */
    bool (*_apply)(const std::vector<bool>&);
    state_t value;
};

class Component final : public BaseNode {
public:
    explicit Component(Scene*, node, const std::string& path);
    Component(const Component&)            = default;
    Component(Component&&)                 = default;
    Component& operator=(Component&&)      = default;
    Component& operator=(const Component&) = default;
    ~Component()                           = default;
    friend std::ostream& operator<<(std::ostream& os, const Component& g);

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
class Input final : public BaseNode {
public:
    Input(Scene* _scene, node _id);
    Input(const Input&)            = default;
    Input(Input&&)                 = default;
    Input& operator=(Input&&)      = default;
    Input& operator=(const Input&) = default;
    ~Input()                       = default;
    friend std::ostream& operator<<(std::ostream& os, const Input& g);

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    /**
     * Set the value, and notify connected nodes.
     * @param value - to set
     **/
    void set(bool value);
    /** Inverts the value and notifies connected nodes. */
    void toggle();

    std::vector<relid> output;
    bool value;
};

/** An output node that displays the result */
class Output final : public BaseNode {
public:
    Output(Scene* _scene, node _id);
    Output(const Output&)            = default;
    Output(Output&&)                 = default;
    Output& operator=(Output&&)      = default;
    Output& operator=(const Output&) = default;
    ~Output()                        = default;
    friend std::ostream& operator<<(std::ostream& os, const Output& g);

    void signal() override;
    bool is_connected() const override;
    state_t get() override;

    relid input;
    state_t value;
};

class Scene {
public:
    Scene(const std::string& name = "");
    Scene(Scene&&)                 = default;
    Scene(const Scene&)            = default;
    Scene& operator=(Scene&&)      = default;
    Scene& operator=(const Scene&) = default;
    ~Scene()                       = default;
    friend std::ostream& operator<<(std::ostream& os, const Scene&);

    template <class T, class... Args> node add_node(Args&&... args)
    {
        constexpr bool is_gate      = std::is_same<T, Gate>::value;
        constexpr bool is_component = std::is_same<T, Component>::value;
        constexpr bool is_input     = std::is_same<T, Input>::value;
        constexpr bool is_output    = std::is_same<T, Output>::value;

        if constexpr (is_gate) {
            last_node[node_t::GATE].id++;
            return gates
                .emplace(last_node[node_t::GATE],
                    Gate { this, last_node[node_t::GATE].id, args... })
                .first->first;
        } else if constexpr (is_component) {
            last_node[node_t::COMPONENT].id++;
            return components
                .emplace(last_node[node_t::COMPONENT],
                    Component {
                        this, last_node[node_t::COMPONENT].id, args... })
                .first->first;
        } else if constexpr (is_input) {
            last_node[node_t::INPUT].id++;
            return inputs
                .emplace(last_node[node_t::INPUT],
                    Input { this, last_node[node_t::INPUT].id, args... })
                .first->first;
        } else if constexpr (is_output) {
            last_node[node_t::OUTPUT].id++;
            return outputs
                .emplace(last_node[node_t::OUTPUT],
                    Output { this, last_node[node_t::OUTPUT].id, args... })
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
        constexpr bool is_gate      = std::is_same<T, Gate>::value;
        constexpr bool is_component = std::is_same<T, Component>::value;
        constexpr bool is_input     = std::is_same<T, Input>::value;
        constexpr bool is_output    = std::is_same<T, Output>::value;
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
    void disconnect(relid);

    /**
     * Update a series relationship to the value, and trigger signal for each
     * connected node
     * @param ids - to update
     * @param value - to set
     */
    void invoke_signal(const std::vector<relid>& ids, state_t value);

    void set_position(node, point_t);

    bool connect_with_id(relid& id, node to_node, sockid to_sock,
        node from_node, sockid from_sock = 0) noexcept;

    // [id][period] map for timer inputs
    std::map<node, int> timer_sub_vec;

    std::map<node, Gate> gates;
    std::map<node, Component> components;
    std::map<node, Input> inputs;
    std::map<node, Output> outputs;
    std::map<relid, Rel> rel;
    node last_node[node_t::NODE_S];
    relid last_rel;

    Metadata meta;
};

} // namespace lcs
