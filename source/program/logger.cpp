#include "lib/nx/nx.h"
#include "nn/fs.hpp"
#include "lib/simpleini-4.20/SimpleIni.h"
#include "logger.hpp"

void OutputDebug(const char* path, const char* output, bool new_line) {
    const size_t output_len = ::strlen(output);
    const size_t file_len = (new_line == true) ? output_len + 1 : output_len;

    long int last_size = 0;
    nn::fs::FileHandle output_handle = {};
    ::Result openResult = nn::fs::OpenFile(std::addressof(output_handle), path, nn::fs::OpenMode_Write);
    if (openResult != 0) {
        nn::fs::CreateFile(path, file_len);
        openResult = nn::fs::OpenFile(std::addressof(output_handle), path, nn::fs::OpenMode_Write);
    }
    else {
        nn::fs::GetFileSize(std::addressof(last_size), output_handle);
        nn::fs::SetFileSize(output_handle, file_len + last_size);
    }

    if (openResult == 0) {
        nn::fs::WriteOption option0(0);

        nn::fs::WriteFile(output_handle, last_size, output, output_len, option0);
        if (new_line == true) {
            nn::fs::WriteFile(output_handle, output_len + last_size, "\n", 1, option0);
        }

        nn::fs::FlushFile(output_handle);
        nn::fs::CloseFile(output_handle);
    }
}

void OutputSysInfo(const char* path, const char* prefix, ::Handle handle, u32 infoType, u64 infoSubType, bool new_line) {
    u64 progInfo = 0;
    char PIDStr[256];

    ::Result infoResult = svcGetInfo(&progInfo, infoType, handle, infoSubType);
    if (infoResult != 0) {
        OutputDebug(path, "Error getting program info.", true);
    } else {
        snprintf(PIDStr, sizeof(PIDStr), "%-20s: 0x%016lX", prefix, progInfo);
        OutputDebug(path, PIDStr, new_line);
    }
}
