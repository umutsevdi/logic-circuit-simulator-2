#include "common.h"
#include "ui/components.h"
#include "ui/util.h"
#include <imgui.h>
#include <string_view>
namespace lcs::ui {

static bool is_open = true;

static ImVec4 log_color(const LcsStyle& style, LogLevel level)
{
    switch (level) {
    case DEBUG: return style.blue_bright;
    case INFO: return style.green;
    case WARN: return style.magenta;
    case ERROR: return style.red;
    }
};

void Console(void)
{
    if (!is_open) {
        return;
    }
    const LcsStyle& style = get_active_style();
    ImGui::Begin("Console", &is_open);
    ImGui::BeginTable("##Logger", 4,
        ImGuiTableFlags_Reorderable | ImGuiTableFlags_BordersInner
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY);
    ImGui::TableHeader("##Console");
    ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed);
    ImGui::NextColumn();
    ImGui::TableSetupColumn("Node", ImGuiTableColumnFlags_WidthFixed);
    ImGui::NextColumn();
    ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthFixed);
    ImGui::NextColumn();
    ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    lcs::l_iterate([&style](size_t idx, const Line& l) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(
            log_color(style, l.severity), "%s", l.log_level_str.begin());
        if (!std::string_view { l.obj.data() }.empty()) {
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(idx);
            if (l.node.id != 0 || l.node.type != 0) {
                NodeTypeTitle(l.node);
            } else {
                ImGui::TextColored(style.yellow, "%s", l.obj.begin());
            }
            ImGui::PopID();
        }
        ImGui::TableSetColumnIndex(2);
        ImGui::TextColored(style.cyan, "%s", l.fn.begin());
        ImGui::TableSetColumnIndex(3);
        ImGui::TextUnformatted(l.expr.begin());
    });
    ImGui::EndTable();
    ImGui::End();
}

void OpenLogger() { is_open = true; }

} // namespace lcs::ui
