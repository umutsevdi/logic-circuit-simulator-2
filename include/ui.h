#pragma once
/*******************************************************************************
 * \file
 * File: include/ui.h
 * Created: 04/29/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <imgui.h>

namespace lcs {
namespace ui {
    enum font_flags_t {
        /** Size Flag: SMALL */
        SMALL = 0b00000,
        /** Size Flag: NORMAL */
        NORMAL = 0b00001,
        /** Size Flag: LARGE */
        LARGE = 0b00010,
        /** Format Flag: REGULAR */
        REGULAR = 0b00000,
        /** Format Flag: ITALIC */
        ITALIC = 0b01000,
        /** Format Flag: BOLD */
        BOLD = 0b10000,

        // Small/Regular/Large * BOLD|ITALIC|REGULAR|BOLD-ITALIC
        FONT_S = BOLD | ITALIC | LARGE
    };

    extern bool is_dark;
    int main(int argc, char* argv[]);
    void before(ImGuiIO& io);
    void after(ImGuiIO& io);
    bool loop(ImGuiIO& io);
    void set_style(ImGuiIO& io, int alpha, bool& is_dark);
    ImFont* get_font(int attributes);

} // namespace ui
} // namespace lcs
