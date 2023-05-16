#ifndef _H_LOADER_
#define _H_LOADER_

typedef void (*OnFileLoaded)(const char* path, void* data, unsigned int bytes, void* userData);

extern "C" void LoadFileAsynch(const char* path, void* target, unsigned int bytes, OnFileLoaded callback, void* userData);

// It's not the cleanest API, but RequestFileAsynch will call MemAlloc for you. It's up to you to call
// MemFree on the data argument of the callback , if it's not null.
// In WASM, the callback does not trigger on cancel.
extern "C" void RequestFileAsynch(OnFileLoaded callback, void* userData);

extern "C" void SaveFile(const char* path, const void* data, unsigned int bytes);
extern "C" bool PresentFile(const void* data, unsigned int bytes);

#define WASM_LOADER_ENABLE_CALLBACKS __attribute__ (( visibility( "default" ) )) extern "C" void TriggerFileLoaderCallback(OnFileLoaded callback, const char* path, void* data, unsigned int bytes, void* userdata) { callback(path, data, bytes, userdata); }

#endif