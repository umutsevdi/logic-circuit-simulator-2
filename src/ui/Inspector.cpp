#include <cmath>

#include <imgui.h>

#include "common.h"
#include "core.h"
#include "imnodes.h"
#include "io.h"
#include "ui/layout.h"
#include "ui/util.h"

namespace lcs::ui {

static void _input_table(NRef<Scene>, const std::vector<relid>&);
static void _output_table(
    NRef<Scene>, const std::vector<relid>&, sockid id = 0);
static void _inspector_input_node(NRef<Scene>, Node);
static void _inspector_output_node(NRef<Scene>, Node);
static void _inspector_component_node(NRef<Scene>, Node);
static void _inspector_gate_node(NRef<Scene>, Node);
static void _inspector_component_context_node(NRef<Scene>, Node);
static void _inspector_tab(NRef<Scene>, Node);

void Inspector(NRef<Scene> scene)
{
    static int node_list[1 << 20] = { 0 };
    static char buffer[128];

    if (ImNodes::NumSelectedNodes() == 0) {
        return;
    }
    int selection_s = ImNodes::NumSelectedNodes();
    ImNodes::GetSelectedNodes(node_list);

    ImGui::Begin("Inspector", nullptr);
    if (selection_s > 1) {
        ImGui::BeginTabBar("InspectorTabs");
        for (int i = 0; i < ImNodes::NumSelectedNodes(); i++) {
            Node node = decode_pair(node_list[i]);
            snprintf(
                buffer, 128, "%s@%u", NodeType_to_str_full(node.type), node.id);
            NodeType_to_str_full(node.type);
            if (ImGui::BeginTabItem(
                    buffer, nullptr, ImGuiTabItemFlags_NoReorder)) {
                _inspector_tab(&scene, node);
                ImGui::EndTabItem();
            };
        }
        ImGui::EndTabBar();
    } else if (selection_s) {
        Node node = decode_pair(node_list[0]);
        _inspector_tab(&scene, node);
    }
    ImGui::End();
}

static void _inspector_tab(NRef<Scene> scene, Node node)
{
    const static ImVec2 __table_l_size = ImGui::CalcTextSize("Frequency");

    ImGui::PushFont(get_font(font_flags_t::BOLD | font_flags_t::LARGE));
    NodeTypeTitle(node);
    ImGui::PopFont();
    ImGui::Separator();

    if (ImGui::BeginTable(
            "##InspectorTable", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn(
            "##Key", ImGuiTableColumnFlags_WidthFixed, __table_l_size.x);

        ImGui::NextColumn();
        ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);
        TablePair(Field("Id"), ImGui::Text("%u", node.id));
        TablePair(
            Field("Type"), ImGui::Text("%s", NodeType_to_str_full(node.type)));

        if (node.type != COMPONENT_INPUT && node.type != COMPONENT_OUTPUT) {
            TablePair(Field("Position"));
            PositionSelector(scene->get_base(node), "Inspector");
        }

        switch (node.type) {
        case NodeType::INPUT: _inspector_input_node(&scene, node); break;
        case NodeType::OUTPUT: _inspector_output_node(&scene, node); break;
        case NodeType::GATE: _inspector_gate_node(&scene, node); break;
        case NodeType::COMPONENT:
            _inspector_component_node(&scene, node);
            break;
        default: _inspector_component_context_node(&scene, node); break;
        }
    };
}
static void _inspector_input_node(NRef<Scene> scene, Node node)
{
    auto _node                = scene->get_node<InputNode>(node);
    constexpr size_t SIZE     = 20;
    static float values[SIZE] = { 0 };
    static int frame_count    = 0;
    if (_node->is_timer()) {
        TablePair(Field("Value"), ToggleButton(_node->get()));
        TablePair(Field("Frequency"));
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
        ImGui::EndTable();

    } else {
        TablePair(Field("Value"), ToggleButton(&_node));
        ImGui::EndTable();
    }
    _output_table(&scene, _node->output);
}

static void _inspector_output_node(NRef<Scene> scene, Node node)
{
    auto _node = scene->get_node<OutputNode>(node);
    TablePair(Field("Value"), ToggleButton(_node->get()));
    ImGui::EndTable();
    std::vector<relid> in;
    in.push_back(_node->input);
    _input_table(&scene, in);
}

static void _inspector_gate_node(NRef<Scene> scene, Node node)
{
    auto _node = scene->get_node<GateNode>(node);
    TablePair(Field("Value"), ToggleButton(_node->get()));
    TablePair(
        Field("Gate Type"), ImGui::Text("%s", GateType_to_str(_node->type())));
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::EndTable();
    _input_table(&scene, _node->inputs);
    _output_table(&scene, _node->output);
}

static void _inspector_component_node(NRef<Scene> scene, Node node)
{
    auto _node = scene->get_node<ComponentNode>(node);
    TablePair(Field("Value"), ToggleButton(_node->get()));
    _input_table(&scene, _node->inputs);
    for (auto out : _node->outputs) {
        if (ImGui::BeginTabBar(std::to_string(out.first).c_str())) {
            _output_table(&scene, out.second, out.first);
        }
    }
    ImGui::EndTable();
}

static void _inspector_component_context_node(NRef<Scene> scene, Node node)
{
    ImGui::EndTable();
    if (node.type == COMPONENT_INPUT) {
        ImGui::BeginTabBar("Inputs");
        std::vector<std::vector<relid>>& inputs
            = scene->component_context->inputs;
        for (size_t i = 0; i < inputs.size(); i++) {
            if (ImGui::BeginTabItem(std::to_string(i).c_str())) {
                Field("Value");
                ImGui::SameLine();
                Node node_in = scene->component_context->get_input(i);
                State s_old  = scene->component_context->get_value(node_in);
                if (State s_new = ToggleButton(
                        scene->component_context->get_value(node_in), true);
                    s_old != s_new) {
                    scene->component_context->set_value(node_in, s_new);
                    scene->component_context->run();
                };
                _output_table(&scene, inputs[i], i);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    } else {
        _input_table(&scene, scene->component_context->outputs);
    }
}

static void _input_table(NRef<Scene> scene, const std::vector<relid>& inputs)
{
    if (ImGui::BeginTable("Inputs", 3, ImGuiTableFlags_BordersInner)) {
        ImGui::TableHeader("Inputs");
        ImGui::TableSetupColumn("Socket", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < inputs.size(); i++) {
            TablePair(Field("%zu", i));
            ImGui::PushFont(
                get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
            if (inputs[i] == 0) {
                ImGui::TextColored(ImVec4(0.3, 0.3, 0.3, 1), "DISCONNECTED");
            } else {
                NRef<Rel> r = scene->get_rel(inputs[i]);
                NodeTypeTitle(r->from_node, r->from_sock);
                ImGui::TableSetColumnIndex(2);
                ToggleButton(r->value);
            }
            ImGui::PopFont();
        }
        ImGui::EndTable();
    };
}

static void _output_table(
    NRef<Scene> scene, const std::vector<relid>& outputs, sockid id)
{
    if (ImGui::BeginTable("Outputs", 2, ImGuiTableFlags_BordersInner)) {
        ImGui::TableHeader("Outputs");
        ImGui::TableSetupColumn("Socket", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        if (outputs.empty()) {
            TablePair(Field("%u", id),
                ImGui::PushFont(
                    get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
                ImGui::TextColored(ImVec4(0.3, 0.3, 0.3, 1), "DISCONNECTED"));
            ImGui::PopFont();
        }
        for (size_t i = 0; i < outputs.size(); i++) {
            TablePair(Field("%u", id));
            ImGui::PushFont(
                get_font(font_flags_t::BOLD | font_flags_t::NORMAL));
            if (outputs[i] == 0) {
                ImGui::TextColored(ImVec4(0.3, 0.3, 0.3, 1), "DISCONNECTED");
            } else {
                NRef<Rel> r = scene->get_rel(outputs[i]);
                NodeTypeTitle(r->to_node, r->to_sock);
            }
            ImGui::PopFont();
        }
        ImGui::EndTable();
    }
}

} // namespace lcs::ui
