#include "ui/components.h"

namespace lcs::ui {
void ShowIcon(font_flags_t size, const char* icon)
{
    ImGui::PushFont(get_font(font_flags_t::ICON | size));
    ImGui::Text("%s", icon);
    ImGui::PopFont();
}
} // namespace lcs::ui
