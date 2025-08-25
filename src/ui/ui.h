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

#include <imgui.h>
#include "core.h"
namespace lcs::ui {
int main(int, char**);
void before(void);
void after(ImGuiIO& io);
bool loop(ImGuiIO& io);
void RenderNotifications(void);

inline ImVec4 v4mul(ImVec4 vec, float pct)
{
    return ImVec4(vec.x * pct, vec.y * pct, vec.z * pct, vec.w);
}

inline ImVec4 v4mul(ImVec4 vec, float pct, float alpha)
{
    return ImVec4(vec.x * pct, vec.y * pct, vec.z * pct, vec.w * alpha);
}

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
    void LoginWindow(bool& df_show);
} // namespace popup

namespace dialog {
    LCS_ERROR open_file(void);
    LCS_ERROR save_file_as(void);
} // namespace dialog
} // namespace lcs::ui
