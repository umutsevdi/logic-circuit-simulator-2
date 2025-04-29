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
    enum font_t {
        SMALL,
        REGULAR,
        LARGE,
        W2,

        FONT_S
    };

    extern bool is_dark;
    int main(int argc, char* argv[]);
    bool loop(ImGuiIO& io);
    void set_style(ImGuiIO& io, int alpha, bool& is_dark);
} // namespace ui
} // namespace lcs
