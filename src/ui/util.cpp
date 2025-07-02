#include "ui/util.h"
#include "core.h"
#include "io.h"
#include <imgui.h>
#include <imnodes.h>
#include <optional>

static std::map<std::string, lcs::ui::ImageHandle> _TEXTURE_MAP;
namespace lcs::ui {

bool PositionSelector(Point& point, const char* prefix)
{
    const static ImVec2 __selector_size = ImGui::CalcTextSize("-000000000000");

    std::string s_prefix = "##";
    s_prefix += prefix;
    ImGui::AlignTextToFramePadding();
    ImGui::Text("( x: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(__selector_size.x);
    bool change_x
        = ImGui::InputInt((s_prefix + "X").c_str(), &point.x, 1.0f, 10.0f, 0);
    ImGui::SameLine();
    ImGui::Text(", y:");
    ImGui::SameLine();
    bool change_y
        = ImGui::InputInt((s_prefix + "Y").c_str(), &point.y, 1.0f, 10.0f, 0);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text(")");
    return change_x || change_y;
}

State ToggleButton(State state, bool clickable)
{
    const LcsStyle& style = get_active_style();
    ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::SMALL));
    switch (state) {
    case State::TRUE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.green);
            ImGui::PushStyleColor(ImGuiCol_Text, style.fg);
            if (ImGui::Button("TRUE ")) {
                state = FALSE;
            };
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(style.green, "TRUE");
        }
        break;
    case State::FALSE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.red);
            if (ImGui::Button("FALSE")) {
                state = TRUE;
            };
        } else {
            ImGui::TextColored(style.red, "FALSE");
        }
        break;
    case State::DISABLED:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.black_bright);
            ImGui::PushStyleColor(ImGuiCol_Text, style.white_bright);
            ImGui::Button("DISCONNECTED");
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(style.black_bright, "DISCONNECTED");
        }
        break;
    }
    if (clickable) {
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    return state;
}

void ToggleButton(NRef<InputNode> node)
{
    State s_old = node->get();

    if (s_old != ToggleButton(node->get(), true)) {
        node->toggle();
        io::scene::notify_change();
    }
}

void NodeTypeTitle(Node n)
{
    static char buffer[256];
    ImGui::PushStyleColor(ImGuiCol_TextLink, NodeType_to_color(n.type));
    snprintf(buffer, 256, "%s@%u", NodeType_to_str_full(n.type), n.id);
    if (ImGui::TextLink(buffer)) {
        ImNodes::ClearNodeSelection();
        switch (n.type) {
        case COMPONENT_INPUT:
        case COMPONENT_OUTPUT:
            ImNodes::SelectNode(Node { 0, n.type }.numeric());
            break;
        default: ImNodes::SelectNode(n.numeric()); break;
        }
    };
    ImGui::PopStyleColor();
}

void NodeTypeTitle(Node n, sockid sock)
{
    ImGui::PushFont(get_font(FontFlags::REGULAR | FontFlags::NORMAL));
    NodeTypeTitle(n);
    ImGui::SameLine();
    ImGui::Text("sock:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.00f), "%u", sock);
    ImGui::PopFont();
}

int encode_pair(Node node, sockid sock, bool is_out)
{
    lcs_assert(node.id < 0xFFFF);
    int x = node.id | (node.type << 16) | (sock << 20);
    if (is_out) {
        x |= 1 << 31;
    }
    return x;
}

Node decode_pair(int code, sockid* sock, bool* is_out)
{
    if (is_out != nullptr) {
        *is_out = code >> 31;
    }
    if (sock != nullptr) {
        *sock = (code >> 20) & 0xFF;
    }
    return Node { static_cast<uint16_t>(code & 0xFFFF),
        static_cast<NodeType>((code >> 16) & 0x0F) };
}

const ImageHandle* get_texture(const std::string& key)
{
    auto p = _TEXTURE_MAP.find(key);
    if (p == _TEXTURE_MAP.end()) {
        return nullptr;
    }
    return &p->second;
}

} // namespace lcs::ui

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <GL/gl.h>
#include <stb_image.h>
// Simple helper function to load an image into a OpenGL texture with common
// settings
bool lcs::ui::load_texture(
    const std::string& key, std::vector<unsigned char>& buffer)
{
    // Load from file
    int image_width           = 0;
    int image_height          = 0;
    unsigned char* image_data = stbi_load_from_memory(
        buffer.data(), buffer.size(), &image_width, &image_height, nullptr, 4);
    if (image_data == nullptr) {
        return false;
    }

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
    _TEXTURE_MAP.insert_or_assign(key,
        lcs::ui::ImageHandle {
            (uint32_t)image_texture, image_width, image_height });
    return true;
}

bool lcs::ui::load_texture(const std::string& key, const std::string& file_path)
{
    std::ifstream infile { file_path, std::ios::binary | std::ios::ate };
    if (!infile) {
        return false;
    }

    std::streamsize file_size = infile.tellg();
    if (file_size < 0) {
        return false;
    }
    infile.seekg(0, std::ios::beg);
    std::vector<unsigned char> data;
    data.resize(file_size);
    if (!infile.read(reinterpret_cast<char*>(data.data()), file_size)) {
        return false;
    }

    return load_texture(key, data);
}
