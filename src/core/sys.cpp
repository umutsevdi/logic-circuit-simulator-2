
#include "core.h"
#include <optional>

namespace lcs {
namespace sys {
    std::string Metadata::to_dependency_string() const
    {
        std::string dep_str = "";
        if (author == "sys") {
            dep_str += '@';
        } else {
            dep_str += author;
        }
        return '/' + name + '/' + std::to_string(version);
    }
    error_t get_component(const std::string& name, component_handle_t& id)
    {
        return error_t::OK;
    }

    uint64_t run_component(component_handle_t id, uint64_t input) { return 0; }

    std::optional<Metadata> get_dependency_info(component_handle_t id)
    {

        return std::nullopt;
    }

} // namespace sys
} // namespace lcs
