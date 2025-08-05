#include "common.h"
#include "core.h"
#include <cstring>

#define expect_at_least(BGNPTR, ENDPTR, TYPE)                                  \
    {                                                                          \
        size_t __REM = (ENDPTR - BGNPTR);                                      \
        if (__REM < sizeof(TYPE)) {                                            \
            return ERROR(Error::INCOMPLETE_INSTR);                             \
        }                                                                      \
    }

namespace lcs {
/**
 * Instr define the instructions to rebuild the given scene. Every instruction
 * has a command and a ending character. Instructions are non readable ASCII
 * characters.
 *
 */
enum Instr : uint8_t {
    END = 0x0,
    DEFINE_SCENE,
    /** Path to a package. */
    INCLUDE = 0x9,
    /** Read no more than min(128, buffer_s) characters stopping at NULL byte.
     */
    SET_NAME = 0xA,
    /** Read no more than min(512, buffer_s) characters stopping at NULL byte.
     */
    SET_DESC = 0xB,
    /** Read no more than min(60, buffer_s) characters stopping at NULL byte. */
    SET_AUTHOR = 0xC,
    /** Read version. Fmt: UINT32 version */
    SET_VERSION = 0xD,
    /** Sets the scene as a component. Fmt: UINT32 input_s, UINT32 output_s  */
    SET_COMP = 0xE,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y,
       UINT8 is_timer, (FLOAT32 freq|UINT8 value) */
    ADD_INPUT = 0xF,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y  */
    ADD_OUT = 0x10,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y,
       UINT32 size, UINT32 type  */
    ADD_GATE = 0x11,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y,
     * UINT8 dep_id
     */
    ADD_COMP = 0x12,
    /** Connect sockets of two nodes. Fmt: UINT32 from(packed), UINT32
       to(packed) */
    CONNECT = 0x13,

    /** ADD_T is about to insert a valid Node. */
    NODE_VALUE = 0x14,
};

static void _push_uint(std::vector<uint8_t>& vec, uint32_t value)
{
    value = htonl(value);
    vec.push_back(value);
    vec.push_back(value >> 8);
    vec.push_back(value >> 16);
    vec.push_back(value >> 24);
}

static uint32_t _pop_uint(const uint8_t** cursor, const uint8_t* endptr)
{
    expect_at_least(*cursor, endptr, uint32_t);
    uint32_t result = 0;
    memcpy(&result, *cursor, sizeof(uint32_t));
    *cursor += sizeof(uint32_t);
    return ntohl(result);
}

static void _encode_meta(const Scene& s, std::vector<uint8_t>& buffer)
{
    size_t name_s   = strnlen(s.name.begin(), s.name.size());
    size_t desc_s   = strnlen(s.description.begin(), s.description.size());
    size_t author_s = strnlen(s.author.begin(), s.author.size());
    if (name_s > 0) {
        buffer.push_back(Instr::SET_NAME);
        buffer.insert(buffer.end(), s.name.begin(), s.name.begin() + name_s);
        buffer.push_back(Instr::END);
    }
    if (desc_s > 0) {
        buffer.push_back(Instr::SET_DESC);
        buffer.insert(buffer.end(), s.description.begin(),
            s.description.begin() + desc_s);
        buffer.push_back(Instr::END);
    }
    if (author_s > 0) {
        buffer.push_back(Instr::SET_AUTHOR);
        buffer.insert(
            buffer.end(), s.author.begin(), s.author.begin() + author_s);
        buffer.push_back(Instr::END);
    }
    buffer.push_back(Instr::SET_VERSION);
    _push_uint(buffer, s.version);
    if (s.component_context.has_value()) {
        buffer.push_back(Instr::SET_COMP);
        _push_uint(buffer, s.component_context->inputs.size());
        _push_uint(buffer, s.component_context->outputs.size());
    }
    for (auto& dep : s.dependencies) {
        std::string name = dep.to_dependency();
        L_DEBUG("%s", name.c_str());
        buffer.push_back(Instr::INCLUDE);
        buffer.insert(buffer.end(), name.begin(), name.begin() + name.length());
        buffer.push_back(Instr::END);
    }
}

template <typename T> static constexpr Instr _get_instr(void)
{
    if constexpr (std::is_same<T, GateNode>()) {
        return ADD_GATE;
    }
    if constexpr (std::is_same<T, InputNode>()) {
        return ADD_INPUT;
    }
    if constexpr (std::is_same<T, OutputNode>()) {
        return ADD_OUT;
    }
    if constexpr (std::is_same<T, ComponentNode>()) {
        return ADD_COMP;
    }
}

template <typename T>
static inline void _encode_node(std::vector<uint8_t>& buffer, const T& it)
{
    buffer.push_back(_get_instr<T>());
    if (it.is_null()) {
        buffer.push_back(Instr::END);
    } else {
        buffer.push_back(Instr::NODE_VALUE);
        uint32_t pos[2] = {};
        memcpy(pos, &it.point.x, sizeof(uint32_t));
        memcpy(&pos[1], &it.point.y, sizeof(uint32_t));
        _push_uint(buffer, pos[0]);
        _push_uint(buffer, pos[1]);
        if constexpr (std::is_same<T, InputNode>()) {
            buffer.push_back(it._freq != 0 ? 1u : 0u);
            if (it._freq) {
                uint32_t s;
                memcpy(&s, &it._freq, sizeof(float));
                _push_uint(buffer, s);
            } else {
                buffer.push_back(it.get() == State::TRUE ? 1u : 0u);
            }
        } else if constexpr (std::is_same<T, GateNode>()) {
            buffer.push_back(it.type());
            _push_uint(buffer, it.inputs.size());
        } else if constexpr (std::is_same<T, ComponentNode>()) {
            buffer.push_back(it.dep_idx);
        }
    }
}

static void _encode_nodes(const Scene& s, std::vector<uint8_t>& buffer)
{
    for (const auto& it : s._gates) {
        _encode_node<GateNode>(buffer, it);
    }
    for (const auto& it : s._inputs) {
        _encode_node<InputNode>(buffer, it);
    }
    for (const auto& it : s._outputs) {
        _encode_node<OutputNode>(buffer, it);
    }
    for (const auto& it : s._components) {
        _encode_node<ComponentNode>(buffer, it);
    }
}

static void _encode_rel(const Scene& s, std::vector<uint8_t>& buffer)
{
    for (const auto& [id, rel] : s._relations) {
        buffer.push_back(Instr::CONNECT);
        _push_uint(buffer, encode_pair(rel.from_node, rel.from_sock, true));
        _push_uint(buffer, encode_pair(rel.to_node, rel.to_sock, false));
    }
}

LCS_ERROR serialize(const Scene& s, std::vector<uint8_t>& buffer)
{
    buffer.clear();
    buffer.reserve(sizeof(s));
    buffer.push_back(1u);
    _encode_meta(s, buffer);
    _encode_nodes(s, buffer);
    _encode_rel(s, buffer);
    return Error::OK;
}

LCS_ERROR static inline _strdecode(
    const uint8_t** buf_r, const uint8_t* endptr, char* buf_w, size_t buf_w_s)
{
    const uint8_t* cursor = *buf_r;
    size_t rem            = endptr - cursor;
    size_t len            = strnlen((const char*)cursor, rem);
    if (len == 0 || len > buf_w_s) {
        return ERROR(Error::INVALID_STRING);
    }
    std::strncpy((char*)buf_w, (const char*)cursor, len);
    // Length of string + NULL byte
    cursor += len + 1;
    *buf_r = cursor;
    return Error::OK;
}

LCS_ERROR static inline _decode_dep(
    const uint8_t** bgnptr, const uint8_t* endptr, Scene& s)
{
    static std::array<char, 300> dependency {};
    Error err
        = _strdecode(bgnptr, endptr, dependency.data(), dependency.max_size());
    if (err) {
        return err;
    }
    s.dependencies.push_back(Scene {});
    err = io::load_dependency(
        dependency.begin(), s.dependencies[s.dependencies.size() - 1]);
    if (err) {
        return err;
    }
    return Error::OK;
}

template <typename T>
LCS_ERROR static inline _decode_node(const uint8_t** bgnptr,
    const uint8_t* endptr, Scene& s, std::vector<Node>& null_list)
{
    const uint8_t* cursor = *bgnptr;
    expect_at_least(cursor, endptr, uint8_t);
    if (*cursor == Instr::END) {
        null_list.push_back(s.add_node<T>());
        *bgnptr = cursor;
        return Error::OK;
    } else if (*cursor != Instr::NODE_VALUE) {
        return ERROR(Error::INVALID_NODE);
    }
    cursor++;
    uint32_t pos_x = _pop_uint(&cursor, endptr);
    uint32_t pos_y = _pop_uint(&cursor, endptr);
    Node n;
    if constexpr (std::is_same<T, GateNode>()) {
        GateNode::Type type = static_cast<GateNode::Type>(*cursor);
        cursor++;
        uint32_t size = _pop_uint(&cursor, endptr);

        n = s.add_node<GateNode>(type, size);
    }
    if constexpr (std::is_same<T, InputNode>()) {
        expect_at_least(cursor, endptr, uint16_t);
        uint8_t is_timer = *cursor;
        cursor++;
        if (is_timer) {
            uint8_t freq = *cursor;
            cursor++;
            n = s.add_node<InputNode>(freq);
        } else {
            expect_at_least(cursor, endptr, uint8_t);
            State value = *cursor ? State::TRUE : State::FALSE;
            cursor++;
            n = s.add_node<InputNode>();
            s.get_node<InputNode>(n)->set(value);
        }
    }
    if constexpr (std::is_same<T, ComponentNode>()) {
        n               = s.add_node<ComponentNode>();
        uint8_t dep_idx = *cursor;
        if (dep_idx >= s.dependencies.size()) {
            return ERROR(Error::INVALID_NODE);
        }
        if (Error err = s.get_node<ComponentNode>(n)->set_component(dep_idx);
            err) {
            return err;
        }
    }
    if constexpr (std::is_same<T, OutputNode>()) {
        n = s.add_node<OutputNode>();
    }
    s.get_node<T>(n)->point.x = pos_x;
    s.get_node<T>(n)->point.y = pos_y;
    *bgnptr                   = cursor;
    return Error::OK;
}

LCS_ERROR static inline _decode_branch(const uint8_t** bgnptr,
    const uint8_t* endptr, Scene& s, std::vector<Node>& null_list)
{
    Error err             = Error::OK;
    const uint8_t* cursor = *bgnptr;
    Instr instr           = (Instr)*cursor;
    cursor++;
    switch (instr) {
    case Instr ::SET_NAME:
        L_DEBUG("%s", "Instr::SET_NAME");
        err = _strdecode(&cursor, endptr, s.name.data(), s.name.size());
        break;
    case Instr ::SET_DESC:
        L_DEBUG("%s", "Instr::SET_DESC");
        err = _strdecode(
            &cursor, endptr, s.description.data(), s.description.size());
        break;
    case Instr ::SET_AUTHOR:
        L_DEBUG("%s", "Instr::SET_AUTHOR");
        err = _strdecode(&cursor, endptr, s.author.data(), s.author.size());
        break;
    case Instr ::SET_VERSION:
        L_DEBUG("%s", "Instr::SET_VERSION");
        s.version = _pop_uint(&cursor, endptr);
        break;
    case Instr ::SET_COMP: {
        L_DEBUG("%s", "Instr::SET_COMP");
        uint32_t input_s  = _pop_uint(&cursor, endptr);
        uint32_t output_s = _pop_uint(&cursor, endptr);
        s.component_context.emplace(ComponentContext { &s });
        s.component_context->setup(input_s, output_s);
        break;
    }
    case Instr::INCLUDE:
        L_DEBUG("Instr::INCLUDE");
        err = _decode_dep(&cursor, endptr, s);
        break;
    case Instr::ADD_INPUT:
        L_DEBUG("Instr::ADD_INPUT");
        err = _decode_node<InputNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::ADD_OUT:
        L_DEBUG("Instr::ADD_OUTPUT");
        err = _decode_node<OutputNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::ADD_GATE:
        L_DEBUG("Instr::ADD_GATE");
        err = _decode_node<GateNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::ADD_COMP:
        L_DEBUG("Instr::ADD_COMP");
        err = _decode_node<ComponentNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::CONNECT: {
        L_DEBUG("Instr::CONNECT");
        uint32_t from_u = _pop_uint(&cursor, endptr);
        uint32_t to_u   = _pop_uint(&cursor, endptr);
        sockid from_sock;
        Node from = decode_pair(from_u, &from_sock);
        sockid to_sock;
        Node to = decode_pair(to_u, &to_sock);
        if (!s.connect(to, to_sock, from, from_sock)) {
            return ERROR(Error::INVALID_RELID);
        };
        break;
    }
    default: {
        return ERROR(Error::INVALID_BYTE);
    }
    }
    L_DEBUG("Instr offset: %zu", cursor - *bgnptr);
    *bgnptr = cursor;
    return err;
}

LCS_ERROR deserialize(const std::vector<uint8_t>& buffer, Scene& s)
{
    const uint8_t* cursor = buffer.data();
    uint8_t version       = *cursor;
    if (version != 1) {
        return ERROR(Error::INVALID_SCENE_FORMAT);
    }
    cursor++; // skip version
    // NOTE: Generate null slot's like regular nodes. Then set all items in
    // null_list to null using BaseNode::set_null function. The reason is
    // Scene::add_node function tries to put next item in a available slot, thus
    // making null item insertion impossible.
    std::vector<Node> null_list {};
    const uint8_t* endptr = buffer.data() + buffer.size();

    while (cursor + sizeof(uint16_t) < buffer.data() + buffer.size()) {
        Error err = _decode_branch(&cursor, endptr, s, null_list);
        if (err) {
            return err;
        }
    }
    return Error::OK;
}

} // namespace lcs
