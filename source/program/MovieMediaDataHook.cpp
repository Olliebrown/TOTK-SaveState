#include "MovieMediaDataHook.hpp"
#include "logger.hpp"

int32_t gMovieFramerate = 30;

// FindInt32
bool MovieMediaDataHook::Callback(void* thisx, const char* name, int32_t* value) {
    int framerate = 30;

    if (thisx != nullptr) {
        if (Orig(thisx, "frame-rate", &framerate)) {
            gMovieFramerate = framerate;
            OutputDebug("sd:/DFPS++_DEBUG.txt", "Updated Movie Framerate: ", false);
            OutputDebug("sd:/DFPS++_DEBUG.txt", std::to_string(gMovieFramerate).c_str(), true);
        }

        return Orig(thisx, name, value);
    }

    return false;
}

const PatchInfo MovieMediaDataHook::CreatePatchInfo(void) {
    return PatchInfo(PatchGameVersion::VERSION_ANY, PatchType::DETOUR, "_ZNK5movie9MediaData9FindInt32EPKcPi", InstallAtOffset, InstallAtPtr, InstallAtSymbol);
}
