#include "common.h"
#include "core.h"
#include <cstring>

#define expect_at_least(BGNPTR, ENDPTR, TYPE)                                  \
    {                                                                          \
        size_t __REM = (ENDPTR - BGNPTR);                                      \
        if (__REM > sizeof(TYPE)) {                                            \
            return ERROR(Error::UNTERMINATED_STATEMENT);                       \
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
    /** Read no more than min(128, buffer_s) characters stopping at NULL byte.
     */
    SET_NAME = 0x10,
    /** Read no more than min(512, buffer_s) characters stopping at NULL byte.
     */
    SET_DESC = 0x11,
    /** Read no more than min(60, buffer_s) characters stopping at NULL byte. */
    SET_AUTHOR = 0x12,
    /** Read version. Fmt: UINT32 version */
    SET_VERSION = 0x13,
    /** Sets the scene as a component. Fmt: UINT32 input_s, UINT32 output_s  */
    SET_COMP = 0x14,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y,
       UINT8 is_timer, (FLOAT32 freq|UINT8 value) */
    ADD_INPUT = 0x15,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y  */
    ADD_OUT = 0x16,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y,
       UINT32 size, UINT32 type  */
    ADD_GATE = 0x17,
    /** Add a node to the scene. Fmt: UINT8 null, UINT32 pos.x, UINT32 pos.y,
     * UINT8 dep_id
     */
    ADD_COMP = 0x18,
    /** Connect sockets of two nodes. Fmt: UINT32 from(packed), UINT32
       to(packed) */
    CONNECT = 0x20,

    /** ADD_T is about to insert a valid Node. */
    NODE_VALUE = 0x1C,
};

static void _encode_meta(const Scene& s, std::vector<uint8_t>& buffer)
{
    size_t name_s   = strnlen(s.name.begin(), s.name.size());
    size_t desc_s   = strnlen(s.description.begin(), s.description.size());
    size_t author_s = strnlen(s.author.begin(), s.author.size());
    buffer.push_back(Instr::SET_NAME);
    buffer.insert(buffer.end(), s.name.begin(), s.name.begin() + name_s);
    buffer.push_back(Instr::END);
    buffer.push_back(Instr::SET_DESC);
    buffer.insert(
        buffer.end(), s.description.begin(), s.description.begin() + desc_s);
    buffer.push_back(Instr::END);
    buffer.push_back(Instr::SET_AUTHOR);
    buffer.insert(buffer.end(), s.author.begin(), s.author.begin() + author_s);
    buffer.push_back(Instr::END);
    buffer.push_back(Instr::SET_VERSION);
    uint32_t version = htonl(s.version);
    buffer.insert(buffer.end(), &version, &version + sizeof(uint32_t));
    if (s.component_context.has_value()) {
        uint32_t input_s  = htonl(s.component_context->inputs.size());
        uint32_t output_s = htonl(s.component_context->outputs.size());
        buffer.push_back(Instr::SET_COMP);
        buffer.insert(buffer.end(), &input_s, &input_s + sizeof(uint32_t));
        buffer.insert(buffer.end(), &output_s, &output_s + sizeof(uint32_t));
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
    constexpr Instr cmd = _get_instr<T>();
    if (it.is_null()) {
        buffer.push_back(cmd);
        buffer.push_back(Instr::END);
    } else {
        buffer.push_back(Instr::NODE_VALUE);
        uint32_t pos[2] = {};
        memcpy(pos, &it.point.x, sizeof(uint32_t));
        memcpy(&pos[1], &it.point.y, sizeof(uint32_t));
        pos[0] = htonl(pos[0]);
        pos[1] = htonl(pos[1]);
        buffer.insert(buffer.end(), pos, pos + sizeof(uint64_t));
        if constexpr (std::is_same<T, InputNode>()) {
            buffer.push_back(it._freq.has_value() ? 1u : 0u);
            if (it._freq.has_value()) {
                uint32_t s;
                memcpy(&s, &it._freq.value(), sizeof(float));
                s = htonl(s);
                buffer.insert(buffer.end(), &s, &s + sizeof(uint32_t));
            } else {
                buffer.push_back(it.get() == State::TRUE ? 1u : 0u);
            }
        } else if constexpr (std::is_same<T, GateNode>()) {
            uint32_t type = htonl(it.type());
            uint32_t size = htonl(it.inputs.size());
            buffer.insert(buffer.end(), &type, &type + sizeof(uint32_t));
            buffer.insert(buffer.end(), &size, &size + sizeof(uint32_t));
        } else if constexpr (std::is_same<T, ComponentNode>()) {
            // FIXME Replace it with the index
            buffer.insert(buffer.end(), it.path.begin(), it.path.end());
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
        uint32_t from = htonl(encode_pair(rel.from_node, rel.from_sock, true));
        uint32_t to   = htonl(encode_pair(rel.to_node, rel.to_sock, false));
        buffer.insert(buffer.end(), &from, &from + sizeof(uint32_t));
        buffer.insert(buffer.end(), &to, &to + sizeof(uint32_t));
    }
}

LCS_ERROR serialize(const Scene& s, std::vector<uint8_t>& buffer)
{
    buffer.clear();
    buffer.reserve(sizeof(s));
    uint32_t v = htonl(VERSION);
    buffer.insert(buffer.begin(), &v, &v + sizeof(uint32_t));
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
    size_t len = strnlen((const char*)cursor, std::min(rem, buf_w_s));
    if (len == 0) {
        return ERROR(Error::INVALID_STRING);
    }
    std::strncpy((char*)buf_w, (const char*)cursor, len);
    cursor += len;
    *buf_r = cursor;
    return Error::OK;
}

template <typename T>
LCS_ERROR static inline _decode_node(const uint8_t** bgnptr,
    const uint8_t* endptr, Scene& s, std::vector<Node>& null_list)
{
    const uint8_t* cursor = *bgnptr;
    expect_at_least(cursor, endptr, uint8_t);
    if (*cursor == Instr::NODE_VALUE) {
    } else if (*cursor == Instr::END) {
        null_list.push_back(s.add_node<T>());
        *bgnptr = cursor;
        return Error::OK;
    } else {
        return ERROR(Error::INVALID_NODE);
    }
    expect_at_least(cursor, endptr, uint64_t);
    uint32_t pos_x, pos_y;
    memcpy(&pos_x, cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);
    memcpy(&pos_y, cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);
    pos_x = ntohl(pos_x);
    pos_y = ntohl(pos_y);

    Node n;
    if constexpr (std::is_same<T, GateNode>()) {
        expect_at_least(cursor, endptr, uint64_t);
        uint32_t type, size;
        memcpy(&type, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        memcpy(&size, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        type = ntohl(type);
        size = ntohl(size);
        n    = s.add_node<GateNode>((GateNode::Type)type, size);
    }
    if constexpr (std::is_same<T, InputNode>()) {
        expect_at_least(cursor, endptr, uint8_t);
        uint8_t is_timer = *cursor;
        cursor++;
        if (is_timer) {
            expect_at_least(cursor, endptr, uint32_t);
            uint32_t freq_u;
            memcpy(&freq_u, cursor, sizeof(uint32_t));
            freq_u = ntohl(freq_u);
            float freq;
            memcpy(&freq, &freq_u, sizeof(float));
            cursor += sizeof(uint32_t);
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
        n = s.add_node<ComponentNode>();
        // FIXME replace with the index
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
    case Instr::SET_NAME:
        err = _strdecode(&cursor, endptr, s.name.data(), s.name.size());
        break;
    case Instr::SET_DESC:
        err = _strdecode(
            &cursor, endptr, s.description.data(), s.description.size());
        break;
    case Instr::SET_AUTHOR:
        err = _strdecode(&cursor, endptr, s.author.data(), s.author.size());
        break;
    case Instr::SET_VERSION:
        expect_at_least(cursor, endptr, uint32_t);
        memcpy(&s.version, cursor, sizeof(uint32_t));
        s.version = ntohl(s.version);
        cursor += sizeof(uint32_t);
        break;
    case Instr::SET_COMP: {
        expect_at_least(cursor, endptr, uint64_t);
        uint32_t io[2];
        memcpy(io, cursor, sizeof(uint64_t));
        io[0] = ntohl(io[0]);
        io[1] = ntohl(io[1]);
        s.component_context.emplace(ComponentContext { &s });
        s.component_context->setup(io[0], io[1]);
        break;
    }
    case Instr::ADD_INPUT:
        err = _decode_node<InputNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::ADD_OUT:
        err = _decode_node<OutputNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::ADD_GATE:
        err = _decode_node<GateNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::ADD_COMP:
        err = _decode_node<ComponentNode>(&cursor, endptr, s, null_list);
        break;
    case Instr::CONNECT: {
        expect_at_least(cursor, endptr, uint64_t);
        uint32_t from_u, to_u;
        memcpy(&from_u, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        memcpy(&to_u, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        from_u = ntohl(from_u);
        to_u   = ntohl(to_u);
        sockid from_sock;
        Node from = decode_pair(from_u, &from_sock);
        sockid to_sock;
        Node to = decode_pair(to_u, &to_sock);
        if (!s.connect(to, to_sock, from, from_sock)) {
            return ERROR(Error::INVALID_RELID);
        };
    }
    default: return ERROR(Error::INVALID_JSON_FORMAT);
    }
    *bgnptr = cursor;
    return err;
}

LCS_ERROR deserialize(const std::vector<uint8_t>& buffer, Scene& s)
{
    const uint8_t* cursor = buffer.data();
    uint32_t version      = ntohl(*(uint32_t*)cursor);
    if (version != 1) {
        return ERROR(Error::INVALID_JSON_FORMAT);
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
