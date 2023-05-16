#include "../loader.h"
#include "../memory.h"
#include "../assert.h"
#include <windows.h>

extern HWND gHwnd;

extern "C" void SaveFile(const char* path, const void* data, unsigned int bytes) {
	DWORD written = 0;
	HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, data, (DWORD)bytes, &written, NULL);
	CloseHandle(hFile);
}

extern "C" bool PresentFile(const void* data, unsigned int bytes) {
	char path[1024] = { 0 };
	ZeroMemory(path, 1024);

    OPENFILENAMEA saveFileName;
	ZeroMemory(&saveFileName, sizeof(OPENFILENAMEA));
    saveFileName.lStructSize = sizeof(OPENFILENAMEA);
	saveFileName.hwndOwner =  gHwnd;
    saveFileName.lpstrFile = path;
    saveFileName.nMaxFile = 1024;
	saveFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (FALSE != GetSaveFileNameA(&saveFileName)) {
		SaveFile(saveFileName.lpstrFile, data, bytes);
		return true;
	}
	return false;
}

namespace Loader_Internal {
	struct AsynchRequestUserData {
		void* userData;
		OnFileLoaded callback;
		DWORD dwFileSize;
	};
}

extern "C" void RequestFileAsynch(OnFileLoaded callback, void* userData) {
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = gHwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = ("All\0*.*\0png\0*.png\0kfs\0*.kfs\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE) {
		if (callback != 0) {
			HANDLE file = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD dwFileSize = GetFileSize(file, NULL);
			CloseHandle(file);

			void* loadArena = MemAlloc(dwFileSize + 1);
			Loader_Internal::AsynchRequestUserData* req = (Loader_Internal::AsynchRequestUserData*)MemAlloc(sizeof(Loader_Internal::AsynchRequestUserData));
			req->callback = callback;
			req->userData = userData;
			req->dwFileSize = dwFileSize;
			
			LoadFileAsynch(ofn.lpstrFile, loadArena, dwFileSize, [](const char* path, void* data, unsigned int bytes, void* userData) {
				Loader_Internal::AsynchRequestUserData* req = (Loader_Internal::AsynchRequestUserData*)userData;

				if (bytes == 0) {
					if (req->callback) {
						req->callback(0, 0, 0, req->userData);
					}
				}
				else {
					PlatformAssert(bytes == req->dwFileSize, __LOCATION__);
					if (req->callback) {
						req->callback(path, data, bytes, req->userData);
					}
				}

				MemRelease(req);
				//MemRelease(data);
			}, req);
		}
		else { // You probably forgot to add a callback
			PlatformAssert(false, __LOCATION__);
		}
	}
	else {
		if (callback != 0) {
			callback(0, 0, 0, userData);
		}
	}
}


extern "C" void LoadFileAsynch(const char* path, void* target, unsigned int bytes, OnFileLoaded callback, void* userData) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytesInFile = GetFileSize(hFile, 0);
	DWORD bytesRead = 0;

    if (bytesInFile > bytes) {
        bytesInFile = bytes;
    }

    if (hFile == INVALID_HANDLE_VALUE) {
		PlatformAssert(false, __LOCATION__);
		callback(path, target, 0, userData);
		return;
	}
	if (ReadFile(hFile, target, bytesInFile, &bytesRead, NULL) == 0) {
		CloseHandle(hFile);
		callback(path, target, 0, userData);
		return;
	}

	CloseHandle(hFile);
	callback(path, target, bytesRead, userData);
}