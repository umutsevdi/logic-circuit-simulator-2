#include "IconsLucide.h"
#include "common.h"
#include "components.h"
#include "configuration.h"
#include "ui.h"
#include <imgui.h>
#include <string_view>
namespace lcs::ui::layout {

static ImVec4 _log_color(const LcsTheme& style, Message::Severity level)
{
    switch (level) {
    case Message::DEBUG: return style.blue_bright;
    case Message::INFO: return style.green;
    case Message::WARN: return style.magenta;
    case Message::FATAL:;
    case Message::ERROR: return style.red;
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
            lcs::fs::clear_log();
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

            lcs::fs::logs_for_each([&style](size_t idx, const Message& l) {
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
            ImGui::SetScrollHereY(1.0f);
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

} // namespace lcs::ui::layout
