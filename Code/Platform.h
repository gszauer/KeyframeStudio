#pragma once

#include <string>

extern "C" void UI_WriteClipboard(const char* buffer); // Can Probably remove
//extern "C" const char* UI_SelectFile(); // Needs to become asynch

// These should be considered finished
std::string MakeNewGuid();

typedef void(*PlatformReadFileResult)(const char* path, unsigned char* buffer, unsigned int size);
extern "C" void PlatformReadFile(const char* fileName, PlatformReadFileResult result);

typedef void(*PlatformSaveAsResult)(bool success);
extern "C" void PlatformSaveAs(const unsigned char* data, unsigned int size, PlatformSaveAsResult result);

typedef void(*PlatformSelectFileResult)(const char* path);
extern "C" void PlatformSelectFile(const char* filter, PlatformSelectFileResult result);
