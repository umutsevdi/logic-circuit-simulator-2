#include "common.h"
#include "core.h"
#include "parse.h"
#include <optional>

namespace lcs {
namespace sys {
    static std::map<std::string, Scene> _loaded_components;

    static error_t _parse_component(const std::string& name)
    {
        if (name[0] == '@') {
            // standard library
        } else {
            std::string n { name };
            std::vector<std::string> tokens = split(n, '/');
            if (tokens.size() != 3) {
                for (auto t : tokens) {
                    L_INFO("TOK: " << t);
                }
                return ERROR(error_t::INVALID_DEPENDENCY_FORMAT);
            }
            if (is_available(tokens[0], tokens[1], tokens[2])) {
                std::string data
                    = read_component(tokens[0], tokens[1], tokens[2]);
                error_t err = load_component(name, data);
                if (err) { return err; }
            }
        }
        return error_t::OK;
    }

    error_t verify_component(const std::string& name)
    {
        if (auto cmp = _loaded_components.find(name);
            cmp != _loaded_components.end()) {
            return error_t::OK;
        }
        error_t err = _parse_component(name);
        if (err) {
            // TODO try loading from network later
            return err;
        }

        if (auto cmp = _loaded_components.find(name);
            cmp == _loaded_components.end()) {
            return ERROR(error_t::COMPONENT_NOT_FOUND);
        }
        return error_t::OK;
    }

    error_t load_component(const std::string name, std::string data)
    {
        Scene s;
        error_t err = parse::from_json(data, s);
        if (err) { return err; }
        if (!s.component_context.has_value()) {
            return ERROR(error_t::NOT_A_COMPONENT);
        }
        _loaded_components[name] = s;
        return error_t::OK;
    }

    uint64_t run_component(const std::string& name, uint64_t input)
    {
        if (verify_component(name)) { return 0; }

        if (auto cmp = _loaded_components.find(name);
            cmp != _loaded_components.end()) {
            L_INFO(_loaded_components[name]);
            return _loaded_components[name].component_context->run(
                &_loaded_components.find(name)->second, input);
        }
        return ERROR(error_t::COMPONENT_NOT_FOUND);
    }

    NRef<const Scene> get_dependency(const std::string& name)
    {
        if (auto cmp = _loaded_components.find(name);
            cmp != _loaded_components.end()) {
            return &cmp->second;
        }
        return nullptr;
    }

    std::string Metadata::to_dependency_string() const
    {
        std::string dep_str = "";
        if (author == "sys") {
            dep_str = '@';
        } else if (author == "") {
            dep_str = "local/";
        } else {
            dep_str = author + '/';
        }
        return dep_str + name + '/' + std::to_string(version);
    }

    error_t Metadata::load_dependencies(void)
    {
        return load_dependencies(to_dependency_string());
    }
    error_t Metadata::load_dependencies(const std::string& self_dependency)
    {
        for (const std::string& dep : dependencies) {
            error_t err = verify_component(dep);
            if (err) { return err; }
            err = _loaded_components[dep].meta.load_dependencies(
                self_dependency);
            if (err) { return err; }
        }
        return error_t::OK;
    }
} // namespace sys
} // namespace lcs
