#pragma once
/*******************************************************************************
 * \file
 * File: ui/Window.h
 * Created: 02/05/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "gtkmm/actionbar.h"
#include "gtkmm/entry.h"
#include "gtkmm/listbox.h"
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>

#include <gtkmm/combobox.h>
#include <gtkmm/paned.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/window.h>

namespace lcs::ui {

class AppWindow : public Gtk::Window {
public:
    AppWindow();

    void init_menubar();
    void init_toolbar();
    void init_box();

    void set_entry();

private:
    Gtk::Entry entry;
    Gtk::Button button_;
    Gtk::HeaderBar headerBar;
    Gtk::Label firstLabel;
    Gtk::Label secondLabel;

    Gtk::Box m_Box_Top, m_Box1, m_Box2;
};

} // namespace lcs::ui
