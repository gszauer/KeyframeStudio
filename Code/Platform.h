#pragma once

#include <string>

// These should be considered finished
std::string MakeNewGuid();

#if 0
typedef void(*PlatformReadFileResult)(const char* path, unsigned char* buffer, unsigned int size);
extern "C" void PlatformReadFile(const char* fileName, PlatformReadFileResult result);
#endif

typedef void(*PlatformSaveAsResult)(bool success);
extern "C" void PlatformSaveAs(const unsigned char* data, unsigned int size, PlatformSaveAsResult result);

typedef void(*PlatformSelectFileResult)(const char* path, unsigned char* buffer, unsigned int size);
extern "C" void PlatformSelectFile(const char* inputFilter, PlatformSelectFileResult result);

extern "C" void PlatformOpenURL(const char* url);