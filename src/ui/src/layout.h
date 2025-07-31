#pragma once
/*******************************************************************************
 * \file
 * File: /ui/layout.h
 * Created: 06/01/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "core.h"
#include <imgui.h>

namespace lcs::ui {

extern bool is_dark;
void MenuBar(void);
void Palette(NRef<Scene>);
void Inspector(NRef<Scene>);
void NodeEditor(NRef<Scene> scene);
int _input_text_callback(ImGuiInputTextCallbackData*);
void Profile(const std::string& name);
void SceneInfo(NRef<Scene>);
void Console(void);
void DebugWindow(NRef<Scene>);

void RenderNotifications(void);

void before(ImGuiIO& io);
void after(ImGuiIO& io);
bool loop(ImGuiIO& io);
void set_style(ImGuiIO& io, bool init = false);
} // namespace lcs::ui
