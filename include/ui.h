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
        SMALL  = 0b00000,
        NORMAL = 0b00001,
        LARGE  = 0b00010,

        REGULAR = 0b00000,
        ITALIC  = 0b01000,
        BOLD    = 0b10000,

        // Small/Regular/Large * BOLD|ITALIC|REGULAR|BOLD-ITALIC
        FONT_S = BOLD | ITALIC | LARGE
    };

    extern bool is_dark;
    int main(int argc, char* argv[]);
    bool loop(ImGuiIO& io);
    void set_style(ImGuiIO& io, int alpha, bool& is_dark);
    ImFont* get_font(int attributes);
} // namespace ui
} // namespace lcs
