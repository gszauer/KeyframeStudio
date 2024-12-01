#include "AssetManager.h"
#include "Libraries/gabor_json.h"
#include "Application.h"
#include "Platform.h"

#include <algorithm>
#include <iostream>

AssetManager* AssetManager::instance = 0;
AssetManager::AssetManager() { }
AssetManager::~AssetManager() { }

AssetManager* AssetManager::GetInstance() {
    if (AssetManager::instance == 0) {
        AssetManager::instance = new AssetManager();
    }
    return AssetManager::instance;
}

void AssetManager::DestroyAll() {
    if (AssetManager::instance != 0) {
        for (auto iter = instance->Begin(), end = instance->End(); iter != end; iter++) {
            delete iter->second;
        }
    }
    delete AssetManager::instance;
    AssetManager::instance = 0;
}

Asset* AssetManager::Get(const std::string& uuid) {
    if (assets.count(uuid) == 0) {
        return 0;
    }
    return assets[uuid];
}

AtlasAsset* AssetManager::LoadAtlasFromMemory(const std::string& atlasName, const char* fileName, unsigned char* buffer, unsigned int size) {
    if (buffer != 0 && size != 0 && fileName != 0 && fileName[0] != 0) {
        JsonValue* root = JsonParseString((char*)buffer, size);
        AtlasAsset* result = AssetManager::GetInstance()->NewAtlas(atlasName);

        if (root != 0 && root->type == JsonValueType::JSON_OBJECT) {
            ImVec2 imgSize(1, 1);
            JsonValue* meta = JsonObjectGet(root, "meta");
            if (meta != 0) {
                JsonValue* size = JsonObjectGet(meta, "size");
                if (size != 0) {
                    if (JsonObjectGet(size, "w") != 0) {
                        imgSize.x = (float)JsonObjectGet(size, "w")->asNumber;
                    }
                    if (JsonObjectGet(size, "h") != 0) {
                        imgSize.y = (float)JsonObjectGet(size, "h")->asNumber;
                    }
                }
            }
            result->sourceSize = imgSize;

            JsonValue* frames = JsonObjectGet(root, "frames");
            if (frames != 0 && frames->type == JsonValueType::JSON_OBJECT) {
                /*for (int i = 0; i < frames->asObject.count; ++i) {
                    std::cout << "Loaded frame: " << frames->asObject.names[i] << "\n";
                }*/
                for (JsonIterator iter = JsonGetIterator(frames); iter.valid; JsonIteratorAdvance(&iter)) {
                    std::string frameName = "::ERROR";
                    if (iter.name != 0) {
                        frameName = iter.name;
                    }

                    if (iter.value == 0 || iter.value->type != JsonValueType::JSON_OBJECT) {
                        continue;
                    }
                    JsonValue* frame = iter.value;

                    ImVec2 minP(0, 0);
                    ImVec2 maxP(imgSize.x, imgSize.y);

                    JsonValue* spriteFrame = JsonObjectGet(frame, "frame");
                    if (spriteFrame != 0) {
                        if (JsonObjectGet(spriteFrame, "x") != 0) {
                            minP.x = (float)JsonObjectGet(spriteFrame, "x")->asNumber;
                            //minP.x /= imgSize.x;
                        }
                        if (JsonObjectGet(spriteFrame, "y") != 0) {
                            minP.y = (float)JsonObjectGet(spriteFrame, "y")->asNumber;
                            //minP.y /= imgSize.y;
                        }
                        if (JsonObjectGet(spriteFrame, "w") != 0) {
                            maxP.x = (float)JsonObjectGet(spriteFrame, "w")->asNumber;
                            //maxP.x /= imgSize.x;
                            maxP.x += minP.x;
                        }
                        if (JsonObjectGet(spriteFrame, "h") != 0) {
                            maxP.y = (float)JsonObjectGet(spriteFrame, "h")->asNumber;
                            //maxP.y /= imgSize.y;
                            maxP.y += minP.y;
                        }
                    }

                    ImVec2 p0 = minP;
                    ImVec2 p1(maxP.x, minP.y);
                    ImVec2 p2 = maxP;
                    ImVec2 p3(minP.x, maxP.y);

                    bool rotation = false;
                    JsonValue* rotated = JsonObjectGet(frame, "rotated");
                    if (rotated != 0) {
                        JsonObjectGet(frame, "rotated");;
                        if (rotated->asBool) {
                            rotation = true;
                        }
                    }

                    result->AddFrame(frameName, p0, p1, p2, p3, rotation);
                }
            }
        }
        JsonFree(root);
        
        return result;

    }
    return 0;
}

#if 0
void AssetManager::LoadAtlasFromFile(const std::string& atlasName, const std::string& fileName, AssetManagerLoadCallback callback) {
    Application::GetInstance()->_FileOperationStarted();
    _lastRequestedAsset = atlasName;
    _lastCallback = callback;

    PlatformReadFile(fileName.c_str(), 
        [](const char* _fileName, unsigned char* buffer, unsigned int size) {
            if (buffer != 0) {

                JsonValue* root = JsonParseString((char*)buffer, size);

                AtlasAsset* result = AssetManager::GetInstance()->NewAtlas(AssetManager::GetInstance()->_lastRequestedAsset);

                if (root != 0 && root->type == JsonValueType::JSON_OBJECT) {
                    ImVec2 imgSize(1, 1);
                    JsonValue* meta = JsonObjectGet(root, "meta");
                    if (meta != 0) {
                        JsonValue* size = JsonObjectGet(meta, "size");
                        if (size != 0) {
                            if (JsonObjectGet(size, "w") != 0) {
                                imgSize.x = (float)JsonObjectGet(size, "w")->asNumber;
                            }
                            if (JsonObjectGet(size, "h") != 0) {
                                imgSize.y = (float)JsonObjectGet(size, "h")->asNumber;
                            }
                        }
                    }
                    result->sourceSize = imgSize;

                    JsonValue* frames = JsonObjectGet(root, "frames");
                    if (frames != 0 && frames->type == JsonValueType::JSON_OBJECT) {
                        /*for (int i = 0; i < frames->asObject.count; ++i) {
                            std::cout << "Loaded frame: " << frames->asObject.names[i] << "\n";
                        }*/
                        for (JsonIterator iter = JsonGetIterator(frames); iter.valid; JsonIteratorAdvance(&iter)) {
                            std::string frameName = "::ERROR";
                            if (iter.name != 0) {
                                frameName = iter.name;
                            }

                            if (iter.value == 0 || iter.value->type != JsonValueType::JSON_OBJECT) {
                                continue;
                            }
                            JsonValue* frame = iter.value;

                            ImVec2 minP(0, 0);
                            ImVec2 maxP(imgSize.x, imgSize.y);

                            JsonValue* spriteFrame = JsonObjectGet(frame, "frame");
                            if (spriteFrame != 0) {
                                if (JsonObjectGet(spriteFrame, "x") != 0) {
                                    minP.x = (float)JsonObjectGet(spriteFrame, "x")->asNumber;
                                    //minP.x /= imgSize.x;
                                }
                                if (JsonObjectGet(spriteFrame, "y") != 0) {
                                    minP.y = (float)JsonObjectGet(spriteFrame, "y")->asNumber;
                                    //minP.y /= imgSize.y;
                                }
                                if (JsonObjectGet(spriteFrame, "w") != 0) {
                                    maxP.x = (float)JsonObjectGet(spriteFrame, "w")->asNumber;
                                    //maxP.x /= imgSize.x;
                                    maxP.x += minP.x;
                                }
                                if (JsonObjectGet(spriteFrame, "h") != 0) {
                                    maxP.y = (float)JsonObjectGet(spriteFrame, "h")->asNumber;
                                    //maxP.y /= imgSize.y;
                                    maxP.y += minP.y;
                                }
                            }

                            ImVec2 p0 = minP;
                            ImVec2 p1(maxP.x, minP.y);
                            ImVec2 p2 = maxP;
                            ImVec2 p3(minP.x, maxP.y);

                            bool rotation = false;
                            JsonValue* rotated = JsonObjectGet(frame, "rotated");
                            if (rotated != 0) {
                                JsonObjectGet(frame, "rotated");;
                                if (rotated->asBool) {
                                    rotation = true;
                                }
                            }

                            result->AddFrame(frameName, p0, p1, p2, p3, rotation);
                        }
                    }
                }
                JsonFree(root);

                AssetManager::GetInstance()->_lastCallback(_fileName, result);
                
            }
            else {
                AssetManager::GetInstance()->_lastCallback(_fileName, 0);
            }
            AssetManager::GetInstance()->_lastCallback = 0;
            AssetManager::GetInstance()->_lastRequestedAsset = "";
            Application::GetInstance()->_FileOperationFinished();
        }
    );

#if 0
    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD fileBytes = GetFileSize(hFile, 0);
    if (hFile == INVALID_HANDLE_VALUE || fileBytes == 0) {
        CloseHandle(hFile);
        return 0;
    }

    char* target = (char*)malloc(fileBytes + 1);
    if (ReadFile(hFile, target, fileBytes, &fileBytes, NULL) == 0) {
        CloseHandle(hFile);
        return 0;
    }
    target[fileBytes] = 0;
    CloseHandle(hFile);

    JsonValue* root = JsonParseString((char*)target, 0);
    free(target);

    AtlasAsset* result = NewAtlas(atlasName);

    if (root != 0 && root->type == JsonValueType::JSON_OBJECT) {
        ImVec2 imgSize(1, 1);
        JsonValue* meta = JsonObjectGet(root, "meta");
        if (meta != 0) {
            JsonValue* size = JsonObjectGet(meta, "size");
            if (size != 0) {
                if (JsonObjectGet(size, "w") != 0) {
                    imgSize.x = (float)JsonObjectGet(size, "w")->asNumber;
                }
                if (JsonObjectGet(size, "h") != 0) {
                    imgSize.y = (float)JsonObjectGet(size, "h")->asNumber;
                }
            }
        }
        result->sourceSize = imgSize;

        JsonValue* frames = JsonObjectGet(root, "frames");
        if (frames != 0 && frames->type == JsonValueType::JSON_OBJECT) {
            /*for (int i = 0; i < frames->asObject.count; ++i) {
                std::cout << "Loaded frame: " << frames->asObject.names[i] << "\n";
            }*/
            for (JsonIterator iter = JsonGetIterator(frames); iter.valid; JsonIteratorAdvance(&iter)) {
                std::string frameName = "::ERROR";
                if (iter.name != 0) {
                    frameName = iter.name;
                }

                if (iter.value == 0 || iter.value->type != JsonValueType::JSON_OBJECT) {
                    continue;
                }
                JsonValue* frame = iter.value;

                ImVec2 minP(0, 0);
                ImVec2 maxP(imgSize.x, imgSize.y);

                JsonValue* spriteFrame = JsonObjectGet(frame, "frame");
                if (spriteFrame != 0) {
                    if (JsonObjectGet(spriteFrame, "x") != 0) {
                        minP.x = (float)JsonObjectGet(spriteFrame, "x")->asNumber;
                        //minP.x /= imgSize.x;
                    }
                    if (JsonObjectGet(spriteFrame, "y") != 0) {
                        minP.y = (float)JsonObjectGet(spriteFrame, "y")->asNumber;
                        //minP.y /= imgSize.y;
                    }
                    if (JsonObjectGet(spriteFrame, "w") != 0) {
                        maxP.x = (float)JsonObjectGet(spriteFrame, "w")->asNumber;
                        //maxP.x /= imgSize.x;
                        maxP.x += minP.x;
                    }
                    if (JsonObjectGet(spriteFrame, "h") != 0) {
                        maxP.y = (float)JsonObjectGet(spriteFrame, "h")->asNumber;
                        //maxP.y /= imgSize.y;
                        maxP.y += minP.y;
                    }
                }

                ImVec2 p0 = minP;
                ImVec2 p1(maxP.x, minP.y);
                ImVec2 p2 = maxP;
                ImVec2 p3(minP.x, maxP.y);

                bool rotation = false;
                JsonValue* rotated = JsonObjectGet(frame, "rotated");
                if (rotated != 0) {
                    JsonObjectGet(frame, "rotated");;
                    if (rotated->asBool) {
                        rotation = true;
                    }
                }

                result->AddFrame(frameName, p0, p1, p2, p3, rotation);
            }
        }
    }

    JsonFree(root);
    return result;
#endif
}
#endif

ImageAsset* AssetManager::DeserializeImage(const char* name, const char* guid, const void* data, int data_len) {
    ImageAsset* result = new ImageAsset(name, data, data_len);
    result->uuid = guid;
    assets[result->uuid] = result;
    return result;
}

ImageAsset* AssetManager::LoadImageFromMemory(const char* _fileName_, unsigned char* buffer, unsigned int size) {
    if (buffer != 0 && _fileName_ != 0 && _fileName_[0] != 0 && size != 0) {
        std::string fileName = _fileName_;
        size_t lastIndex = fileName.find_last_of('/');
        if (lastIndex == std::string::npos) {
            lastIndex = fileName.find_last_of("\\");
        }
        if (lastIndex != std::string::npos) {
            fileName = fileName.substr(lastIndex + 1);
        }

        ImageAsset* result = new ImageAsset(fileName, (const void*)buffer, (size_t)size);
        AssetManager::GetInstance()->assets[result->uuid] = result;
        return result;
    }
    
    return 0;
}

#if 0
void AssetManager::LoadImageFromFile(const std::string& fileName, AssetManagerLoadCallback callback) {
    Application::GetInstance()->_FileOperationStarted();
    _lastCallback = callback;

    PlatformReadFile(fileName.c_str(),
        [](const char* _fileName_, unsigned char* buffer, unsigned int size) {
            if (buffer != 0) {
                std::string _fileName = _fileName_;
                size_t lastIndex = _fileName.find_last_of('/');
                if (lastIndex == std::string::npos) {
                    lastIndex = _fileName.find_last_of("\\");
                }

                std::string fileName = _fileName;
                if (lastIndex != std::string::npos) {
                    fileName = _fileName.substr(lastIndex + 1);
                }

                ImageAsset* result = new ImageAsset(fileName, (const void*)buffer, (size_t)size);
                AssetManager::GetInstance()->assets[result->uuid] = result;
                AssetManager::GetInstance()->_lastCallback(_fileName_, result);
            }
            else {
                AssetManager::GetInstance()->_lastCallback(_fileName_, 0);
            }
            AssetManager::GetInstance()->_lastCallback = 0;
            Application::GetInstance()->_FileOperationFinished();
        }
    );
#endif

#if 0
    size_t lastIndex = _fileName.find_last_of('/');
    if (lastIndex == std::string::npos) {
        lastIndex = _fileName.find_last_of("\\");
    }

    std::string fileName = _fileName;
    if (lastIndex != std::string::npos) {
        fileName = _fileName.substr(lastIndex + 1);
    }

    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD fileBytes = GetFileSize(hFile, 0);

    if (hFile == INVALID_HANDLE_VALUE || fileBytes == 0) {
        CloseHandle(hFile);
        return 0;
    }

    void* target = malloc(fileBytes);
    if (ReadFile(hFile, target, fileBytes, &fileBytes, NULL) == 0) {
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);

    ImageAsset* result = new ImageAsset(fileName, target, fileBytes);
    assets[result->uuid] = result;

    free(target);
    return result;
}
#endif

std::map<std::string, Asset*>::iterator AssetManager::Begin() {
    return assets.begin();
}

std::map<std::string, Asset*>::iterator AssetManager::End() {
    return assets.end();
}

AnimationAsset* AssetManager::NewAnimation(const std::string& name, int frameRate, int frameCount, bool looping) {
    AnimationAsset* result = new AnimationAsset(name, frameCount, frameRate, looping);
    assets[result->uuid] = result;
    return result;
}

AtlasAsset* AssetManager::NewAtlas(const std::string& name) {
    AtlasAsset* result = new AtlasAsset(name);
    assets[result->GetGUID()] = result;
    return result;
}


AtlasAsset* AssetManager::DeserializeAtlas(const char* name, const char* guid, int w, int h) {
    AtlasAsset* result = new AtlasAsset(name);
    result->SetGuid(guid);
    result->sourceSize.x = (float)w;
    result->sourceSize.y = (float)h;

    assets[result->GetGUID()] = result;
    return result;
}

AnimationAsset* AssetManager::DeserializeAnimation(const char* name, const char* guid, int frameRate, int frameCount, bool looping) {
    AnimationAsset* result = new AnimationAsset(name, frameCount, frameRate, looping);
    result->uuid = guid;

    assets[result->uuid] = result;
    return result;
}