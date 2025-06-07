#include <base64.h>
#include <json/json.h>

#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>

#include "common.h"
#include "core.h"
#include "io.h"

namespace lcs::io {

namespace fs = std::filesystem;

struct Inode {
    Inode(bool _is_saved, std::string _path, Scene _scene)
        : is_saved { _is_saved }
        , path { _path }
        , scene { std::move(_scene) }
    {
    }
    bool is_saved;
    std::string path;
    Scene scene;
};

fs::path TMP;
fs::path APP_DATA;
fs::path LIBRARY;
fs::path LOCAL;

static std::string current_path;
static std::map<std::string, Scene> COMPONENT_STORAGE;
static std::vector<Inode> SCENE_STORAGE;
static size_t active_scene = SIZE_MAX;
static bool _has_changes   = false;

void init_paths(bool is_testing)
{
    const char* home = std::getenv("HOME");
#ifdef _WIN32
    // TODO
#elif defined(__linux__)
    TMP      = "/tmp/" APPNAME_LONG;
    APP_DATA = home ? std::string(home) + "/.local/share/" APPNAME_LONG
                    : "/tmp/" APPNAME_LONG;
#elif defined(__unix__)
    // TODO For BSD & Mac
#endif
    if (is_testing) {
        time_t now = time(0);
        tm* t      = localtime(&now);
        std::stringstream s_time { ".test_" };
        s_time << "run" << "_" << t->tm_hour << "_" << t->tm_min << "_"
               << t->tm_sec;
        s_time <<
#ifdef NDEBUG
            "_rel"
#else
            "_dbg"
#endif
            ;
        APP_DATA = TMP / s_time.str();
        TMP      = TMP / s_time.str() / "tmp";
        L_INFO("Creating testing environment at:" << APP_DATA);
    }
    LIBRARY = APP_DATA / "lib";
    LOCAL   = APP_DATA / "local";
    try {
        if (!fs::exists(TMP)) {
            L_INFO("Creating" << TMP);

            fs::create_directories(TMP);
        }
        if (!fs::exists(LIBRARY)) {
            L_INFO("Creating" << LIBRARY);
            fs::create_directories(LIBRARY);
        }
        if (!fs::exists(LOCAL)) {
            L_INFO("Creating" << LOCAL);

            fs::create_directories(LOCAL);
        }
        if (is_testing) {
            TESTLOG.emplace(APP_DATA / "log.txt");
        }
    } catch (const std::exception& e) {
        L_ERROR("Directory creation failed: " << e.what());
    }
}

bool write(const std::string& path, const std::string& data)
{
    L_INFO("Saving to " << path);
    try {
        fs::create_directories(std::string {
            path.begin(), path.begin() + path.find_last_of("/") });
        std::ofstream outfile { path };
        if (outfile) {
            outfile << data;
            return true;
        }
        L_ERROR("Failed to open file for writing: " << path);
    } catch (const std::exception& e) {
        L_ERROR("Exception in write_component: " << e.what());
    }
    return false;
}

std::string read(const std::string& path)
{
    L_INFO("Reading from " << path);
    if (path.find(".json") == std::string::npos) {
        return "";
    }
    fs::path json_path = fs::path(path);
    std::ifstream infile { json_path };
    std::string content((std::istreambuf_iterator<char>(infile)),
        std::istreambuf_iterator<char>());
    return content;
}

Error load(const std::string& data, Scene& s)
{
    if (data == "") {
        return ERROR(Error::NOT_FOUND);
    }
    Json::Reader reader {};
    Json::Value root;
    if (!reader.parse(data, root)) {
        return ERROR(Error::INVALID_JSON_FORMAT);
    }
    return s.from_json(root);
}

namespace scene {

    Error open(const std::string& path, size_t& idx)
    {
        for (size_t i = 0; i < SCENE_STORAGE.size(); i++) {
            if (SCENE_STORAGE[i].path == path) {
                idx = i;
                if (idx != active_scene) {
                    active_scene = idx;
                    _has_changes = true;
                }
                return Error::OK;
            }
        }
        Inode inode { true, path, Scene {} };
        std::string data = read(path);
        Error err        = io::load(data, inode.scene);
        if (err) {
            return err;
        }
        SCENE_STORAGE.push_back(std::move(inode));
        idx          = SCENE_STORAGE.size() - 1;
        active_scene = idx;
        _has_changes = true;
        return Error::OK;
    }

    Error close(size_t idx)
    {
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        lcs_assert(idx < SCENE_STORAGE.size());
        SCENE_STORAGE.erase(SCENE_STORAGE.begin() + idx);
        if (active_scene > 0) {
            active_scene--;
            _has_changes = true;
        }
        return Error::OK;
    }

    void notify_change(size_t idx)
    {
        L_INFO("Change happened");
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        lcs_assert(idx < SCENE_STORAGE.size());
        Inode& inode   = SCENE_STORAGE[idx];
        inode.is_saved = false;
        _has_changes   = true;
    }

    bool is_saved(size_t idx)
    {
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        lcs_assert(idx < SCENE_STORAGE.size());
        return SCENE_STORAGE[idx].is_saved;
    }

    Error save(size_t idx)
    {
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        lcs_assert(idx < SCENE_STORAGE.size());
        Inode& inode = SCENE_STORAGE[idx];
        if (inode.is_saved) {
            L_INFO("Already saved!");
            return Error::OK;
        }

        if (inode.scene.component_context.has_value()) {
            inode.path = inode.scene.to_filepath();
        }

        if (!write(inode.path, inode.scene.to_json().toStyledString())) {
            return ERROR(Error::NO_SAVE_PATH_DEFINED);
        }
        inode.is_saved = true;
        // Reload component storage if saved scene is a component
        if (inode.scene.component_context.has_value()) {
            std::string dependency_string = inode.scene.to_dependency();
            if (auto compiter = COMPONENT_STORAGE.find(dependency_string);
                compiter != COMPONENT_STORAGE.end()) {
                COMPONENT_STORAGE.insert_or_assign(dependency_string, Scene {});
                COMPONENT_STORAGE[dependency_string].from_json(
                    inode.scene.to_json());
            }
        }
        return Error::OK;
    }

    Error save_as(const std::string& new_path, size_t idx)
    {
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        lcs_assert(idx < SCENE_STORAGE.size());
        Inode& inode = SCENE_STORAGE[idx];
        if (inode.path == new_path) {
            return Error::OK;
        }
        inode.path     = new_path;
        inode.is_saved = false;
        Error err      = save(idx);
        if (err) {
            return err;
        }
        return Error::OK;
    }

    NRef<Scene> get(size_t idx)
    {
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        if (idx >= SCENE_STORAGE.size()) {
            return nullptr;
        }
        if (idx != active_scene) {
            L_INFO("idx != active_scene");
            _has_changes = true;
        }
        active_scene = idx;
        return &SCENE_STORAGE[idx].scene;
    }

    size_t create(const std::string& name, const std::string& author,
        const std::string& description, int version)
    {
        SCENE_STORAGE.emplace_back(
            false, "", Scene { name, author, description, version });
        _has_changes = true;
        return SCENE_STORAGE.size() - 1;
    }

    void iterate(std::function<bool(std::string_view name,
            std::string_view path, bool is_saved, bool is_active)>
            run)
    {
        size_t updated_scene = active_scene;
        for (size_t i = 0; i < SCENE_STORAGE.size(); i++) {
            if (run(std::string_view { SCENE_STORAGE[i].scene.name.begin() },
                    std::string_view { SCENE_STORAGE[i].path },
                    SCENE_STORAGE[i].is_saved, i == active_scene)) {
                updated_scene = i;
            };
        }
        if (active_scene != updated_scene) {
            _has_changes = true;
        }
        active_scene = updated_scene;
    }

    void run_frame(size_t idx)
    {
        if (idx == SIZE_MAX) {
            idx = active_scene;
        }
        if (idx >= SCENE_STORAGE.size()) {
            return;
        }
        SCENE_STORAGE[idx].scene.run_timers();
        for (auto compname : SCENE_STORAGE[idx].scene.dependencies) {
            COMPONENT_STORAGE.find(compname)->second.run_timers();
            // FIX Component may not be fetched correctly
        }
    }

    bool has_changes(void)
    {
        if (_has_changes) {
            L_INFO("Updating tab(" << active_scene << ") at "
                                   << SCENE_STORAGE[active_scene].path);
            _has_changes = false;
            return true;
        }
        return false;
    }

} // namespace scene

namespace component {

    void _load_stdlib(void);

    Error _check_fs(const std::string& name)
    {
        std::string n { name };
        std::vector<std::string> tokens = split(n, '/');
        if (tokens.size() != 3) {
            return ERROR(Error::INVALID_DEPENDENCY_FORMAT);
        }
        std::string path;
        Scene s;
        if (tokens[0] == "local") {
            path = LOCAL
                / (base64_encode(tokens[1] + "/" + tokens[2]) + ".json");
        } else {
            path = LIBRARY / tokens[0]
                / (base64_encode(tokens[1] + "/" + tokens[2]) + ".json");
        }
        std::string data = read(path);
        if (data == "") {
            return ERROR(Error::COMPONENT_NOT_FOUND);
        }
        Error err = load(data, s);
        if (err) {
            return err;
        }
        if (!s.component_context.has_value()) {
            return ERROR(Error::NOT_A_COMPONENT);
        }
        COMPONENT_STORAGE.insert_or_assign(s.to_dependency(), std::move(s));
        return Error::OK;
    }

    Error _check_src(const std::string&) { return Error::NOT_FOUND; }

    Error fetch(const std::string& name, bool invalidate)
    {
        if (!invalidate) {
            if (auto cmp = COMPONENT_STORAGE.find(name);
                cmp != COMPONENT_STORAGE.end()) {
                return Error::OK;
            }
        }
        Error err = _check_fs(name);
        if (err == Error::COMPONENT_NOT_FOUND) {
            err = _check_src(name);
        }
        if (err) {
            return err;
        }
        return Error::OK;
    }

    Error fetch(
        const std::string& name, const std::string& data, bool invalidate)
    {
        if (!invalidate) {
            if (auto cmp = COMPONENT_STORAGE.find(name);
                cmp != COMPONENT_STORAGE.end()) {
                return Error::OK;
            }
        }
        Scene s;
        if (Error err = load(data, s); err) {
            return err;
        }
        if (!s.component_context.has_value()) {
            return ERROR(Error::NOT_A_COMPONENT);
        }
        COMPONENT_STORAGE.insert_or_assign(name, std::move(s));
        return Error::OK;
    }

    uint64_t run(const std::string& name, uint64_t input)
    {
        if (fetch(name)) {
            return 0;
        }
        return COMPONENT_STORAGE[name].component_context->run(input);
    }

    NRef<const Scene> get(const std::string& name)
    {
        if (auto cmp = COMPONENT_STORAGE.find(name);
            cmp != COMPONENT_STORAGE.end()) {
            return &cmp->second;
        }
        return nullptr;
    }
} // namespace component
} // namespace lcs::io
