#pragma once
/*******************************************************************************
 * \file
 * File: include/ui/flows.h
 * Created: 06/19/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

namespace lcs::ui {
bool save_as_flow(const char* title);
void close_flow(void);
void open_flow(void);

extern bool new_flow_show;
void new_flow(void);
} // namespace lcs::ui
