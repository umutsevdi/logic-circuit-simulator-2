#include "IconsLucide.h"
#include "components.h"
#include "ui.h"
#include <imgui.h>
#include <imnodes.h>

namespace lcs::ui::layout {

static void tab_window(void);
static bool _show_new;
static bool _show_close;
static void new_flow(void);
static void close_flow(void);

bool pref_show = false;
bool df_show   = false;
void MenuBar(void)
{
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));

    ImGui::PushFont(get_font(FontFlags::NORMAL));
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (IconButton<NORMAL>(ICON_LC_PLUS, "New")) {
                _show_new = true;
            }
            if (IconButton<NORMAL>(ICON_LC_FOLDER_OPEN, "Open")) {
                dialog::open_file();
            }
            ImGui::BeginDisabled(tabs::active() == nullptr);
            if (IconButton<NORMAL>(ICON_LC_SAVE, "Save")) {
                if (tabs::save() == Error::NO_SAVE_PATH_DEFINED) {
                    dialog::save_file_as();
                };
            }

            if (IconButton<NORMAL>(ICON_LC_SAVE_ALL, "Save As")) {
                dialog::save_file_as();
            }
            ImGui::EndDisabled();
            if (IconButton<NORMAL>(ICON_LC_SETTINGS_2, "Preferences")) {
                pref_show = true;
            }
            if (IconButton<NORMAL>(ICON_LC_X, "Close")) {
                close_flow();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::Checkbox("Palette", &user_data.palette);
            ImGui::Checkbox("Inspector", &user_data.inspector);
            ImGui::Checkbox("Console", &user_data.console);
            ImGui::Checkbox("Scene Info", &user_data.scene_info);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
            - ImGui::GetStyle().ItemInnerSpacing.y);

        tab_window();
        //        ImVec2 pos        = ImGui::GetWindowContentRegionMax();
        //      ImVec2 login_text = ImGui::CalcTextSize("Login");

        //      ImGui::PushStyleColor(
        //          ImGuiCol_Button,
        //          ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        //      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        //          ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
        //      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        //          ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        //      if (auth::is_logged_in()) {
        //          const ImageHandle* profile
        //              = get_texture(net::get_account().avatar_url);
        //          if (profile) {
        //              ImGui::PushStyleVar(
        //                  ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.1f, 0.1f));
        //              float h_image = (1.2 * login_text.y
        //                  + 2 * ImGui::GetStyle().ItemInnerSpacing.y);
        //              ImGui::SameLine(pos.x - ImGui::GetStyle().ItemSpacing.x
        //                  - profile->w / profile->h * h_image);
        //              if (ImGui::ImageButton(net::get_account().login.c_str(),
        //                      profile->gl_id,
        //                      ImVec2(profile->w / profile->h * h_image,
        //                      h_image), ImVec2(0, 0), ImVec2(1, 1))) {
        //                  ImGui::OpenPopup("Profile Settings");
        //              }
        //              if (ImGui::BeginPopup(
        //                      "Profile Settings", ImGuiWindowFlags_ChildMenu))
        //                      {
        //                  if (IconButton<NORMAL>(ICON_LC_SETTINGS_2,
        //                  "Settings")) { } if
        //                  (IconButton<NORMAL>(ICON_LC_LOG_OUT, "Log out")) {
        //                      net::get_flow().resolve();
        //                  }
        //                  ImGui::EndPopup();
        //              }
        //              ImGui::PopStyleVar();
        //          }
        //      } else {
        //          ImGui::SameLine(pos.x - 2 * login_text.x);
        //          ImGui::BeginDisabled(net::get_flow().get_state()
        //              == net::AuthenticationFlow::State::POLLING);
        //          ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
        //              + ImGui::GetStyle().ItemInnerSpacing.y);
        //          if (IconButton<NORMAL>(ICON_LC_LOG_IN, "Login")) {
        //              L_INFO("Login button pressed");
        //              df_show = true;
        //          }
        //          ImGui::EndDisabled();
        //      }
        // ImGui::PopStyleColor();
        // ImGui::PopStyleColor();
        // ImGui::PopStyleColor();
        ImGui::EndMainMenuBar();
        //        popup::LoginWindow(df_show);
        //  ImGui::Separator();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    popup::Preferences(pref_show);
    new_flow();
    close_flow();
}

void tab_window(void)
{
    ImGui::BeginTabBar("Scene Tabs", ImGuiTabBarFlags_FittingPolicyScroll);
    tabs::for_each([](std::string_view name, std::string_view path,
                       bool is_saved, bool is_active) -> bool {
        bool result = false;
        bool keep   = true;

        std::string_view scene_name;
        if (!name.empty()) {
            scene_name = name;
        } else if (!path.empty()) {
            scene_name = path;
        } else {
            scene_name = "Untitled Scene";
        }
        if (ImGui::BeginTabItem(scene_name.begin(), &keep,
                (is_saved ? ImGuiTabItemFlags_None
                          : ImGuiTabItemFlags_UnsavedDocument))) {
            if (!is_active) {
                result = true;
                ImNodes::ClearLinkSelection();
                ImNodes::ClearNodeSelection();
            }
            ImGui::EndTabItem();
        }
        if (!keep) {
            close_flow();
        }
        return result;
    });
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImNodesCol_TitleBar));
    ImGui::PushFont(get_font(ICON | SMALL));
    if (ImGui::TabItemButton(ICON_LC_PLUS)) {
        _show_new = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
}

static void new_flow(void)
{
    if (!_show_new) {
        return;
    }
    ImGui::OpenPopup("New Scene");
    if (ImGui::BeginPopupModal("New Scene", &_show_new,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        static bool is_scene         = true;
        static char author[60]       = "local";
        static char name[128]        = { 0 };
        static char description[512] = { 0 };
        static size_t input_size     = 0;
        static size_t output_size    = 0;

        if (ImGui::BeginTable("##NewFlow", 2, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn(
                "##NewFlowKey", ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                "##Value", ImGuiTableColumnFlags_WidthStretch);

            TablePair(Field("Scene Name"),
                ImGui::InputText("##SceneCreate_Name", name, 128,
                    ImGuiInputTextFlags_CharsNoBlank));

            TablePair(Field("Author"),
                ImGui::PushFont(
                    get_font(FontFlags::ITALIC | FontFlags::NORMAL)),
                ImGui::InputText("##SceneCreate_Author", author, 60,
                    ImGuiInputTextFlags_ReadOnly),
                ImGui::PopFont());

            TablePair(Field("Description"),
                ImGui::InputTextMultiline(
                    "##SceneCreate_Desc", description, 512));

            TablePair(Field("Type"));
            if (ImGui::RadioButton("##IsScene", is_scene)) {
                is_scene = true;
            }
            ImGui::SameLine();
            IconText<NORMAL>(ICON_LC_LAND_PLOT, "Scene");
            ImGui::SameLine();
            if (ImGui::RadioButton("##IsComponent", !is_scene)) {
                is_scene = false;
            }
            ImGui::SameLine();
            IconText<NORMAL>(ICON_LC_PACKAGE, "Component");

            if (!is_scene) {
                ImGui::Separator();
                TablePair(Field("Input Size"));
                ImGui::InputInt("##CompInputSize", (int*)&input_size);
                TablePair(Field("Output Size"));
                ImGui::InputInt("##CompOutputSize", (int*)&output_size);
            }
            TablePair(
                if (ImGui::Button("Create")) {
                    NRef<Scene> scene = tabs::active(
                        tabs::create(name, author, description, 1));
                    if (!is_scene) {
                        scene->component_context.emplace(
                            &scene, input_size, output_size);
                    }
                    _show_new = false;
                },
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    _show_new = false;
                });
            ImGui::EndTable();
        };
        ImGui::EndPopup();
    }
}

void close_flow(void)
{
    if (!_show_close) {
        return;
    }
    if (tabs::active() == nullptr) {
        exit(0);
    }
    if (tabs::is_saved()) {
        if (!tabs::close()) {
            _show_close = false;
        }
    } else {
        ImGui::OpenPopup("Close Scene");
        if (ImGui::BeginPopupModal("Close Scene", &_show_close,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text(
                "You have unsaved changes. Would you like to save your changes "
                "before closing?");
            if (IconButton<NORMAL>(ICON_LC_SAVE, "Save & Close")
                && dialog::save_file_as() == Error::OK) {
                if (!tabs::close()) {
                    _show_close = false;
                }
            }
            ImGui::SameLine();
            if (IconButton<NORMAL>(ICON_LC_SAVE_OFF, "Exit Without Saving")) {
                if (!tabs::close()) {
                    _show_close = false;
                }
            }
        }
    }
}

} // namespace lcs::ui::layout
