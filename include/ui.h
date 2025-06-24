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

    extern bool is_dark;

    int main(int argc, char* argv[]);
    void before(ImGuiIO& io);
    void after(ImGuiIO& io);
    bool loop(ImGuiIO& io);
    void set_style(ImGuiIO& io, bool init = false);
} // namespace ui
} // namespace lcs
