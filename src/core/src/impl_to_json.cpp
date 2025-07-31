#include "common.h"
#include "core.h"
#include "port.h"
#include <json/value.h>
#include <type_traits>

namespace lcs::io {
/**
 * Tokens define the active sections. LcsFiles work like a stack. When a
 * section is pushed its encoding system is applied its being popped.
 */
enum Token : uint8_t {
    LF  = 0x0A,
    END = 0x3,

    SECT_META  = 0x4D,
    SECT_INC   = 0x16,
    SECT_NODE  = 0x4E,
    SECT_CON   = 0x43,
    SECT_NGATE = 0x77,
    SECT_NIN   = 0x69,
    SECT_NOUT  = 0x6F,
    SECT_NCOMP = 0x63,

    TRUE          = 0x6,
    FALSE         = 0x15,
    EMPTY         = 0x18,
    TIMER_INPUT   = 0x11,
    DEFAULT_INPUT = 0x13,

};

static void _encode_meta(const Scene& s, std::vector<uint8_t>& buffer)
{
    buffer.push_back(Token::SECT_META);
    size_t name_s   = strnlen(s.name.begin(), s.name.max_size());
    size_t desc_s   = strnlen(s.description.begin(), s.description.max_size());
    size_t author_s = strnlen(s.author.begin(), s.author.max_size());
    buffer.insert(buffer.end(), s.name.begin(), s.name.begin() + name_s);
    buffer.push_back(Token::LF);
    buffer.insert(
        buffer.end(), s.description.begin(), s.description.begin() + desc_s);
    buffer.push_back(Token::LF);
    buffer.insert(buffer.end(), s.author.begin(), s.author.begin() + author_s);
    buffer.push_back(Token::LF);
    buffer.push_back(Token::END);
}

template <typename T>
static inline void _encode_node(std::vector<uint8_t>& buffer, const T& it)
{
    if (it.is_null()) {
        buffer.push_back(Token::EMPTY);
    } else {
        uint32_t pos[2] = {};
        memcpy(pos, &it.point.x, sizeof(uint32_t));
        memcpy(&pos[1], &it.point.y, sizeof(uint32_t));
        pos[0] = net::htonl(pos[0]);
        pos[1] = net::htonl(pos[1]);
        buffer.insert(buffer.end(), pos, pos + 2 * sizeof(uint32_t));
        if constexpr (std::is_same<T, InputNode>()) {
            if (it._freq.has_value()) {
                buffer.push_back(Token::TIMER_INPUT);
                uint32_t s;
                memcpy(&s, &it._freq.value(), sizeof(float));
                s = net::htonl(s);
                buffer.insert(buffer.end(), &s, &s + sizeof(uint32_t));
            } else {
                buffer.push_back(Token::DEFAULT_INPUT);
                buffer.push_back(
                    it.get() == State::TRUE ? Token::TRUE : Token::FALSE);
            }
        } else if constexpr (std::is_same<T, GateNode>()) {
            uint32_t type = net::htonl(it.type());
            uint32_t size = net::htonl(it.inputs.size());
            buffer.insert(buffer.end(), &type, &type + sizeof(uint32_t));
            buffer.insert(buffer.end(), &size, &size + sizeof(uint32_t));
        } else if constexpr (std::is_same<T, ComponentNode>()) {
            buffer.insert(buffer.end(), it.path.begin(), it.path.end());
        }
    }
    buffer.push_back(Token::LF);
}

static void _encode_nodes(const Scene& s, std::vector<uint8_t>& buffer)
{
    buffer.push_back(Token::SECT_NODE);
    {
        buffer.push_back(Token::SECT_NGATE);
        for (auto it = s._gates.begin(); it != s._gates.end(); ++it) {
            _encode_node<GateNode>(buffer, *it);
        }
        buffer.push_back(Token::END);
        buffer.push_back(Token::SECT_NIN);
        for (auto it = s._inputs.begin(); it != s._inputs.end(); ++it) {
            _encode_node<InputNode>(buffer, *it);
        }
        buffer.push_back(Token::END);
        buffer.push_back(Token::SECT_NOUT);
        for (auto it = s._outputs.begin(); it != s._outputs.end(); ++it) {
            _encode_node<OutputNode>(buffer, *it);
        }
        buffer.push_back(Token::END);
        buffer.push_back(Token::SECT_NCOMP);
        for (auto it = s._components.begin(); it != s._components.end(); ++it) {
            _encode_node<ComponentNode>(buffer, *it);
        }
        buffer.push_back(Token::END);
    }
    buffer.push_back(Token::END);
}

static void _encode_rel(const Scene& s, std::vector<uint8_t>& buffer)
{
    buffer.push_back(Token::SECT_CON);
    for (const auto& [id, rel] : s._relations) {
        uint32_t from
            = net::htonl(encode_pair(rel.from_node, rel.from_sock, true));
        uint32_t to = net::htonl(encode_pair(rel.to_node, rel.to_sock, false));
        buffer.insert(buffer.end(), &from, &from + sizeof(uint32_t));
        buffer.insert(buffer.end(), &to, &to + sizeof(uint32_t));
        buffer.push_back(Token::LF);
    }
    buffer.push_back(Token::END);
}

LCS_ERROR read_scene(const Scene& s, std::vector<uint8_t>& buffer)
{
    buffer.clear();
    buffer.reserve(sizeof(s));
    buffer.insert(buffer.begin(), std::string_view { VERSION }.begin(),
        std::string_view { VERSION }.end());
    _encode_meta(s, buffer);
    _encode_nodes(s, buffer);
    _encode_rel(s, buffer);
    return Error::OK;
}

} // namespace lcs::io
