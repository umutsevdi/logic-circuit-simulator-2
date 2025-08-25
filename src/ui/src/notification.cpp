#include <IconsLucide.h>
#include "components.h"
#include "ui.h"

namespace lcs::ui {
static size_t last_time            = 0;
static constexpr int MAX_DURATION  = 5 * 1000;
static constexpr int ANIM_DURATION = 0.2 * 1000;

struct Notification {
    Notification(const char* _icon, const char* _title, const char* _message,
        bool _is_error)
        : icon { _icon }
        , duration { MAX_DURATION }
        , is_error { _is_error }
    {
        std::strncpy(title.data(), _title, title.max_size() - 1);
        std::strncpy(message.data(), _message, message.max_size() - 1);
    }
    const char* icon = ICON_LC_INFO;
    std::array<char, 128> title {};
    std::array<char, 512> message {};
    int duration      = MAX_DURATION;
    int anim_duration = 0;
    bool is_created   = false;
    bool is_error     = false;
};

static std::vector<Notification> notifications;
static bool _show_toast(
    Notification& n, uint64_t tick, const ImVec2 pos, uint64_t& height);

void Toast(
    const char* icon, const char* title, const char* message, bool is_error)
{
    notifications.emplace_back(icon, title, message, is_error);
    L_DEBUG("Push notification %s", title, message);
}

void RenderNotifications(void)
{
    size_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                     .count();
    size_t tick = now - last_time;
    std::vector<size_t> delete_list {};
    delete_list.reserve(notifications.size());

    const auto vp_size = ImGui::GetMainViewport()->Size;
    size_t height      = 0;
    for (auto i = notifications.begin(); i != notifications.end();) {
        if (!_show_toast(*i, tick,
                ImVec2(vp_size.x - ImGui::GetStyle().ItemSpacing.x,
                    vp_size.y - ImGui::GetStyle().ItemSpacing.y - height),
                height)) {
            i = notifications.erase(i);
        } else {
            i++;
        }
    }
    last_time = now;
}

static bool _show_toast(
    Notification& n, uint64_t tick, ImVec2 pos, uint64_t& height)
{
    if (n.is_created) {
        if (n.duration >= 0) {
            n.duration -= tick;
        } else {
            n.duration = 0;
            if (n.anim_duration > 0) {
                n.anim_duration -= tick;
            } else {
                n.anim_duration = 0;
            }
        }
    } else {
        n.anim_duration += tick;
        if (n.anim_duration >= ANIM_DURATION) {
            n.anim_duration = ANIM_DURATION;
            n.is_created    = true;
        }
    }

    if (!n.anim_duration && !n.duration) {
        return false;
    }
    float alpha = n.anim_duration / ((float)ANIM_DURATION);
    pos.x       = pos.x + 200 * (1 - alpha);
    if (n.is_error) {
        ImGui::PushStyleColor(
            ImGuiCol_Border, v4mul(get_active_style().red, 1.f, alpha));
    } else {
        ImGui::PushStyleColor(
            ImGuiCol_Border, v4mul(get_active_style().green, 1.f, alpha));
    }
    ImGui::SetNextWindowBgAlpha(alpha);
    ImGui::PushStyleColor(ImGuiCol_Text,
        v4mul(ImGui::GetStyleColorVec4(ImGuiCol_Text), 1.f, alpha));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
        v4mul(ImGui::GetStyleColorVec4(ImGuiCol_Border), 1.f, alpha));
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0),
        ImVec2(ImGui::GetMainViewport()->Size.x * 0.2,
            ImGui::GetMainViewport()->Size.y * 0.2));
    ImGui::Begin(std::to_string(pos.y).c_str(), nullptr,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoSavedSettings);

    ImGui::PushStyleColor(
        ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Border));
    IconText<LARGE>(n.icon, "%s", n.title.data());
    ImGui::PopStyleColor();
    ImGui::ProgressBar(n.duration / (float)MAX_DURATION,
        ImVec2(ImGui::GetWindowWidth(), ImGui::GetStyle().ItemSpacing.y), "##");
    height += ImGui::GetWindowHeight() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::TextUnformatted(n.message.data());

    if (IconButton<NORMAL>(ICON_LC_EYE_OFF, _("Dismiss"))) {
        n.duration = 0;
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    return true;
}

} // namespace lcs::ui
