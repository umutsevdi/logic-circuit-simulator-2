#include "common.h"
#include "io.h"
#include "net.h"
#include "ui.h"
#include "ui/configuration.h"
#include "ui/flows.h"
#include "ui/layout.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>

static bool show_demo_window = true;
ImVec4 clear_color           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float f;

namespace lcs::ui {

Style current;
void before(ImGuiIO&)
{
    ImNodes::CreateContext();

    if (net::get_flow().start_existing()) {
        net::get_flow().resolve();
    };
    current = get_config().dark_theme;
}

bool loop(ImGuiIO& imio)
{
    MenuBar();
    ImGui::Begin("Root", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking);
    ImGui::SetWindowSize(imio.DisplaySize);
    ImGui::SetWindowPos(ImVec2(0, ImGui::CalcTextSize("File").y));
    ImGui::DockSpace(ImGui::GetID("Root"), ImVec2(0.f, 0.f),
        ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
    new_flow();
    {
        NRef<Scene> scene = io::scene::get();
        SceneInfo(&scene);
        if (scene != nullptr) {
            io::scene::run_frame();
        }
        NodeEditor(&scene);
        Inspector(&scene);
        Palette();
        Logger();

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

            ImGui::Checkbox("Demo Window",
                &show_demo_window); // Edit bools storing our window
                                    // open/close state

            ImGui::SliderFloat("float", &f, 0.0f,
                1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color",
                (float*)&clear_color); // Edit 3 floats representing a color

            if ((ImGui::Checkbox("DarkModeButton", &is_dark))) {
                ImGui::SameLine();
                ImGui::Text("Dark Mode: %s", is_dark ? "true" : "false");
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / imio.Framerate, imio.Framerate);
            ImGui::End();
        }
    }
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
