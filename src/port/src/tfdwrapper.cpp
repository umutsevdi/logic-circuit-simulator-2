#include "common.h"
#include "port.h"
#include "tinyfiledialogs.h"

namespace lcs {

static const char* _PATH_FILTER[1] = { "*.json" };
static std::string args;

static void _open_scene_dialog(void)
{
    const char* path = tinyfd_openFileDialog("Select a scene", LIBRARY.c_str(),
        1, _PATH_FILTER, "LCS Scene File", 0);
    if (path != nullptr) {
        size_t idx;
        Error err = scene::open(path, idx);
        if (err) {
            ERROR(err);
        }
    }
}

static bool _save_scene_dialog(void)
{
    const char* new_path = tinyfd_saveFileDialog(
        args.c_str(), LOCAL.c_str(), 1, _PATH_FILTER, "Save the scene as");
    if (new_path != nullptr) {
        std::string path_as_str { new_path };
        if (path_as_str.find(".json") != std::string::npos) {
            return io::scene::save_as(new_path) == Error::OK;
        } else {
            return io::scene::save_as(path_as_str + ".json") == Error::OK;
        }
    }
    return false;
}

static void _close_dialog(void)
{
    if (io::scene::get() == nullptr) {
        exit(0);
    }
    if (io::scene::is_saved()) {
        io::scene::close();
    } else {
        int option = tinyfd_messageBox("Close Scene",
            "You have unsaved changes. Would you like to save your changes "
            "before closing?",
            "yesno", "question", 0);
        if (!option || _save_scene_dialog("Save scene")) {
            io::scene::close();
        }
    }
}

static void _alert(void)
{
    if (args != "") {
        tinyfd_messageBox(APPNAME_LONG, args.c_str(), "ok", "error", 1);
    }
}

enum DialogRequest { UNSET, OPEN_FILE, SAVE_FILE, ALERT, CLOSE };
static DialogRequest raised = DialogRequest::UNSET;

void request_open_file(void) { raised = DialogRequest::OPEN_FILE; }

void request_save_file(const char* path)
{
    args   = path;
    raised = DialogRequest::SAVE_FILE;
}

void request_alert(const char* msg)
{
    args   = msg;
    raised = DialogRequest::ALERT;
}

void request_close(void) { raised = DialogRequest::CLOSE; }

void handle_requests(void)
{
    switch (raised) {
    case DialogRequest::UNSET: return;
    case DialogRequest::OPEN_FILE: _open_scene_dialog(); break;
    case DialogRequest::SAVE_FILE: _save_scene_dialog(); break;
    case DialogRequest::ALERT: _alert(); break;
    case DialogRequest::CLOSE: _close_dialog(); break;
    }
    raised = DialogRequest::UNSET;
}
} // namespace lcs
