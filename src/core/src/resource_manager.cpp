#include "core.h"
#include "port.h"

namespace lcs::io {

struct Inode {
    Inode(bool _is_saved, std::string _path, Scene _scene)
        : is_saved { _is_saved }
        , path { _path }
        , scene { std::move(_scene) }
    {
    }
    bool is_saved;
    bool has_changes = true;
    std::string path;
    Scene scene;
};
static std::vector<Inode> SCENE_STORAGE;
static size_t active_scene = SIZE_MAX;

LCS_ERROR scene_open(const std::string& path, size_t& idx)
{
    for (size_t i = 0; i < SCENE_STORAGE.size(); i++) {
        if (SCENE_STORAGE[i].path == path) {
            idx = i;
            if (idx != active_scene) {
                active_scene                 = idx;
                SCENE_STORAGE[i].has_changes = true;
            }
            return OK;
        }
    }
    Inode inode { true, path, Scene {} };
    std::vector<uint8_t> data;
    if (!read(path, data)) {
        return ERROR(Error::NOT_FOUND);
    }
    Error err = deserialize(data, inode.scene);
    if (err) {
        return err;
    }
    SCENE_STORAGE.push_back(std::move(inode));
    idx                            = SCENE_STORAGE.size() - 1;
    active_scene                   = idx;
    SCENE_STORAGE[idx].has_changes = true;
    return OK;
}

LCS_ERROR scene_close(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = active_scene;
    }
    lcs_assert(idx < SCENE_STORAGE.size());
    SCENE_STORAGE.erase(SCENE_STORAGE.begin() + idx);
    if (active_scene > 0) {
        active_scene--;
        SCENE_STORAGE[active_scene].has_changes = true;
    }
    return OK;
}

void scene_notify(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = active_scene;
    }
    lcs_assert(idx < SCENE_STORAGE.size());
    Inode& inode      = SCENE_STORAGE[idx];
    inode.is_saved    = false;
    inode.has_changes = true;
}

bool is_saved(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = active_scene;
    }
    lcs_assert(idx < SCENE_STORAGE.size());
    return SCENE_STORAGE[idx].is_saved;
}

LCS_ERROR scene_save(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = active_scene;
    }
    lcs_assert(idx < SCENE_STORAGE.size());
    Inode& inode = SCENE_STORAGE[idx];
    if (inode.is_saved) {
        L_INFO("Already saved!");
        return OK;
    }
    std::vector<uint8_t> scene_bin;
    Error err = serialize(inode.scene, scene_bin);
    if (err) {
        return err;
    }
    if (!write(inode.path, scene_bin)) {
        return ERROR(Error::NO_SAVE_PATH_DEFINED);
    }
    inode.is_saved = true;
    return OK;
}

LCS_ERROR scene_save_as(const std::string& new_path, size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = active_scene;
    }
    lcs_assert(idx < SCENE_STORAGE.size());
    Inode& inode = SCENE_STORAGE[idx];
    if (inode.path == new_path) {
        return OK;
    }
    inode.path     = new_path;
    inode.is_saved = false;
    Error err      = scene_save(idx);
    if (err) {
        return err;
    }
    return OK;
}

NRef<Scene> scene_get(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = active_scene;
    }
    if (idx >= SCENE_STORAGE.size()) {
        return nullptr;
    }
    if (idx != active_scene) {
        L_INFO("idx != active_scene");
        SCENE_STORAGE[idx].has_changes = true;
    }
    active_scene = idx;
    return &SCENE_STORAGE[idx].scene;
}

size_t scene_new(const std::string& name, const std::string& author,
    const std::string& description, int version)
{
    SCENE_STORAGE.emplace_back(
        false, "", Scene { name, author, description, version });
    return SCENE_STORAGE.size() - 1;
}

void iterate(std::function<bool(std::string_view name, std::string_view path,
        bool is_saved, bool is_active)>
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
        SCENE_STORAGE[updated_scene].has_changes = true;
    }
    active_scene = updated_scene;
}

bool has_changes(void)
{
    if (active_scene >= SCENE_STORAGE.size()) {
        return false;
    }
    if (SCENE_STORAGE[active_scene].has_changes) {
        L_DEBUG("Updating tab(%d). Tab[%s]", active_scene,
            SCENE_STORAGE[active_scene].path.c_str());
        SCENE_STORAGE[active_scene].has_changes = false;
        return true;
    }
    return false;
}

static Error _check_fs(const std::string& name, Scene& s)
{
    std::string n { name };
    std::vector<std::string> tokens = split(n, '/');
    if (tokens.size() != 3) {
        return ERROR(Error::INVALID_DEPENDENCY_FORMAT);
    }
    std::string path = fs::LIBRARY / (base64_encode(name) + ".lcs");
    std::vector<uint8_t> data;
    if (!read(path, data)) {
        return ERROR(Error::COMPONENT_NOT_FOUND);
    }
    Error err = deserialize(data, s);
    if (err) {
        return err;
    }
    if (!s.component_context.has_value()) {
        return ERROR(Error::NOT_A_COMPONENT);
    }
    return OK;
}

static Error _check_src(const std::string& path, Scene& scene)
{
    std::string url = API_ENDPOINT "/" + path;
    std::vector<uint8_t> resp;
    return Error::INCOMPLETE_INSTR;
    // Error err = net::get_request(url, resp);
    // if (err) {
    //     return err;
    // }
    // err = deserialize(resp, scene);
    // if (err) {
    //     return err;
    // }
    // write(fs->LIBRARY / (base64_encode(path) + ".lcs"), resp);
    return Error::OK;
}

Error load_dependency(const std::string& name, Scene& scene)
{
    L_DEBUG("Fetching %s", name.c_str());
    Error err = _check_fs(name, scene);
    if (err == COMPONENT_NOT_FOUND) {
        err = _check_src(name, scene);
    } else if (err) {
        return err;
    }
    return OK;
}

} // namespace lcs::io
