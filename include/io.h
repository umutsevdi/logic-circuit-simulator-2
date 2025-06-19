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
#include <filesystem>

namespace Json {
class Value;
}
namespace lcs {
class Scene;
namespace io {

    extern std::filesystem::path ROOT;
    extern std::filesystem::path TMP;
    extern std::filesystem::path LIBRARY;
    extern std::filesystem::path LOCAL;
    extern std::filesystem::path CACHE;

    /** An interface that provides utilities to serialize/deserialize nodes */
    class Serializable {
        /**
         * Converts given object to a Json::Value
         * @returns Json document
         */
        virtual Json::Value to_json(void) const = 0;
        /**
         * Reads contents of the document and updates its fields.
         * @returns Error on failure:
         *
         *  - Error::INVALID_SCENE
         *  - Error::REL_CONNECT_ERROR
         *  - Error::UNDEFINED_DEPENDENCY
         *  - Error::INVALID_NODE
         *  - Error::INVALID_NODE
         *  - Error::INVALID_NODE
         *  - Error::INVALID_COMPONENT
         *  - Error::INVALID_INPUT
         *  - Error::INVALID_COMPONENT
         *  - Error::INVALID_GATE
         *  - Error::INVALID_JSON_FORMAT
         */
        virtual Error from_json(const Json::Value&) = 0;
    };

    /**
     * Initializes required folder structure for the application.
     * If testing flag is enabled files are saved to a temporary location.
     * @param is_testing whether testing mode is enabled or not
     */
    void init_paths(bool is_testing = false);

    /**
     * Reads contents of the given JSON file and returns it as a string.
     * @param path to read
     * @returns data
     */
    std::string read(const std::string& path);

    /**
     * Reads contents of the given binary file and writes it to the buffer.
     * @param path to read
     * @param data to save
     * @returns whether reading is successful or not
     */
    bool read(const std::string& path, std::vector<unsigned char>& data);

    /**
     * Write contents of data to the desired path.
     * @param path to save
     * @param data to save
     * @returns Whether the operation is successful or not
     */
    bool write(const std::string& path, const std::string& data);

    /**
     * Write contents of data to the desired path. Used for binary files.
     * @param path to save
     * @param data to save
     * @returns Whether the operation is successful or not
     */
    bool write(const std::string& path, std::vector<unsigned char>& data);

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
    Error load(const std::string& data, Scene& scene);

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
        Error open(const std::string& path, size_t& idx);

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
        Error save(size_t idx = SIZE_MAX);

        /**
         * Updates the contents of given scene.
         * @param new_path to save
         * @param idx index of the scene, active scene if not provided
         * @returns Error on failure:
         *
         * - Error::NO_SAVE_PATH_DEFINED
         */
        Error save_as(const std::string& new_path, size_t idx = SIZE_MAX);

        /**
         * Closes the scene with selected path, erasing from memory.
         * @param idx index of the scene, active scene if not provided
         * @returns Error::OK
         */
        Error close(size_t idx = SIZE_MAX);

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
        Error fetch(const std::string& name, bool invalidate = false);

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
        Error fetch(const std::string& name, const std::string& data,
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
