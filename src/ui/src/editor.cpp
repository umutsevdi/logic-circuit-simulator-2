#include <imgui.h>
#include <imnodes.h>
#include <nfd.h>
#include "common.h"
#include "configuration.h"
#include "ui.h"

namespace lcs::ui {

static std::string _inistr;
void before(void)
{
    IMGUI_CHECKVERSION();
    ui::bind_config(ImGui::CreateContext());
    ImGuiIO& imio = ImGui::GetIO();
    (void)imio;
    _inistr          = fs::INI.string();
    imio.IniFilename = _inistr.c_str();

    imio.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    NFD_Init();
    ImNodes::CreateContext();
    set_style(imio, true);
}

static bool keep_loop = true;
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
    return keep_loop;
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
