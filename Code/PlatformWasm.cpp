#include <string>

// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#implement-c-in-javascript

extern "C" void JS_GenGUID(const char* ptr);
extern "C" void JS_OpenURL(const char* ptr);
extern "C" void JS_PresentFile(void* data, unsigned int size);
extern "C" void JS_SelectFile(PlatformSelectFileResult callback);

std::string MakeNewGuid() {
    char result[40] = { 0 };
	JS_GenGUID(result);
    return result;
}

extern "C" void PlatformSaveAs(const unsigned char* data, unsigned int size, PlatformSaveAsResult result) {
    JS_PresentFile((void*)data, size);
    result(true);
}

extern "C" void WASM_InvokePlatformSelectCallback(PlatformSelectFileResult target, const char* path, unsigned char* buffer, unsigned int size) {
    if (target != 0) {
        target(path, buffer, size);
    }
}

extern "C" void PlatformSelectFile(const char* filter, PlatformSelectFileResult result) {
    JS_SelectFile(result);
}

extern "C" void PlatformOpenURL(const char* url) {
    JS_OpenURL(url);
}