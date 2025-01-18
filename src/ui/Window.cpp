#include "Window.h"
#include "Application.h"
#include <glibmm.h>
#include <iostream>

namespace lcs::ui {

AppWindow::AppWindow()
    : button_("Click Me!")
    , m_Box_Top(Gtk::Orientation::VERTICAL)
    , m_Box1(Gtk::Orientation::VERTICAL, 10)
    , m_Box2(Gtk::Orientation::VERTICAL, 10)
{
    set_title("Logic Circuit Simulator");
    signal_hide().connect([this]() {
        this->show();
        std::cout << "I am hidden" << std::endl;
    });

    set_default_size(400, 300);
    set_default_widget(m_Box_Top);
    m_Box_Top.append(button_);
    set_child(m_Box_Top);
    set_entry();

    g_object_set(gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme", TRUE, NULL);

    button_.signal_clicked().connect([this]() {
        std::cout << this->entry.get_text() << std::endl;
        Gtk::MessageDialog dialog(*this, "Hello from GTK4!");
        dialog.set_visible();
        dialog.set_secondary_text("You clicked the button.");
    }

    );

    init_menubar();

    init_box();
}

void AppWindow::init_box() { }

void AppWindow::init_menubar() { }

void AppWindow::set_entry()
{
    entry.set_max_length(50);
    entry.set_text("hello");
    entry.set_text(entry.get_text() + " world");
    entry.select_region(0, entry.get_text_length());
    entry.set_expand(true);
    m_Box_Top.append(entry);
}

int start(int argc, char* argv[])
{
    auto app = Gtk::Application::create("org.example.gtkmm");
    return app->make_window_and_run<lcs::ui::AppWindow>(argc, argv);
}
}
