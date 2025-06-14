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

#include <imgui.h>

#include "core.h"

namespace lcs {
class Scene;
}
namespace lcs::ui {
void MenuBar(void);
void TabWindow(void);
void Palette(void);
void Inspector(NRef<Scene>);
void NodeEditor(NRef<Scene> scene);
int _input_text_callback(ImGuiInputTextCallbackData*);
void Profile(const std::string& name);
} // namespace lcs::ui
