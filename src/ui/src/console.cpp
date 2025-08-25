#include <IconsLucide.h>
#include <imgui.h>
#include <string_view>
#include "common.h"
#include "components.h"
#include "configuration.h"
#include "ui.h"
namespace lcs::ui::layout {

static ImVec4 _log_color(const LcsTheme& style, Message::Severity level)
{
    switch (level) {
    case Message::DEBUG: return style.blue_bright;
    case Message::INFO: return style.green;
    case Message::WARN: return style.magenta;
    case Message::FATAL:;
    case Message::ERROR: return style.red;
    default: return style.fg;
    }
};

void Console(void)
{
    static bool auto_scroll = true;
    if (!user_data.console) {
        return;
    }
    const LcsTheme& style = get_active_style();
    std::string title     = std::string { _("Console") } + "###Console";
    if (ImGui::Begin(title.c_str(), &user_data.console)) {
        HINT(nullptr, _("Console"), _("Displays warning messages."));
        if (IconButton<NORMAL>(ICON_LC_TRASH, _("Clear"))) {
            lcs::fs::clear_log();
        }
        HINT(nullptr, _("Clear"), _("Clears all log messages."));
        ImGui::SameLine();
        ImGui::Checkbox(_("Auto-scroll"), &auto_scroll);
        HINT(nullptr, _("Auto-scroll"),
            _("When enabled, the scrollbar will be locked to the bottom \n"
              "to display live updates."));
        if (ImGui::BeginTable("##ConsoleTable", 5,
                ImGuiTableFlags_Reorderable | ImGuiTableFlags_BordersInner
                    | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
                    | ImGuiTableColumnFlags_NoResize)) {
            ImGui::TableHeader("##Console");
            ImGui::TableSetupColumn(
                _("Time"), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(
                _("Severity"), ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                _("Module"), ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                _("Function"), ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                _("Message"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            lcs::fs::logs_for_each([&style](size_t idx, const Message& l) {
                bool selected = false;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Selectable(("##" + std::to_string(idx)).c_str(),
                    &selected, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::SameLine();
                ImGui::TextUnformatted(l.time_str.data());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(_log_color(style, l.severity), "%s",
                    l.log_level_str.data());
                if (!std::string_view { l.obj.data() }.empty()) {
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID(idx);
                    ImGui::TextColored(style.yellow, "%s", l.obj.data());
                    ImGui::PopID();
                }
                ImGui::TableSetColumnIndex(3);
                ImGui::TextColored(style.cyan, "%s", l.fn.data());
                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(l.expr.data());

                if (selected) {
                    std::stringstream buffer {};
                    buffer << l.log_level_str.data() << '\t'
                           << l.file_line.data() << '\t' << l.obj.data() << "\t"
                           << l.fn.data() << '\t' << l.expr.data() << std::endl;
                    Toast(ICON_LC_CLIPBOARD_COPY, _("Clipboard"),
                        _("Message is copied to the clipboard."));
                    ImGui::SetClipboardText(buffer.str().c_str());
                }
            });
            if (auto_scroll) {
                ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

} // namespace lcs::ui::layout
