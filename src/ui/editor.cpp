#include "ui.h"

#include <imgui.h>

bool static show_demo_window = true;
ImVec4 clear_color           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

namespace lcs::ui {

bool loop(ImGuiIO& io)
{
    static float f     = 0.0f;
    static int counter = 0;

    ImGui::Begin("Party!"); // Create a window called "Hello,
                            // world!" and append into it.

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

    if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                 // widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    if ((ImGui::Checkbox("DarkModeButton", &is_dark))) {
        ImGui::SameLine();
        ImGui::Text("Dark Mode: %s", is_dark ? "true" : "false");
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
    return show_demo_window;
}
} // namespace lcs::ui
