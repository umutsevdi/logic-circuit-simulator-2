#include "common.h"
#include "core.h"
#include "imgui.h"
#include "io.h"
#include "ui.h"
#include "ui/layout.h"
#include "ui/util.h"
#include <charconv>
#include <cmath>

namespace lcs::ui {

static Node node = 0;

void set_selected(Node new_node) { node = new_node; };
static void _inspector_input_node(NRef<InputNode>);
static void _inspector_output_node(NRef<OutputNode>);
static void _inspector_component_node(NRef<ComponentNode>);
static void _inspector_gate_node(NRef<GateNode>);
// static void _inspector_component_context_node(NRef<ComponentContext>);

void Inspector(NRef<Scene> scene)
{
    if (node.id == 0) {
        return;
    }

    NRef<BaseNode> base_node = scene->get_base(node);
    if (base_node == nullptr) { /** TODO */
        return;
    }

    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 app_size = io.DisplaySize;

    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2 { app_size.x * 0.15f, app_size.y * 0.2f });
    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::LARGE));
    NodeTypeTitle(node);
    ImGui::PopFont();
    ImGui::Separator();
    Field("Id:");
    ImGui::SameLine();
    ImGui::Text("%ul", node.id);

    Field("Type:");
    ImGui::SameLine();
    ImGui::Text("%s", NodeType_to_str(node.type));

    Field("Position");
    ImGui::SameLine();
    int positions[2] = { base_node->point.x, base_node->point.y };
    ImGui::InputInt2("##", positions);

    switch (node.type) {
    case NodeType::INPUT:
        _inspector_input_node(scene->get_node<InputNode>(node));
        break;
    case NodeType::OUTPUT:
        _inspector_output_node(scene->get_node<OutputNode>(node));
        break;
    case NodeType::GATE:
        _inspector_gate_node(scene->get_node<GateNode>(node));
        break;
    case NodeType::COMPONENT:
        _inspector_component_node(scene->get_node<ComponentNode>(node));
        break;
    default: break;
    }

    ImGui::End();
    node = 0;
}

void _inspector_input_node(NRef<InputNode> _node)
{
    constexpr size_t SIZE     = 20;
    static float values[SIZE] = { 0 };
    static int frame_count    = 0;
    if (_node->is_timer()) {
        Field("Value");
        ImGui::SameLine();
        ToggleButton(_node->get());
        Field("Frequency");
        ImGui::SameLine();
        float freq_value = _node->_freq.value();
        if (ImGui::InputFloat("Hz", &freq_value, 0.25f, 1.0f, "%.2f")) {
            freq_value = std::max(
                0.25f, std::min(std::round(freq_value * 4.0f) / 4.0f, 10.0f));
            if (freq_value != _node->_freq.value()) {
                _node->_freq = freq_value;
                io::scene::notify_change();
            }
        }

        if (++frame_count % SIZE == 0) {
            for (size_t i = 1; i < SIZE; i++) {
                values[i - 1] = values[i];
            }
        }
        values[SIZE - 1] = _node->get() == State::TRUE;
        ImGui::PlotLines("##Frequency", values, SIZE, 0, nullptr, 0.0f, 1.0f);

    } else {
        Field("Value");
        ImGui::SameLine();
        ToggleButton(&_node);
    }
}

static void _inspector_output_node(NRef<OutputNode> _node)
{
    Field("Value");
    ImGui::SameLine();
    ToggleButton(_node->get());
}
static void _inspector_component_node(NRef<ComponentNode> _node) { }
static void _inspector_gate_node(NRef<GateNode> _node) { }

} // namespace lcs::ui
