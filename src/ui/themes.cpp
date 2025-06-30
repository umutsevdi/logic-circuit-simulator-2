#include "ui/util.h"

constexpr ImVec4 to_imvec4(int clr)
{
    return ImVec4(((clr >> 16) & 0xFF) / 255.0f, ((clr >> 8) & 0xFF) / 255.0f,
        (clr & 0xFF) / 255.0f, 1.0f);
}

namespace lcs ::ui {
void init_StyleInfo(void) { }

const LcsStyle COLORMAP[Style::STYLE_S] = {
    // Style::SEOUL256_LIGHT
    LcsStyle {
        // Seoul256 Light Alacritty Colors
        // name: seoul256-light
        // inspired by: https://github.com/junegunn/seoul256.vim
        .bg             = to_imvec4(0xdadada),
        .fg             = to_imvec4(0x4e4e4e),
        .black          = to_imvec4(0x4e4e4e),
        .red            = to_imvec4(0xaf005f),
        .green          = to_imvec4(0x5f875f),
        .yellow         = to_imvec4(0xaf5f00),
        .blue           = to_imvec4(0x007173),
        .magenta        = to_imvec4(0x870087),
        .cyan           = to_imvec4(0x008787),
        .white          = to_imvec4(0xe4e4e4),
        .black_bright   = to_imvec4(0x626262),
        .red_bright     = to_imvec4(0xd70087),
        .green_bright   = to_imvec4(0x87af87),
        .yellow_bright  = to_imvec4(0xdfbc72),
        .blue_bright    = to_imvec4(0x5fafd7),
        .magenta_bright = to_imvec4(0xaf5fff),
        .cyan_bright    = to_imvec4(0x00afaf),
        .white_bright   = to_imvec4(0xffffff),
        .is_dark        = false,
    },
    // Style::ACME
    LcsStyle {
        // Colors (acme)
        .bg             = to_imvec4(0xffffea),
        .fg             = to_imvec4(0x000000),
        .black          = to_imvec4(0x101010),
        .red            = to_imvec4(0xaf5f00),
        .green          = to_imvec4(0xcccc7c),
        .yellow         = to_imvec4(0xffff5f),
        .blue           = to_imvec4(0xaeeeee),
        .magenta        = to_imvec4(0x505050),
        .cyan           = to_imvec4(0xafffd7),
        .white          = to_imvec4(0xfcfcce),
        .black_bright   = to_imvec4(0x101010),
        .red_bright     = to_imvec4(0xaf5f00),
        .green_bright   = to_imvec4(0xcccc7c),
        .yellow_bright  = to_imvec4(0xffff5f),
        .blue_bright    = to_imvec4(0xaeeeee),
        .magenta_bright = to_imvec4(0x505050),
        .cyan_bright    = to_imvec4(0xafffd7),
        .white_bright   = to_imvec4(0xfcfcce),
        .is_dark        = false,
    },
    // Style::GRUVBOX_LIGHT
    LcsStyle {
        // Colors (Gruvbox light)
        .bg             = to_imvec4(0xf9f5d7),
        .fg             = to_imvec4(0x3c3836),
        .black          = to_imvec4(0xfbf1c7),
        .red            = to_imvec4(0xcc241d),
        .green          = to_imvec4(0x98971a),
        .yellow         = to_imvec4(0xd79921),
        .blue           = to_imvec4(0x458588),
        .magenta        = to_imvec4(0xb16286),
        .cyan           = to_imvec4(0x689d6a),
        .white          = to_imvec4(0x7c6f64),
        .black_bright   = to_imvec4(0x928374),
        .red_bright     = to_imvec4(0x9d0006),
        .green_bright   = to_imvec4(0x79740e),
        .yellow_bright  = to_imvec4(0xb57614),
        .blue_bright    = to_imvec4(0x076678),
        .magenta_bright = to_imvec4(0x8f3f71),
        .cyan_bright    = to_imvec4(0x427b58),
        .white_bright   = to_imvec4(0x3c3836),
        .is_dark        = false,
    },
    // Style::ONE_LIGHT]
    LcsStyle {
        // Colors (One Light)
        .bg             = to_imvec4(0xf8f8f8),
        .fg             = to_imvec4(0x2a2b33),
        .black          = to_imvec4(0x000000),
        .red            = to_imvec4(0xde3d35),
        .green          = to_imvec4(0x3e953a),
        .yellow         = to_imvec4(0xd2b67b),
        .blue           = to_imvec4(0x2f5af3),
        .magenta        = to_imvec4(0xa00095),
        .cyan           = to_imvec4(0x3e953a),
        .white          = to_imvec4(0xbbbbbb),
        .black_bright   = to_imvec4(0x000000),
        .red_bright     = to_imvec4(0xde3d35),
        .green_bright   = to_imvec4(0x3e953a),
        .yellow_bright  = to_imvec4(0xd2b67b),
        .blue_bright    = to_imvec4(0x2f5af3),
        .magenta_bright = to_imvec4(0xa00095),
        .cyan_bright    = to_imvec4(0x3e953a),
        .white_bright   = to_imvec4(0xffffff),
        .is_dark        = false,
    },
    // Style::SEASHELLS
    LcsStyle {
        // Colors (SeaShells)
        // Source
        // https//raw.githubusercontent.com/mbadolato/iTerm2-Color-Schemes/master/schemes/SeaShells.itermcolors
        .bg             = to_imvec4(0x061923),
        .fg             = to_imvec4(0xe5c49e),
        .black          = to_imvec4(0x1d485f),
        .red            = to_imvec4(0xdb662d),
        .green          = to_imvec4(0x008eab),
        .yellow         = to_imvec4(0xfeaf3c),
        .blue           = to_imvec4(0x255a62),
        .magenta        = to_imvec4(0x77dbf4),
        .cyan           = to_imvec4(0x5fb1c2),
        .white          = to_imvec4(0xe5c49e),
        .black_bright   = to_imvec4(0x545d65),
        .red_bright     = to_imvec4(0xdd998a),
        .green_bright   = to_imvec4(0x739da8),
        .yellow_bright  = to_imvec4(0xfedaae),
        .blue_bright    = to_imvec4(0x0bc7e3),
        .magenta_bright = to_imvec4(0xc6e8f1),
        .cyan_bright    = to_imvec4(0x97b9c0),
        .white_bright   = to_imvec4(0xffe9d7),
        .is_dark        = true,
    },
    // Style::XTERM
    LcsStyle {
        // XTerm's default colors
        .bg             = to_imvec4(0x000000),
        .fg             = to_imvec4(0xffffff),
        .black          = to_imvec4(0x000000),
        .red            = to_imvec4(0xcd0000),
        .green          = to_imvec4(0x00cd00),
        .yellow         = to_imvec4(0xcdcd00),
        .blue           = to_imvec4(0x0000ee),
        .magenta        = to_imvec4(0xcd00cd),
        .cyan           = to_imvec4(0x00cdcd),
        .white          = to_imvec4(0xe5e5e5),
        .black_bright   = to_imvec4(0x7f7f7f),
        .red_bright     = to_imvec4(0xff0000),
        .green_bright   = to_imvec4(0x00ff00),
        .yellow_bright  = to_imvec4(0xffff00),
        .blue_bright    = to_imvec4(0x5c5cff),
        .magenta_bright = to_imvec4(0xff00ff),
        .cyan_bright    = to_imvec4(0x00ffff),
        .white_bright   = to_imvec4(0xffffff),
        .is_dark        = true,
    },
    // Style::GRUVBOX_DARK
    LcsStyle {
        // Colors (Gruvbox dark)
        .bg             = to_imvec4(0x282828),
        .fg             = to_imvec4(0xebdbb2),
        .black          = to_imvec4(0x282828),
        .red            = to_imvec4(0xcc241d),
        .green          = to_imvec4(0x98971a),
        .yellow         = to_imvec4(0xd79921),
        .blue           = to_imvec4(0x458588),
        .magenta        = to_imvec4(0xb16286),
        .cyan           = to_imvec4(0x689d6a),
        .white          = to_imvec4(0xa89984),
        .black_bright   = to_imvec4(0x928374),
        .red_bright     = to_imvec4(0xfb4934),
        .green_bright   = to_imvec4(0xb8bb26),
        .yellow_bright  = to_imvec4(0xfabd2f),
        .blue_bright    = to_imvec4(0x83a598),
        .magenta_bright = to_imvec4(0xd3869b),
        .cyan_bright    = to_imvec4(0x8ec07c),
        .white_bright   = to_imvec4(0xebdbb2),
        .is_dark        = true,
    },
    // Style::ONE_DARK
    LcsStyle {
        // Colors (One Dark)
        .bg             = to_imvec4(0x282c34),
        .fg             = to_imvec4(0xabb2bf),
        .black          = to_imvec4(0x1e2127),
        .red            = to_imvec4(0xe06c75),
        .green          = to_imvec4(0x98c379),
        .yellow         = to_imvec4(0xd19a66),
        .blue           = to_imvec4(0x61afef),
        .magenta        = to_imvec4(0xc678dd),
        .cyan           = to_imvec4(0x56b6c2),
        .white          = to_imvec4(0xabb2bf),
        .black_bright   = to_imvec4(0x5c6370),
        .red_bright     = to_imvec4(0xe06c75),
        .green_bright   = to_imvec4(0x98c379),
        .yellow_bright  = to_imvec4(0xd19a66),
        .blue_bright    = to_imvec4(0x61afef),
        .magenta_bright = to_imvec4(0xc678dd),
        .cyan_bright    = to_imvec4(0x56b6c2),
        .white_bright   = to_imvec4(0xffffff),
        .is_dark        = true,
    },
};
const LcsStyle& get_style(Style s) { return COLORMAP[s]; }
} // namespace lcs::ui
