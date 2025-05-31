#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>

#include "ui.h"
#include "ui/layout.h"
#include "ui/nodes.h"

static bool show_demo_window = true;
ImVec4 clear_color           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float f;

namespace lcs::ui {

constexpr ImVec4 node_to_color(node_t type)
{
    switch (type) {
    case node_t::GATE: return ImVec4(0, 1, 1, 1);
    case node_t::INPUT: return ImVec4(0.3, 0.3, 0.7, 1);
    case node_t::OUTPUT: return ImVec4(0.6, 0.0, 0.6, 1);
    case node_t::COMPONENT: return ImVec4(0.3, 0.7, 0.3, 1);
    default: return ImVec4(0.8, 0.6, 0.1, 1);
    }
}

void _value_tooltip(state_t s)
{
    switch (s) {
    case state_t::TRUE:
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
        ImGui::Text("TRUE");
        break;
    case state_t::FALSE:
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
        ImGui::Text("FALSE");
        break;
    case state_t::DISABLED:
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
        ImGui::Text("DISABLED");
        break;
    default: break;
    }
    ImGui::PopStyleColor();
}

void node_to_title(node n)
{
    ImGui::TextColored(node_to_color(n.type), "%s", node_to_str(n.type));
    ImGui::SameLine();
    ImGui::Text("@");
    ImGui::SameLine();
    ImGui::TextColored(
        ImVec4(0, 1.0, 0, 1), "%s", std::to_string(n.id).c_str());
}

void node_to_title(node n, sockid sock)
{
    ImGui::PushFont(get_font(font_flags_t::REGULAR | font_flags_t::NORMAL));
    node_to_title(n);
    ImGui::SameLine();
    ImGui::Text("sock:");
    ImGui::SameLine();
    ImGui::TextColored(
        ImVec4(0.5f, 0.5f, 1.0f, 1.00f), "%s", std::to_string(sock).c_str());
    ImGui::PopFont();
}

int _hash_node_sock_pair(node node, sockid sock)
{
    // WARN: I hope you never reach 24 bit id.
    int x = node.id;
    lcs_assert(node.id < 0xFFFF);
    x |= node.type << 16;
    x |= sock << 24;
    return x;
}

void before(ImGuiIO&) { ImNodes::CreateContext(); }

bool loop(ImGuiIO& io)
{
    ImGui::PushFont(get_font(font_flags_t::NORMAL));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoTitleBar;
    MenuBar();
    ImVec2 app_size = io.DisplaySize;
    ImGui::Begin("Node Editor", nullptr, flags);
    ImGui::SetWindowSize(ImVec2 { app_size.x * 0.85f, app_size.y * 0.85f });
    ImGui::SetWindowPos(
        ImVec2 { app_size.x * 0.0f, app_size.y * 0.0f + 20.0f });
    bool scene_changed = TabWindow();
    ImNodes::BeginNodeEditor();
    NRef<Scene> scene = io::scene::get();
    if (scene == nullptr) {
        ImNodes::EndNodeEditor();
        ImGui::End();
        ImGui::PopFont();
        return true;
    }
    for (auto& out : scene->_inputs) {
        NodeView<InputNode>(&out.second).show_node(scene_changed);
    }
    for (auto& out : scene->_outputs) {
        NodeView<OutputNode>(&out.second).show_node(scene_changed);
    }
    for (auto& out : scene->_gates) {
        NodeView<GateNode>(&out.second).show_node(scene_changed);
    }
    for (auto& out : scene->_components) {
        NodeView<ComponentNode>(&out.second).show_node(scene_changed);
    }
    for (auto& r : scene->_relations) {
        ImNodes::PushColorStyle(ImNodesCol_Link,
            r.second.value == state_t::TRUE        ? IM_COL32(0, 255, 100, 255)
                : r.second.value == state_t::FALSE ? IM_COL32(255, 0, 100, 255)
                                                   : IM_COL32(55, 55, 55, 255)

        );
        ImNodes::Link(r.first,
            _hash_node_sock_pair(r.second.from_node, r.second.from_sock),
            _hash_node_sock_pair(r.second.to_node, r.second.to_sock));
        ImNodes::PopColorStyle();
    }

    ImNodes::EndNodeEditor();

    {
        int linkid = 0;
        if (ImNodes::IsLinkHovered(&linkid)) {
            if (auto r = scene->get_rel(linkid);
                ImGui::BeginTooltip() && r != nullptr) {
                ImGui::PushFont(
                    get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
                ImGui::Text("%s",
                    std::string { "Relation " + std::to_string(linkid) }
                        .c_str());
                ImGui::Separator();
                ImGui::PopFont();
                ImGui::PushFont(get_font(font_flags_t::NORMAL));
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "From:");
                ImGui::SameLine();
                node_to_title(r->from_node, r->from_sock);
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "To:");
                ImGui::SameLine();
                node_to_title(r->to_node, r->to_sock);
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "Value:");
                ImGui::SameLine();
                _value_tooltip(r->value);
                ImGui::PopFont();
                ImGui::EndTooltip();
            }
        }

        int nodeid_encoded = 0;
        if (ImNodes::IsNodeHovered(&nodeid_encoded)) {
            node nodeid { 0x0FFFF & (uint32_t)nodeid_encoded,
                (node_t)(nodeid_encoded >> 16) };

            if (NRef<BaseNode> n = scene->get_base(nodeid);
                ImGui::BeginTooltip() && n != nullptr) {
                ImGui::PushFont(
                    get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
                node_to_title((node)nodeid);
                ImGui::Separator();
                ImGui::PopFont();
                ImGui::PushFont(get_font(font_flags_t::NORMAL));
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "Position:");
                ImGui::SameLine();
                ImGui::Text("(%d, %d)", n->point.x, n->point.y);
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "Connected:");
                ImGui::SameLine();
                ImGui::Text("%s", n->is_connected() ? "true" : "false");
                if (nodeid.type != node_t::COMPONENT) {
                    ImGui::TextColored(ImVec4(200, 200, 0, 255), "Value:");
                    ImGui::SameLine();
                    _value_tooltip(n->get());
                } else {
                    auto comp = scene->get_node<ComponentNode>(nodeid);
                    ImGui::Text("(");
                    ImGui::SameLine();
                    for (size_t i = 0; i < comp->inputs.size(); i++) {
                        _value_tooltip(comp->get(i));
                        ImGui::SameLine();
                    }
                    ImGui::Text(")");
                }

                ImGui::PopFont();
                ImGui::EndTooltip();
            }
        }
    }

    ImGui::End();
    ImGui::PopFont();

    ImGui::Begin("Font Window");
    ImGui::PushFont(get_font(SMALL | ITALIC));
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();
    ImGui::PushFont(get_font(LARGE | BOLD));
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();
    ImGui::PushFont(get_font(NORMAL));
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();
    ImGui::PushFont(get_font(BOLD | ITALIC));
    ImGui::Text("This is some useful text.");
    ImGui::PopFont();

    ImGui::Checkbox("Demo Window",
        &show_demo_window); // Edit bools storing our window
                            // open/close state

    ImGui::SliderFloat("float", &f, 0.0f,
        1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color",
        (float*)&clear_color); // Edit 3 floats representing a color

    if ((ImGui::Checkbox("DarkModeButton", &is_dark))) {
        ImGui::SameLine();
        ImGui::Text("Dark Mode: %s", is_dark ? "true" : "false");
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / io.Framerate, io.Framerate);
    ImGui::End();

    return show_demo_window;
}

void after(ImGuiIO&)
{
    if (io::scene::get() != nullptr) {
        io::scene::save();
        io::scene::close();
    }
    ImNodes::DestroyContext();
}
} // namespace lcs::ui
