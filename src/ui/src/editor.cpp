#include "common.h"
#include "imnodes.h"
#include "io.h"
#include "ui.h"
#include "ui/flows.h"
#include "ui/layout.h"
#include "ui/util.h"
#include <imgui.h>
#include <tinyfiledialogs.h>

static bool show_demo_window = true;
ImVec4 clear_color           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float f;

static ImGuiID key_palette;
static ImGuiID key_inspector;
static ImGuiID key_sceneinfo;
static ImGuiID key_console;

namespace lcs::ui {
void before(ImGuiIO&) { ImNodes::CreateContext(); }

bool loop(ImGuiIO& imio)

{
    MenuBar();
    NRef<Scene> scene = io::scene::get();
    new_flow();
    if (scene != nullptr) {
        scene->run();
    }
    SceneInfo(&scene);
    NodeEditor(&scene);
    Inspector(&scene);
    Palette(&scene);
    Console();
    DebugWindow(&scene);

    ImGui::Begin("Font Window");
    {
        ImGui::PushFont(get_font(SMALL | ITALIC));
        ImGui::Text("This is some useful text.");
        ImGui::PopFont();
        ImGui::PushFont(get_font(LARGE | BOLD));
        ImGui::Text("This is some useful text.");
        ImGui::PopFont();
        ImGui::PushFont(get_font(NORMAL));
        ImGui::Text("This is some useful text.");
        ImGui::PopFont();
        ImGui::PushFont(get_font(BOLD | ITALIC));
        ImGui::Text("This is some useful text.");
        ImGui::PopFont();

        ImGui::Checkbox("Demo Window", &show_demo_window);

        ImGui::End();
    }
    RenderNotifications();
    return show_demo_window;
}

void after(ImGuiIO&)
{
    if (io::scene::get() != nullptr) {
        if (io::scene::is_saved()) {
            io::scene::close();
        } else {
            int option = tinyfd_messageBox("Close Scene",
                "You have unsaved changes. Would you like to save your changes "
                "before closing?",
                "yesno", "question", 0);
            if (!option || save_as_flow("Save scene")) {
                io::scene::close();
            }
        }
    }
    ImNodes::DestroyContext();
}
} // namespace lcs::ui
