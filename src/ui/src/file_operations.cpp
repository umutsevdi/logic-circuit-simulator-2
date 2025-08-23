#include <imgui.h>
#include <nfd.h>
#include <string>
#include "common.h"
#include "core.h"

namespace lcs::ui {
namespace dialog {

    static nfdu8filteritem_t filters[]
        = { { "Logic Circuit Simulation File", "lcs" } };

    LCS_ERROR open_file(void)
    {
        nfdu8char_t* out = nullptr;
        nfdopendialogu8args_t args {};
        args.filterList  = filters;
        args.filterCount = 1;

        nfdresult_t result = NFD_OpenDialogU8_With(&out, &args);
        if (result == NFD_ERROR) {
            L_ERROR("%s", NFD_GetError());
            return Error::NFD;
        } else if (result == NFD_OKAY) {
            std::vector<uint8_t> data;
            size_t idx = 0;
            Error err  = tabs::open(out, idx);
            NFD_FreePathU8(out);
            return err;
        }
        return Error::OK;
    }

    LCS_ERROR save_file_as()
    {
        nfdu8char_t* out = nullptr;
        nfdsavedialogu8args_t args {};
        args.filterList    = filters;
        args.filterCount   = 1;
        nfdresult_t result = NFD_SaveDialogU8_With(&out, &args);
        if (result == NFD_ERROR) {
            L_ERROR("%s", NFD_GetError());
            return Error::NFD;
        }
        if (result == NFD_OKAY) {
            std::string path = out;
            if (path.rfind(".lcs") == std::string::npos) {
                L_DEBUG("File is not an .lcs file. Adding file extension.");
                path += ".lcs";
            }
            if (Error err = tabs::save_as(path); err) {
                return err;
            }
            NFD_FreePathU8(out);
        }
        return Error::OK;
    }
} // namespace dialog

} // namespace lcs::ui
