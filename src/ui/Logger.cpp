
#include "IconsLucide.h"
#include "common.h"
#include "imgui.h"
#include "ui/components.h"
#include "ui/util.h"
#include <sstream>
namespace lcs::ui {

static bool is_open;

void Logger(void)
{
    std::ostringstream& flog = std::get<std::ostringstream>(FLOG);

    ImGui::Begin("Logger", &is_open);
    ImGui::InputTextMultiline("##LOG", flog.str().data(), flog.str().size(),
        ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_ReadOnly);
    if (IconButton<NORMAL>(ICON_LC_TRASH, "Clear")) {
        flog.clear();
    }
    ImGui::End();
}

void OpenLogger() { is_open = true; }

} // namespace lcs::ui
