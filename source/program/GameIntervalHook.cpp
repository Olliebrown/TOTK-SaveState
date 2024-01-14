#include "GameIntervalHook.hpp"
#include "DeviceGetProcAddressHook.hpp"
#include "MovieMediaDataHook.hpp"
#include "MovieRenderHook.hpp"
#include "main.hpp"

#include "nn/fs.hpp"
#include "logger.hpp"
#include "lib/simpleini-4.20/SimpleIni.h"

// sTargetInterval is the interval of FPS target, 1 = 60, 2 = 30, 3 = 20
int32_t frame = 1;
int32_t gMovieFrames = -1;
float gMovieFrametime = NORMAL_FRAMETIME;
float gMovieTime = 0.0f;
bool gInMovie = false;
bool idk = true;

void FPS_Interval() {
    /* Mount ROM */
    nn::fs::MountRom("content");
    nn::fs::MountSdCard("sd");

    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "Program ID", CUR_PROCESS_HANDLE, 18, 0, true);

    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "Heap Address", CUR_PROCESS_HANDLE, 4, 0, true);
    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "Heap Size", CUR_PROCESS_HANDLE, 5, 0, true);

    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "Stack Address", CUR_PROCESS_HANDLE, 14, 0, true);
    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "Stack Size", CUR_PROCESS_HANDLE, 15, 0, true);

    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "ASLR Address", CUR_PROCESS_HANDLE, 12, 0, true);
    OutputSysInfo("sd:/DFPS++_DEBUG.txt", "ASLR Size", CUR_PROCESS_HANDLE, 13, 0, true);

    const char* iniFilePath = "content:/dfps/default.ini";
    nn::fs::FileHandle input_handle = {};

    // Open the INI file for reading
    nn::Result openResult = nn::fs::OpenFile(std::addressof(input_handle), iniFilePath, nn::fs::OpenMode_Read);
    if (!openResult.isSuccess()) {
        OutputDebug("sd:/DFPS++_DEBUG.txt", "Error Opening the file.", true);
        return;
    }

    // Get the size of the file
    long int file_size = 0;
    nn::Result sizeResult = nn::fs::GetFileSize(std::addressof(file_size), input_handle);
    if (!sizeResult.isSuccess()) {
        OutputDebug("sd:/DFPS++_DEBUG.txt", "Error Getting the file.", true);
        return;
    }

    // Allocate a buffer based on the file size
    char* buffer = new char[file_size];
    nn::Result readResult = nn::fs::ReadFile(input_handle, 0, buffer, file_size);
    if (!readResult.isSuccess()) {
        OutputDebug("sd:/DFPS++_DEBUG.txt", "Error reading the file.", true);
        return;
    }

    // Pass the data through a std::string (sanitizes the data)
    std::string iniContent(buffer, file_size);
    const char* iniContentCStr = iniContent.c_str();

    // Attempt to interpret as ini data
    CSimpleIniA ini;
    if (!ini.LoadData(iniContentCStr)) {
        OutputDebug("sd:/DFPS++_DEBUG.txt", "FAILED TO READ INI FILE.", true);
        OutputDebug("sd:/DFPS++_DEBUG.txt", "INPUT INI:", true);
        OutputDebug("sd:/DFPS++_DEBUG.txt", iniContentCStr, true);
    }

    // Report the parsed max framerate as a string
    const char* maxFramerate = ini.GetValue("dFPS", "MaxFramerate", "-1");
    OutputDebug("sd:/DFPS++_DEBUG.txt", "FPS VALUE:", false);
    OutputDebug("sd:/DFPS++_DEBUG.txt", maxFramerate, true);

    // Convert the string to an integer
    int maxFramerateValue = std::stoi(maxFramerate);
    if (maxFramerateValue > 0) {
        // The MaxFramerate value was found
        if (maxFramerateValue == 60) {
            frame = 1;
        }
        else if (maxFramerateValue == 30) {
            frame = 2;
        }
        else if (maxFramerateValue == 20) {
            frame = 3;
        }
        else {
            OutputDebug("sd:/DFPS++_DEBUG.txt", "Max DFPS++.ini, Bad Value.", true);
            frame = 1; // Default value if not found
        }
    }

    // Cleanup
    delete[] buffer;
    nn::fs::CloseFile(input_handle);
    return;
}

void GameIntervalHook::Callback(IntervalFunc_Arg0* a0, IntervalFunc_Arg1* a1) {
    // Do we need to read the ini file? (once after startup)
    if (idk == true) {
        idk = false;
        FPS_Interval();
    }

    int32_t interval = frame;

    // Are we trying to play a movie?
    if (gFramesSinceLastMovieFrame != -1) {
        // Is this a movie frame?
        if (gFramesSinceLastMovieFrame == 0) {
            interval = 60 / gMovieFramerate;
            gMovieFrametime = 1.0f / (float)gMovieFramerate;
            gInMovie = true;
        }

        // Count movie frames
        gFramesSinceLastMovieFrame++;

        // If we pass 15 frames without a movie frame, the movie is over
        if (gFramesSinceLastMovieFrame > 16) {
            gFramesSinceLastMovieFrame = -1;
            gMovieFrames = -1;
            gMovieTime = 0.0f;
            gInMovie = false;
        }
    }

    // Check arguments before proceeding
    if (a0 == nullptr || a1 == nullptr) {
        return;
    }

    // game runs at the incorrect speed if we don't do this
#if DYNAMIC_TIMESTEP==0
    a0->unk_0x14 = interval;
    a0->unk_0x18 = interval;
#else
    a0->unk_0x14 = GAME_UPDATE_INTERVAL;
    a0->unk_0x18 = GAME_UPDATE_INTERVAL;
#endif
    a0->unk_0x1C = 0;

    if (a1->unk_0x140 == nullptr) {
        return;
    }

    if (a1->unk_0x140->window == nullptr) {
        return;
    }

    if (gNvnWindowSetPresentInterval != nullptr) {
        gNvnWindowSetPresentInterval(a1->unk_0x140->window, interval);
    }
}

const PatchInfo GameIntervalHook::CreatePatchInfo(PatchGameVersion version, uintptr_t function) {
    return PatchInfo(version, PatchType::DETOUR, function, InstallAtOffset, InstallAtPtr, InstallAtSymbol);
}


