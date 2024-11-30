#pragma once

#include "Libraries/sokol_app.h"
#include "Libraries/sokol_gfx.h"
#include "Libraries/sokol_imgui.h"
#include "Track.h"
#include "SceneNode.h"
#include <map>

enum class AssetType {
	ASSET_INVALID = 0,
	ASSET_IMAGE,
	ASSET_ATLAS,
	ASSET_ANIMATION,
	ASSET_FRAME
};

class Asset {
public:
	bool deleted;
protected:
	std::string uuid;
	AssetType rtti;
public:
	static Asset* clicked;

	std::string name;
	std::string nameInput;

	Asset();
	virtual ~Asset();
	virtual void EditorImmediateInspector(int* selectedFrame);
	void Serialize(std::stringstream& out, int indent);

	const std::string& GetGUID();
	AssetType GetType();

	bool IsDeleted();
protected:
	Asset(const Asset&) = delete;
	Asset& operator=(const Asset&) = delete;

	virtual void SerializeInner(std::stringstream& out, int indent);
};

class ImageAsset : public Asset {
	friend class AssetManager;
public:
	ImTextureID id;
	sg_image cleanup;
	simgui_image_t cleanup_gui;
	ImVec2 size;
	std::string base64;

	virtual ~ImageAsset();
	virtual void EditorImmediateInspector(int* selectedFrame);
protected:
	ImageAsset(const std::string& name, const void* data, size_t data_size);
	virtual void SerializeInner(std::stringstream& out, int indent);
};

class AtlasAsset;

class AtlasFrame : public Asset {
	friend class AtlasAsset;
protected:
	AtlasFrame(AtlasAsset* owner);
	AtlasFrame(const AtlasFrame&) = delete;
	AtlasFrame& operator=(const AtlasFrame&) = delete;

	// These are in normalized space
	ImVec2 p0; 
	ImVec2 p1;
	ImVec2 p2;
	ImVec2 p3;

	// Same as above, but rotated 90 degrees
	ImVec2 r0;
	ImVec2 r1;
	ImVec2 r2;
	ImVec2 r3;
public:
	void SetGuid(const std::string& guid);

	AtlasAsset* atlas; // The owning atlas
	bool rotated;

	virtual void EditorImmediateInspector(int* selectedFrame);
	void EditorImmediateInspector();

	ImVec2 GetSize();
	ImVec2 GetMin();
	ImVec2 GetMax();

	ImVec2 Uv0();
	ImVec2 Uv1();
	ImVec2 Uv2();
	ImVec2 Uv3();

	ImVec2 P0();
	ImVec2 P1();
	ImVec2 P2();
	ImVec2 P3();

	ImVec2 GetRawPoint(int index);
	void SetRawPoint(int index, const ImVec2& value);
protected:
	virtual void SerializeInner(std::stringstream& out, int indent);
};

class AtlasAsset : public Asset {
private:
	std::vector<AtlasFrame*> allFrames;
	std::vector<AtlasFrame*> frames;
protected:
	virtual void SerializeInner(std::stringstream& out, int indent);
public:
	ImVec2 sourceSize;
	void SetGuid(const std::string& guid);

	AtlasFrame* GetFrameByIndex(int index);

	AtlasAsset(const std::string& name);
	virtual ~AtlasAsset();

	int NumFrames();
	AtlasFrame* AddFrame(const std::string& name, const ImVec2& p0, const ImVec2& p1,
		const ImVec2& p2, const ImVec2& p3, bool rotated);
	virtual void EditorImmediateInspector(int* selectedFrame);
	void ImguiAssetsList(Asset* SelectedAsset, bool FocusNode, class AnimationAsset* SelectedAnimation, int SelectedFrame);
};

class AnimationAsset : public Asset {
	friend class AssetManager;
public:
	int frameRate;
	bool looping;
protected:
	int editorFrameToDelete;
	Track* editorTrackToDeleteFrom;
	SceneNode* editorGroupToDelete;
	std::vector<Track*> reoderTracks; // Editor only.

	int frameCount;
	std::map<SceneNode*, Track*> tracks;
	std::map<SceneNode*, bool> uiStateOpen;

	AnimationAsset(const std::string& _name, int numFrames, int fps, bool _looping);
	Track* _GetTrack(SceneNode* node, const std::string& name);
	virtual void SerializeInner(std::stringstream& out, int indent);
public:
	~AnimationAsset();
	int GetFrameCount();
	void SetFrameCount(int newCount);

	Track* AddTrack(SceneNode* node, const std::string& name, TrackType type);
	Track* GetTrack(SceneNode* node, const std::string& name);

	float GetPlaybackTime();

	bool Contains(SceneNode* node);
	bool Contains(SceneNode* node, const std::string& trackName);
	ImMat3 GetMatrix(SceneNode* node, int frame, ImMat3* outPos = 0, ImMat3* outRot = 0, ImMat3* outScl = 0);
	float GetRotation(SceneNode* node, int frame);
	void EditorImmediateInspector(int* selectedFrame);

	int ImguiDrawSequencer(int currentFrame);
};