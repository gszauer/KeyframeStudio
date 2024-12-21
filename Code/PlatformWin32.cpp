#include "Platform.h"
#include "Libraries/imgui.h"

#include <Windows.h>
#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <rpc.h>
#include <cstring>
#include <iostream>

std::string MakeNewGuid() {
	UUID _uuid;
	UuidCreate(&_uuid);

	char* str;
	UuidToStringA(&_uuid, (RPC_CSTR*)&str);
	std::string result = str;
	RpcStringFreeA((RPC_CSTR*)&str);

	return result;
}

#if 0
extern "C" void UI_WriteClipboard(const char* buffer) {
    if (buffer == 0) {
        return;
    }
    const char* null = buffer;
    while (*null) {
        ++null;
    }
    int len = (int)(null - buffer);

    HGLOBAL hdst;
    LPWSTR dst;

    // Allocate string
    hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR));
    dst = (LPWSTR)GlobalLock(hdst);

    for (int i = 0; i < len; ++i) {
        dst[i] = buffer[i];
    }
    dst[len] = 0;
    GlobalUnlock(hdst);

    // Set clipboard data
    if (!OpenClipboard(NULL)) {
        IM_ASSERT(false);
    }
    EmptyClipboard();
    if (!SetClipboardData(CF_UNICODETEXT, hdst)) {
        IM_ASSERT(false);
    }
    CloseClipboard();
}
#endif

#if 0
extern "C" void PlatformReadFile(const char* fileName, PlatformReadFileResult result) {
    HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD bytesInFile = GetFileSize(hFile, 0);
    DWORD bytesRead = 0;

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }
    unsigned char* fileBuffer = new unsigned char[bytesInFile + 1];

    if (ReadFile(hFile, fileBuffer, bytesInFile, &bytesRead, NULL) != 0) {
        fileBuffer[bytesInFile] = 0; // Force json parser to stop
        result(fileName, fileBuffer, bytesInFile);
    }
    else {
        result(fileName, 0, 0);
    }

    delete[] fileBuffer;
    CloseHandle(hFile);
}
#endif

extern "C" void PlatformSaveAs(const unsigned char* data, unsigned int size, PlatformSaveAsResult result) {
    static char path[1024] = { 0 };
    ZeroMemory(path, 1024);

    OPENFILENAMEA saveFileName;
    ZeroMemory(&saveFileName, sizeof(OPENFILENAMEA));
    saveFileName.lStructSize = sizeof(OPENFILENAMEA);
    saveFileName.hwndOwner = GetActiveWindow();// gHwnd;
    saveFileName.lpstrFile = path;
    saveFileName.lpstrFilter = "Key Frame Studio\0*.kfs\0All\0*.*\0\0";
    saveFileName.nMaxFile = 1024;
    saveFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (FALSE != GetSaveFileNameA(&saveFileName)) {
        DWORD written = 0;
        HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        WriteFile(hFile, data, (DWORD)size, &written, NULL);
        CloseHandle(hFile);

        result(true);
    }
    else {
        result(false);
    }
}

extern "C" void PlatformOpenURL(const char* url) {
    ShellExecuteA(0, 0, url, 0, 0, SW_SHOW);
}

#if 0
extern "C" void PlatformSelectFile(const char* filter, PlatformSelectFileResult result) {
    static CHAR UI_fileNameBuffer[1024] = { 0 };
    memset(UI_fileNameBuffer, 0, 1024);
    OPENFILENAMEA ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = UI_fileNameBuffer;
    ofn.nMaxFile = sizeof(UI_fileNameBuffer);
    if (filter != 0) {
        ofn.lpstrFilter = (filter);
    }
    else {
        ofn.lpstrFilter = ("All\0*.*\0png\0*.png\0kfs\0*.kfs\0\0");
    }

    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        result(UI_fileNameBuffer);
    }
}
#endif

extern "C" void PlatformSelectFile(const char* filter, PlatformSelectFileResult result) {
    static CHAR UI_fileNameBuffer[1024] = { 0 };
    memset(UI_fileNameBuffer, 0, 1024);
    OPENFILENAMEA ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = UI_fileNameBuffer;
    ofn.nMaxFile = sizeof(UI_fileNameBuffer);
    if (filter != 0) {
        ofn.lpstrFilter = (filter);
    }
    else {
        ofn.lpstrFilter = ("All\0*.*\0png\0*.png\0kfs\0*.kfs\0\0");
    }

    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        HANDLE hFile = CreateFileA(UI_fileNameBuffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        DWORD bytesInFile = GetFileSize(hFile, 0);
        DWORD bytesRead = 0;

        if (hFile == INVALID_HANDLE_VALUE) {
            return;
        }
        unsigned char* fileBuffer = new unsigned char[bytesInFile + 1];

        if (ReadFile(hFile, fileBuffer, bytesInFile, &bytesRead, NULL) != 0) {
            fileBuffer[bytesInFile] = 0; // Force json parser to stop
            result(UI_fileNameBuffer, fileBuffer, bytesInFile);
        }
        else {
            result(UI_fileNameBuffer, 0, 0);
        }

        delete[] fileBuffer;
        CloseHandle(hFile);
    }
    else {
        result(0, 0, 0);
    }
}