#include "IconsLucide.h"
#include "common.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/layout.h"
#include "ui/util.h"
#include <imgui.h>
#include <string_view>
namespace lcs::ui {

static ImVec4 _log_color(const LcsTheme& style, LogLevel level)
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
    if (!user_data.console) {
        return;
    }
    const LcsTheme& style = get_active_style();
    if (ImGui::Begin("Console", &user_data.console)) {
        if (IconButton<NORMAL>(ICON_LC_TRASH, "Clear")) {
            lcs::l_clear();
        }
        if (ImGui::BeginTable("##ConsoleTable", 5,
                ImGuiTableFlags_Reorderable | ImGuiTableFlags_BordersInner
                    | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
                    | ImGuiTableColumnFlags_NoResize)) {
            ImGui::TableHeader("##Console");
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(
                "Severity", ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn("Node", ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                "Function", ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                "Message", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            lcs::l_iterate([&style](size_t idx, const Line& l) {
                bool selected = false;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Selectable(("##" + std::to_string(idx)).c_str(),
                    &selected, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::SameLine();
                ImGui::TextUnformatted(l.time_str.begin());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(_log_color(style, l.severity), "%s",
                    l.log_level_str.begin());
                if (!std::string_view { l.obj.data() }.empty()) {
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID(idx);
                    ImGui::TextColored(style.yellow, "%s", l.obj.begin());
                    ImGui::PopID();
                }
                ImGui::TableSetColumnIndex(3);
                ImGui::TextColored(style.cyan, "%s", l.fn.begin());
                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(l.expr.begin());

                if (selected) {
                    std::stringstream buffer {};
                    buffer << l.log_level_str.begin() << '\t'
                           << l.file_line.begin() << '\t' << l.obj.begin()
                           << "\t" << l.fn.begin() << '\t' << l.expr.begin()
                           << std::endl;
                    Toast(ICON_LC_CLIPBOARD_COPY, "Clipboard",
                        "Log message was copied to the clipboard.");
                    ImGui::SetClipboardText(buffer.str().c_str());
                }
            });
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

} // namespace lcs::ui
