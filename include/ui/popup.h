#pragma once
/*******************************************************************************
 * \file
 * File: include/ui/popup.h
 * Created: 07/07/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <imgui.h>

namespace lcs::ui {
void Preferences(bool& pref_show);
void LoginWindow(bool& df_show);
} // namespace lcs::ui
