#include <string>

std::string MakeNewGuid() {
	return "-INVALID-GUID-";
}

extern "C" void PlatformSaveAs(const unsigned char* data, unsigned int size, PlatformSaveAsResult result) {
    result(false);
}

extern "C" void PlatformSelectFile(const char* filter, PlatformSelectFileResult result) {
    result(0);
}