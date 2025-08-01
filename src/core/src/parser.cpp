#include "common.h"
#include "core.h"
#include "port.h"
#include <json/value.h>
#include <cstring>
#include <stack>
#include <type_traits>

namespace lcs::io {
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

static inline size_t _strdecode(const uint8_t* buffer, size_t rem,
    char* write_buffer, size_t write_buffer_s)
{
    const uint8_t* cursor = buffer;
    size_t len = strnlen((const char*)cursor, std::min(rem, write_buffer_s));
    std::strncpy((char*)write_buffer, (char*)cursor, len);
    write_buffer[write_buffer_s - 1] = 0;
    return len;
}

LCS_ERROR deserialize(const std::vector<uint8_t>& buffer, Scene& s)
{
    const uint8_t* cursor = buffer.data();
    uint32_t version      = ntohl(*(uint32_t*)cursor);
    if (version != 1) {
        return ERROR(Error::INVALID_JSON_FORMAT);
    }
    cursor++; // skip version

    while (cursor < buffer.data() + buffer.size()) {
        size_t rem = buffer.size() - (cursor - buffer.data());

        switch (*cursor) {
        case SET_NAME:
            cursor += _strdecode(cursor + 1, rem, s.name.data(), s.name.size());
            break;
        case SET_DESC:
            cursor += _strdecode(
                cursor + 1, rem, s.description.data(), s.description.size());
            break;
        case SET_AUTHOR:
            cursor += _strdecode(
                cursor + 1, rem, s.author.data(), s.author.size());
            break;
        case SET_VERSION:
            if (rem < sizeof(uint32_t)) {
                return ERROR(Error::INVALID_JSON_FORMAT);
            }
            memcpy(&s.version, cursor + 1, sizeof(uint32_t));
            s.version = ntohl(s.version);
            cursor += 5;
            break;
        case SET_COMP: {
            if (rem < sizeof(uint64_t)) {
                return ERROR(Error::INVALID_JSON_FORMAT);
            }
            uint32_t io[2];
            memcpy(io, cursor + 1, sizeof(uint64_t));
            io[0] = ntohl(io[0]);
            io[1] = ntohl(io[1]);
            s.component_context.emplace(ComponentContext { &s });
            s.component_context->setup(io[0], io[1]);
        }
                       // TODO You are here!
        case ADD_INPUT:
        case ADD_OUT:
        case ADD_GATE:
        case ADD_COMP:
        case CONNECT:
        default: return ERROR(Error::INVALID_JSON_FORMAT);
        }
    }
    return Error::OK;
}

} // namespace lcs::io
