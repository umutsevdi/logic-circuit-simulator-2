
// Dear ImGui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline (GLFW is a cross-platform general purpose library
// for handling windows, inputs, OpenGL/Vulkan/Metal graphics context
// creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local
// docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "io.h"
#include "ui.h"
#include "ui/configuration.h"
#include "ui/util.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900)                                    \
    && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See
// 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

namespace lcs {
namespace ui {
    bool is_dark = false;
    int main(int, char**)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            return 1;
        }

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100 (WebGL 1.0)
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
        // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
        const char* glsl_version = "#version 300 es";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(
            GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // // 3.2+ only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        // // 3.0+ only
#endif

        // Create window with graphics context
        load_config();
        Configuration& cfg = get_config();
        GLFWwindow* window = glfwCreateWindow(cfg.startup_win_x,
            cfg.startup_win_y, "Logic Circuit Simulator", nullptr, nullptr);
        if (window == nullptr) {
            return 1;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& imio = ImGui::GetIO();
        (void)imio;
        imio.IniFilename = INI.c_str();
        imio.ConfigFlags
            |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        imio.ConfigFlags
            |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        imio.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ui::before(imio);
        set_style(imio, true);
        // Setup Dear ImGui style
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
        ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You
        // can also load multiple fonts and use ImGui::PushFont()/PopFont() to
        // select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if
        // you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return a nullptr.
        // Please handle those errors in your application (e.g. use an
        // assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and
        // stored into a texture when calling
        // ImFontAtlas::Build()/GetTexDataAsXXXX(), which
        // ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
        // Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a
        // string literal you need to write a double backslash \\ !
        // - Our Emscripten build process allows embedding fonts to be
        // accessible at runtime from the "fonts/" folder. See
        // Makefile.emscripten for details.
        // io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // ImFont* font =
        // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
        // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font !=
        // nullptr);

        // Our state
        bool show_demo_window    = true;
        bool show_another_window = false;
        ImVec4 clear_color       = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // Main loop
#ifdef __EMSCRIPTEN__
        // For an Emscripten build we are disabling file-system access, so let's
        // not attempt to do a fopen() of the imgui.ini file. You may manually
        // call LoadIniSettingsFromMemory() to load settings from your own
        // storage.
        io.IniFilename = nullptr;
        EMSCRIPTEN_MAINLOOP_BEGIN
#else
        while (!glfwWindowShouldClose(window))
#endif
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard
            // flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input
            // data to your main application, or clear/overwrite your copy of
            // the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard
            // input data to your main application, or clear/overwrite your copy
            // of the keyboard data. Generally you may always pass all inputs to
            // dear imgui, and hide them from your application based on those
            // two flags.
            glfwPollEvents();
            if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
                ImGui_ImplGlfw_Sleep(10);
                continue;
            }

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();

            ImGui::NewFrame();
            ImGui::ShowDemoWindow(&show_demo_window);
            if (show_demo_window) {
                show_demo_window = ui::loop(imio);
            }

            // 3. Show another simple window.
            if (show_another_window) {
                ImGui::Begin("Another Window",
                    &show_another_window); // Pass a pointer to our bool
                                           // variable (the window will have a
                                           // closing button that will clear the
                                           // bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me")) {
                    show_another_window = false;
                }
                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w,
                clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (!get_config().is_applied) {
                L_INFO("Configuration changes were found!");
                set_style(imio);
            }
            glfwSwapBuffers(window);
        }
#ifdef __EMSCRIPTEN__
        EMSCRIPTEN_MAINLOOP_END;
#endif
        ui::after(imio);

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }
} // namespace ui
} // namespace lcs
