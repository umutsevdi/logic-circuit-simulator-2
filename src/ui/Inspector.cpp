#include "IconsLucide.h"
#include "imnodes.h"
#include "io.h"
#include "ui/components.h"
#include <imgui.h>
#include <cmath>

namespace lcs::ui {

static void _disconnect_button_tooltip();
static void _input_table(NRef<Scene>, const std::vector<relid>&);
static void _output_table(NRef<Scene>, const std::vector<relid>&);
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
        L_INFO("Selecting: " << node);
        _inspector_tab(&scene, node);
    }
    ImGui::End();
}

static void _inspector_tab(NRef<Scene> scene, Node node)
{
    const static ImVec2 __table_l_size = ImGui::CalcTextSize("SOCKET COUNT");

    ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::LARGE));
    NodeTypeTitle(node);
    ImGui::PopFont();
    if (node.type != COMPONENT_OUTPUT && node.type != COMPONENT_INPUT) {
        ImGui::SameLine();
        if (IconButton<NORMAL>(ICON_LC_TRASH_2, "")) {
            ImNodes::ClearNodeSelection(node.numeric());
            scene->remove_node(node);
            return;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("Delete Node");
        }
    }
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
    } else {
        TablePair(
            Field("Value"), State old = _node->get();
            if (State t = ToggleButton(old, true);
                t != old) { _node->toggle(); });
    }

    TablePair(Field("Outputs"));
    if (ImGui::BeginTable("InputList", 3, ImGuiTableFlags_BordersInner)) {
        ImGui::TableSetupColumn("Socket", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        TablePair(Field("1"));
        _output_table(&scene, _node->output);
        ImGui::TableSetColumnIndex(2);
        ToggleButton(_node->get());
        ImGui::EndTable();
    }
    ImGui::EndTable();
}

static void _inspector_output_node(NRef<Scene> scene, Node node)
{
    auto _node = scene->get_node<OutputNode>(node);
    TablePair(Field("Value"), ToggleButton(_node->get()));
    std::vector<relid> in;
    in.push_back(_node->input);
    TablePair(Field("Inputs"));
    _input_table(&scene, in);
    ImGui::EndTable();
}

static void _inspector_gate_node(NRef<Scene> scene, Node node)
{
    const static ImVec2 __selector_size = ImGui::CalcTextSize("-000000000000");

    auto _node = scene->get_node<GateNode>(node);
    TablePair(Field("Value"), ToggleButton(_node->get()));
    TablePair(
        Field("Gate Type"), ImGui::Text("%s", GateType_to_str(_node->type())));
    ImGui::BeginDisabled(_node->type() == GateType::NOT);
    TablePair(Field("Socket Count"));
    size_t socket_count = _node->inputs.size();
    size_t inc          = 1;
    ImGui::PushItemWidth(__selector_size.x);
    if (ImGui::InputScalar("##SocketCount", ImGuiDataType_U8, &socket_count,
            &inc, &inc, nullptr)) {
        bool keep = true;
        while (socket_count != _node->inputs.size() && keep) {
            if (socket_count > _node->inputs.size()) {
                keep = _node->increment();
            } else {
                keep = _node->decrement();
            }
        }
    }
    ImGui::EndDisabled();
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    TablePair(Field("Inputs"));
    _input_table(&scene, _node->inputs);

    TablePair(Field("Outputs"));
    if (ImGui::BeginTable("InputList", 3, ImGuiTableFlags_BordersInner)) {
        ImGui::TableSetupColumn("Socket", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        TablePair(Field("1"));
        _output_table(&scene, _node->output);
        ImGui::TableSetColumnIndex(2);
        ToggleButton(_node->get());
        ImGui::EndTable();
    }
    ImGui::EndTable();
}

static void _inspector_component_node(NRef<Scene> scene, Node node)
{
    auto _node = scene->get_node<ComponentNode>(node);
    TablePair(Field("Value"), ToggleButton(_node->get()));
    TablePair(Field("Inputs"));
    _input_table(&scene, _node->inputs);
    TablePair(Field("Outputs"));
    for (auto out : _node->outputs) {
        if (ImGui::BeginTabBar(std::to_string(out.first).c_str())) {
            _output_table(&scene, out.second);
        }
    }
    ImGui::EndTable();
}

static void _inspector_component_context_node(NRef<Scene> scene, Node node)
{
    ComponentContext& ctx = scene->component_context.value();
    if (node.type == COMPONENT_INPUT) {
        TablePair(Field("Outputs"));
        if (ImGui::BeginTable(
                "InputsComponentInputList", 3, ImGuiTableFlags_BordersInner)) {
            ImGui::TableSetupColumn("Socket", ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                "Connection", ImGuiTableColumnFlags_WidthStretch);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                "Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (size_t i = 0; i < ctx.inputs.size(); i++) {
                TablePair(Field("%zu", i + 1));
                _output_table(&scene, ctx.inputs[i]);
                ImGui::TableSetColumnIndex(2);
                State value = ctx.get_value(ctx.get_input(i));
                ImGui::PushID((std::to_string(i) + "btn").c_str());
                if (State new_value = ToggleButton(value, true);
                    value != new_value) {
                    ctx.set_value(ctx.get_input(i), new_value);
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    } else {
        TablePair(Field("Inputs"));
        _input_table(&scene, scene->component_context->outputs);
    }
    ImGui::EndTable();
}

static void _input_table(NRef<Scene> scene, const std::vector<relid>& inputs)
{
    if (ImGui::BeginTable("Inputs", 4, ImGuiTableFlags_BordersInner)) {
        ImGui::TableSetupColumn("Socket", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "##Disconnect", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < inputs.size(); i++) {
            TablePair(Field("%zu", i + 1));
            ImGui::SameLine();
            ImGui::PushFont(
                get_font(FontFlags::BOLD | FontFlags::NORMAL));
            State value = State::DISABLED;
            if (inputs[i] != 0) {
                NRef<Rel> r = scene->get_rel(inputs[i]);
                NodeTypeTitle(r->from_node, r->from_sock);
                value = r->value;
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::BeginDisabled(inputs[i] == 0);
            if (IconButton<NORMAL>(ICON_LC_CIRCLE_SLASH_2, "")) {
                scene->disconnect(inputs[i]);
                io::scene::notify_change();
            }
            ImGui::EndDisabled();
            ImGui::TableSetColumnIndex(3);
            ToggleButton(value);
            ImGui::PopFont();
        }
        if (ImGui::TableGetHoveredColumn() == 2) {
            _disconnect_button_tooltip();
        };
        ImGui::EndTable();
    };
}

static void _output_table(NRef<Scene> scene, const std::vector<relid>& outputs)
{
    if (ImGui::BeginTable("Outputs", 2, ImGuiTableFlags_BordersInner)) {
        ImGui::TableHeader("Outputs");
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("Disconnect", ImGuiTableColumnFlags_WidthFixed);
        if (outputs.empty()) {
            ImGui ::TableNextRow();
            ImGui ::TableSetColumnIndex(1);
            ImGui::BeginDisabled(true);
            IconButton<NORMAL>(ICON_LC_CIRCLE_SLASH_2, "");
            ImGui::EndDisabled();
        }

        for (size_t i = 0; i < outputs.size(); i++) {
            ImGui ::TableNextRow();
            ImGui ::TableSetColumnIndex(0);
            ImGui::PushFont(
                get_font(FontFlags::BOLD | FontFlags::NORMAL));
            if (outputs[i] == 0) {
                ImGui::TextColored(ImVec4(0.3, 0.3, 0.3, 1), "DISCONNECTED");
            } else {
                NRef<Rel> r = scene->get_rel(outputs[i]);
                NodeTypeTitle(r->to_node, r->to_sock);
            }
            ImGui::PopFont();
            ImGui ::TableSetColumnIndex(1);

            ImGui::BeginDisabled(outputs[i] == 0);
            if (IconButton<NORMAL>(ICON_LC_CIRCLE_SLASH_2, "")) {
                scene->disconnect(outputs[i]);
                io::scene::notify_change();
            }
            ImGui::EndDisabled();
        }
        if (ImGui::TableGetHoveredColumn() == 1) {
            _disconnect_button_tooltip();
        };
        ImGui::EndTable();
    }
}

static void _disconnect_button_tooltip()
{
    if (ImGui::BeginTooltip()) {
        ImGui::Text("Disconnect relation");
        ImGui::EndTooltip();
    }
}
} // namespace lcs::ui
  //
