#pragma once
/*******************************************************************************
 * \file
 * File: ui/ui.h
 * Created: 08/12/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "core.h"
#include <imgui.h>
namespace lcs::ui {
int main(int, char**);
void before(void);
void after(ImGuiIO& io);
bool loop(ImGuiIO& io);
void RenderNotifications(void);

#define V4MUL(vec, pct, ...)                                                   \
    ImVec4((vec).x*(pct), (vec).y*(pct), (vec).z*(pct), (vec).w __VA_ARGS__)
#define DL(_a, _b) (t.is_dark ? (_a) : (_b))

void set_style(ImGuiIO& io, bool init = false);

namespace layout {
    void MenuBar(void);
    void Palette(NRef<Scene>);
    void Inspector(NRef<Scene>);
    void NodeEditor(NRef<Scene> scene);
    int _input_text_callback(ImGuiInputTextCallbackData*);
    void Profile(const std::string& name);
    void SceneInfo(NRef<Scene>);
    void Console(void);
    void DebugWindow(NRef<Scene>);
} // namespace layout

namespace popup {
    void Preferences(bool& pref_show);
    void LoginWindow(bool& df_show);
} // namespace popup

namespace dialog {
    LCS_ERROR open_file(void);
    LCS_ERROR save_file_as(void);
} // namespace dialog
} // namespace lcs::ui
