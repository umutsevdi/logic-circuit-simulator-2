#pragma once
/*******************************************************************************
 * \file
 * File: io.h
 * Created: 04/11/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include "common.h"
#include <cstdint>

namespace Json {
class Value;
}
namespace lcs {
class Scene;
namespace io {
    /**
     * Parse a component from given string data, if it's a valid component
     * insert it to _loaded_components
     *
     * @param data to parse
     * @param scene to update
     * @returns Error on failure:
     *
     * - Error::NOT_FOUND
     * - Error::INVALID_JSON_FORMAT
     * - Serializable::from_json
     *
     */
    LCS_ERROR load(const std::string& data, Scene& scene);

    /***************************************************************************
                                        Scene
    ***************************************************************************/

    namespace scene {

        /**
         * Reads scene data from given string and updates the given scene.
         * @param path to file
         * @param idx on successful open_scene calls idx will be updated to a
         * valid idx to access scene.
         * @returns Error on failure:
         *
         * - io::load
         */
        LCS_ERROR open(const std::string& path, size_t& idx);

        /**
         * Alerts the component cache about changes in a scene.
         * @param idx to update
         */
        void notify_change(size_t idx = SIZE_MAX);

        /**
         * Get if there are any changes that have been done to the scene
         * outside the Node editor. Node editor uses this method to update
         * itself.
         *
         * NOTE: Has changes assumes, after calling this method required changes
         * are made. So calling this method again would return always false.
         *
         * @returns whether there are changes within the scene
         */
        bool has_changes(void);

        bool is_saved(size_t idx = SIZE_MAX);

        /**
         * Updates the contents of given scene.
         * @param idx index of the scene, active scene if not provided
         * @returns Error on failure:
         *
         * - Error::NO_SAVE_PATH_DEFINED
         */
        LCS_ERROR save(size_t idx = SIZE_MAX);

        /**
         * Updates the contents of given scene.
         * @param new_path to save
         * @param idx index of the scene, active scene if not provided
         * @returns Error on failure:
         *
         * - Error::NO_SAVE_PATH_DEFINED
         */
        LCS_ERROR save_as(const std::string& new_path, size_t idx = SIZE_MAX);

        /**
         * Closes the scene with selected path, erasing from memory.
         * @param idx index of the scene, active scene if not provided
         * @returns Error::OK
         */
        LCS_ERROR close(size_t idx = SIZE_MAX);

        /**
         * Selects a scene as current.
         * @param idx to select
         * @returns reference to scene
         */
        NRef<Scene> get(size_t idx = SIZE_MAX);

        /**
         * Runs a single frame to update values of selected scene and it's
         * dependency's clocks.
         * @param idx to select
         */
        void run_frame(size_t idx = SIZE_MAX);

        /**
         * Creates an empty scene with given name
         * @param name Scene name
         * @param author Scene author
         * @param description Scene description
         * @param version Scene version
         * @returns scene index
         */
        size_t create(const std::string& name = "",
            const std::string& author = "", const std::string& description = "",
            int version = 1);

        /**
         * Iterate over all opened scenes and perform an action
         * @param run method to execute
         *  > name - name of the scene
         *  > path - path to the scene, empty string if unsaved
         *  > is_saved - whether the scene is saved or not
         *  > is_active - whether current scene is active
         *  > returns true current scene should be active
         */
        void iterate(std::function<bool(std::string_view name,
                std::string_view path, bool is_saved, bool is_active)>
                run);

    } // namespace scene

    /***************************************************************************
                                        Component
    ***************************************************************************/

    namespace component {
        /**
         * Verifies the existence of a component. If it hasn't been loaded yet,
         * loads the component from the file system. If the component does not
         * exist in file system attempts to pull it from an available mirror.
         * @param name component name
         * @param invalidate whether it should invalidate cache or not
         * @returns Error on failure:
         *
         * - Error::NOT_A_COMPONENT
         * - io::load
         */
        LCS_ERROR fetch(const std::string& name, bool invalidate = false);

        /**
         * Fetches a component from given JSON document.
         * @param name component name
         * @param data data to save
         * @param invalidate whether it should invalidate cache or not
         * @returns Error on failure:
         *
         * - Error::NOT_A_COMPONENT
         * - io::load
         */
        LCS_ERROR fetch(const std::string& name, const std::string& data,
            bool invalidate = false);

        /**
         * Executes the component with given id with provided input, returning
         * it's result.
         *
         * @param name component name
         * @param input binary encoded input value
         * @returns - binary encoded result
         */
        uint64_t run(const std::string& name, uint64_t input);

        /**
         * Returns a reference to a dependency. Dependency has to be loaded to
         * use this method.
         * @param name of the component
         * @returns Constant reference to a component or nullptr
         */
        NRef<const Scene> get(const std::string& name);
    } // namespace component
} // namespace io
} // namespace lcs
