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

enum FontFlags {
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

enum Style {
    // Light Themes
    SEOUL256_LIGHT,
    ACME,
    GRUVBOX_LIGHT,
    ONE_LIGHT,
    // Dark Themes
    SEASHELLS,
    XTERM,
    GRUVBOX_DARK,
    ONE_DARK,

    STYLE_S
};

struct LcsStyle {
    ImVec4 bg;
    ImVec4 fg;
    ImVec4 black;
    ImVec4 red;
    ImVec4 green;
    ImVec4 yellow;
    ImVec4 blue;
    ImVec4 magenta;
    ImVec4 cyan;
    ImVec4 white;
    ImVec4 black_bright;
    ImVec4 red_bright;
    ImVec4 green_bright;
    ImVec4 yellow_bright;
    ImVec4 blue_bright;
    ImVec4 magenta_bright;
    ImVec4 cyan_bright;
    ImVec4 white_bright;
    bool is_dark = false;
};
const LcsStyle& get_style(Style);
const LcsStyle& get_active_style();

inline ImVec4 NodeType_to_color(mode_t type)
{
    const LcsStyle& style = get_active_style();
    switch (type) {
    case NodeType::GATE: return style.red;
    case NodeType::INPUT: return style.green;
    case NodeType::OUTPUT: return style.yellow;
    case NodeType::COMPONENT: return style.magenta;
    default: return style.cyan;
    }
}

void SceneType(NRef<Scene>);

int hash_pair(Node node, sockid sock, bool is_out);

} // namespace lcs::ui
