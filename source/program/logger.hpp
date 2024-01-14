// logger.hpp
#ifndef LOGGER_HPP
#define LOGGER_HPP

void OutputDebug(const char* path, const char* output, bool new_line);
void OutputSysInfo(const char* path, const char* prefix, ::Handle handle, u32 infoType, u64 infoSubType, bool new_line);

#endif // LOGGER_HPP
