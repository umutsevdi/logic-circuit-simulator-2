#include "ui/components.h"

namespace lcs::ui {
void ShowIcon(FontFlags size, const char* icon)
{
    ImGui::PushFont(get_font(FontFlags::ICON | size));
    ImGui::Text("%s", icon);
    ImGui::PopFont();
}
} // namespace lcs::ui
