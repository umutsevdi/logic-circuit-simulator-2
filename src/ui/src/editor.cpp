#include "common.h"
#include "components.h"
#include "configuration.h"
#include "imnodes.h"
#include "ui.h"
#include <imgui.h>
#include <nfd.h>

static bool show_demo_window = true;
ImVec4 clear_color           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float f;

static ImGuiID key_palette;
static ImGuiID key_inspector;
static ImGuiID key_sceneinfo;
static ImGuiID key_console;

namespace lcs::ui {
void before(void)
{
    IMGUI_CHECKVERSION();
    ui::bind_config(ImGui::CreateContext());
    ImGuiIO& imio = ImGui::GetIO();
    (void)imio;
    imio.IniFilename = fs::INI.c_str();

    imio.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    NFD_Init();
    ImNodes::CreateContext();
    set_style(imio, true);
}

bool loop(ImGuiIO& imio)

{
    layout::MenuBar();
    NRef<Scene> scene = tabs::active();
    if (scene != nullptr) {
        scene->run(imio.DeltaTime);
    }
    layout::SceneInfo(&scene);
    layout::NodeEditor(&scene);
    layout::Inspector(&scene);
    layout::Palette(&scene);
    layout::Console();
#ifndef NDEBUG
    layout::DebugWindow(&scene);
    ImGui::ShowDemoWindow(nullptr);
#endif
    RenderNotifications();
    return show_demo_window;
}

void after(ImGuiIO&)
{
    if (tabs::active() != nullptr) {
        if (tabs::is_saved()) {
            tabs::close();
        }
        //      FIXME
        //      else {
        //          int option = tinyfd_messageBox("Close Scene",
        //              "You have unsaved changes. Would you like to save your
        //              changes " "before closing?", "yesno", "question", 0);
        //          if (!option || save_as_flow("Save scene")) {
        //              tabs::scene_close();
        //          }
        //      }
    }
    ImNodes::DestroyContext();
    NFD_Quit();
}
} // namespace lcs::ui
