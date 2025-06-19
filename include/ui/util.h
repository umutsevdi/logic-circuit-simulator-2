#pragma once
/*******************************************************************************
 * \file
 * File: /home/umutsevdi/src/dev/lc-simulator-2/include/ui/util.h
 * Created: 06/06/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "core.h"
#include <imgui.h>
namespace lcs::ui {

enum font_flags_t {
    /** Size Flag: SMALL */
    SMALL = 0b00000,
    /** Size Flag: NORMAL */
    NORMAL = 0b00001,
    /** Size Flag: LARGE */
    LARGE = 0b00010,
    /** Size Flag: ULTRA: Only for icons */
    ULTRA = 0b00100,
    /** Format Flag: REGULAR */
    REGULAR = 0b00000,
    /** Format Flag: ITALIC */
    ITALIC = 0b001000,
    /** Format Flag: BOLD */
    BOLD = 0b010000,
    /** Format Flag: ICON */
    ICON = 0b100000,

    // Small/Regular/Large * BOLD|ITALIC|REGULAR|BOLD-ITALIC
    FONT_S = ICON | ULTRA | 1
};

int encode_pair(Node node, sockid sock, bool is_out);
Node decode_pair(int pair_code, sockid* sock = nullptr, bool* is_out = nullptr);

ImFont* get_font(int attributes);
float get_font_size(int attributes);
constexpr ImVec4 NodeType_to_color(mode_t type)
{
    switch (type) {
    case NodeType::GATE: return ImVec4(0, 1, 1, 1);
    case NodeType::INPUT: return ImVec4(0.3, 0.3, 0.7, 1);
    case NodeType::OUTPUT: return ImVec4(0.6, 0.0, 0.6, 1);
    case NodeType::COMPONENT: return ImVec4(0.3, 0.7, 0.3, 1);
    default: return ImVec4(0.8, 0.6, 0.1, 1);
    }
}

void SceneType(NRef<Scene>);

int hash_pair(Node node, sockid sock, bool is_out);

} // namespace lcs::ui
