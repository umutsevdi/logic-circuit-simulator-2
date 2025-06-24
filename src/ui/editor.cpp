#include "IconsLucide.h"
#include "common.h"
#include "net.h"
#include "ui.h"
#include "ui/components.h"
#include "ui/configuration.h"
#include "ui/flows.h"
#include "ui/layout.h"
#include "ui/util.h"
#include <imgui.h>
#include <imnodes.h>
#include <tinyfiledialogs.h>
#include <algorithm>
#include <cstring>

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

bool loop(ImGuiIO& io)
{
    if (ImGui::Begin("Color Table")) {
        static const char* table[] = {
            // Light Themes
            "SEOUL256_LIGHT",
            "ACME",
            "GRUVBOX_LIGHT",
            "ONE_LIGHT",
            // Dark Themes
            "SEASHELLS",
            "XTERM",
            "GRUVBOX_DARK",
            "ONE_DARK",
        };
        if (ImGui::Combo("Select Theme", (int*)&current, table, (int)STYLE_S)) {
            get_config().dark_theme = current;
            get_config().is_applied = false;
        }
        const LcsStyle& style = ui::get_style(get_config().dark_theme);
        ImGui::ColorButton("bg", style.bg);
        ImGui::SameLine();
        ImGui::ColorButton("fg", style.fg);
        ImGui::ColorButton("black", style.black);
        ImGui::SameLine();
        ImGui::ColorButton("black_bright", style.black_bright);
        ImGui::ColorButton("red", style.red);
        ImGui::SameLine();
        ImGui::ColorButton("red_bright", style.red_bright);
        ImGui::ColorButton("green", style.green);
        ImGui::SameLine();
        ImGui::ColorButton("green_bright", style.green_bright);
        ImGui::ColorButton("yellow", style.yellow);
        ImGui::SameLine();
        ImGui::ColorButton("yellow_bright", style.yellow_bright);
        ImGui::ColorButton("blue", style.blue);
        ImGui::SameLine();
        ImGui::ColorButton("blue_bright", style.blue_bright);
        ImGui::ColorButton("magenta", style.magenta);
        ImGui::SameLine();
        ImGui::ColorButton("magenta_bright", style.magenta_bright);
        ImGui::ColorButton("cyan", style.cyan);
        ImGui::SameLine();
        ImGui::ColorButton("cyan_bright", style.cyan_bright);
        ImGui::ColorButton("white", style.white);
        ImGui::SameLine();
        ImGui::ColorButton("white_bright", style.white_bright);
        ImGui::End();
    }
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus;
    MenuBar();
    ImVec2 app_size = io.DisplaySize;
    ImGui::Begin("Node Editor", nullptr, flags);
    ImGui::SetWindowSize(ImVec2 { app_size.x, app_size.y - 20.0f });
    ImGui::SetWindowPos(
        ImVec2 { app_size.x * 0.0f, app_size.y * 0.0f + 20.0f });
    TabWindow();
    new_flow();
    NRef<Scene> scene = io::scene::get();
    io::scene::run_frame();
    if (scene == nullptr) {
        ImGui::End();
        return true;
    }

    float left_width = app_size.x * 0.3f;
    if (ImGui::BeginChild("LeftMenu", ImVec2 { left_width, app_size.y * 0.85f },
            ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoTitleBar)) {
        if (ImGui::BeginChild(
                "SceneInfo", ImVec2 { left_width, app_size.y * 0.85f / 2 })) {
            ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
            ImGui::TextColored(ImVec4(200, 200, 0, 255), "Scene Name");
            ImGui::PopFont();
            if (ImGui::InputText("##SceneNameInputText", scene->name.data(),
                    scene->name.max_size(), ImGuiInputTextFlags_CharsNoBlank)) {
                io::scene::notify_change();
            };
            ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
            ImGui::TextColored(ImVec4(200, 200, 0, 255), "Description");
            ImGui::PopFont();
            if (ImGui::InputTextMultiline("##SceneDescInputText",
                    scene->description.data(), scene->description.max_size(),
                    ImVec2(ImGui::GetWindowWidth(),
                        ImGui::CalcTextSize("\n\n\n").y))) {
                io::scene::notify_change();
            };
            ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
            ImGui::TextColored(ImVec4(200, 200, 0, 255), "Version");
            ImGui::PopFont();
            if (ImGui::InputInt("##SceneVersion", &scene->version)) {
                scene->version = std::max(scene->version, 0);
                io::scene::notify_change();
            };

            if (scene->component_context.has_value()) {
                ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
                ImGui::TextColored(
                    ImVec4(200, 200, 0, 255), "Component Attributes");
                ImGui::Separator();
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "Input Size");
                ImGui::PopFont();
                size_t input_size  = scene->component_context->inputs.size();
                size_t output_size = scene->component_context->outputs.size();

                ImGui::InputInt("##CompInputSize", (int*)&input_size);
                ImGui::PushFont(get_font(FontFlags::BOLD | FontFlags::NORMAL));
                ImGui::TextColored(ImVec4(200, 200, 0, 255), "Output Size");
                ImGui::PopFont();
                ImGui::InputInt("##CompOutputSize", (int*)&output_size);

                if (input_size != scene->component_context->inputs.size()
                    || output_size
                        != scene->component_context->outputs.size()) {
                    scene->component_context->setup(input_size, output_size);
                    io::scene::notify_change();
                }
            }
            if (IconButton<NORMAL>(ICON_LC_UPLOAD, "Upload")) {
                std::string resp;
                net::upload_scene(&scene, resp);
            }
            ImGui::EndChild();
        }
        if (ImGui::BeginChild("ObjectInspector",
                ImVec2 { left_width, app_size.y * 0.85f / 2 })) {
            ImGui::TextColored(ImVec4(200, 200, 0, 255), "Object Inspector");
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();
    NodeEditor(&scene);
    ImGui::SameLine();
    ImGui::BeginChild("List",
        ImVec2 { ImGui::CalcTextSize("COMPONENT").x, app_size.y * 0.85f },
        ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoTitleBar);
    {
        Node dragged_node;
        if (scene == nullptr) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Input")) {
            dragged_node = scene->add_node<InputNode>();
        }
        if (ImGui::Button("Output")) {
            dragged_node = scene->add_node<OutputNode>();
        }
        if (ImGui::Button("Timer")) {
            dragged_node = scene->add_node<InputNode>(1.0f);
        }
        if (ImGui::Button("NOT Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::NOT);
        }
        if (ImGui::Button("AND Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::AND);
        }
        if (ImGui::Button("OR Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::OR);
        }
        if (ImGui::Button("XOR Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::XOR);
        }
        if (ImGui::Button("NAND Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::NAND);
        }
        if (ImGui::Button("NOR Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::NOR);
        }
        if (ImGui::Button("XNOR Gate")) {
            dragged_node = scene->add_node<GateNode>(GateType::XNOR);
        }
        ImGui::EndChild();
    }
    Inspector(&scene);

    Palette();
    ImGui::End();

    ImGui::Begin("Font Window");
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
        1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
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
