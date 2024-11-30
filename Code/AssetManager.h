#pragma once

#include <vector>
#include <map>
#include <string>
#include <list>
#include "Libraries/imgui.h"
#include "Libraries/imgui_internal.h"
#include "Asset.h"

typedef void(*AssetManagerLoadCallback)(const char* fileName, Asset* resultAsset);

class AssetManager {
private:
	static AssetManager* instance;
public:
	static AssetManager* GetInstance();
	static void DestroyAll();
protected:
	std::map<std::string, Asset*> assets; // Key is uuid

	AssetManager();
	~AssetManager();

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	std::string _lastRequestedAsset; // Initial value doesn't matter. Used as a temp helper for callbacks
	AssetManagerLoadCallback _lastCallback;
public:
	Asset* Get(const std::string& uuid);
	
	void LoadImageFromFile(const std::string& fileName, AssetManagerLoadCallback callback);
	void LoadAtlasFromFile(const std::string& atlasName, const std::string& fileName, AssetManagerLoadCallback callback);

	AnimationAsset* NewAnimation(const std::string& name, int frameRate, int frameCount, bool looping);
	AtlasAsset* NewAtlas(const std::string& name);

	ImageAsset* DeserializeImage(const char* name, const char* guid, const void* data, int data_len);
	AtlasAsset* DeserializeAtlas(const char* name, const char* guid, int w, int h);
	AnimationAsset* DeserializeAnimation(const char* name, const char* guid, int frameRate, int frameCount, bool looping);

	std::map<std::string, Asset*>::iterator Begin();
	std::map<std::string, Asset*>::iterator End();
};