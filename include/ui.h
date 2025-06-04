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

#include "core.h"

namespace lcs {
namespace ui {
    int hash_pair(Node node, sockid sock, bool is_out);

    extern bool is_dark;

    int main(int argc, char* argv[]);
    void before(ImGuiIO& io);
    void after(ImGuiIO& io);
    bool loop(ImGuiIO& io);
    void set_style(ImGuiIO& io, int alpha, bool& is_dark);

    bool save_as_flow(const char* title);
    void close_flow(void);

} // namespace ui
} // namespace lcs
