#pragma once

#include "Action.h"
#include <string>

#define MAX_UNDO_STEPS 200

class OmniAction;

class UndoManager {
protected:
	static UndoManager* instance;
public:
	static UndoManager* GetInstance();
	static void DestroyAll();
	static bool IsActive;
protected:
	int cursor;
	int current; // tail = cursor - current

	// Memory block we placement new into!
	unsigned char* allActionsMemory; // TODO: Can probably delete this
	OmniAction* actions;

	UndoManager();
	~UndoManager();
	UndoManager(const UndoManager&) = delete;
	UndoManager* operator=(const UndoManager&) = delete;

	OmniAction* GetAction();
	Action* Tail();
public:
	bool CanUndo();
	bool CanRedo();

	void Undo();
	void Redo();

	void SetName(class SceneNode* target, const std::string& name);

	void SetPosition(class TransformNode* target, const ImVec2& pos, bool setX, bool setY);
	void SetRotation(class TransformNode* target, float rot);
	void SetScale(class TransformNode* target, const ImVec2& scl, bool setX, bool setY);

	void SetPivotTool(class SpriteNode* target, const ImVec2& pivot, const ImVec2& pos);

	void SetSize(class SpriteNode* target, const ImVec2& size, bool setX, bool setY);
	void SetPivot(class SpriteNode* target, const ImVec2& pivot, bool setX, bool setY);
	void SetSort(class SpriteNode* target, int sort);
	void SetVisibility(class SpriteNode* target, bool vis);
	void SetTint(class SpriteNode* target, ImU32 tnt);
	void SetFrameMin(class SpriteNode* target, const ImVec2& val, bool setX, bool setY);
	void SetFrameMax(class SpriteNode* target, const ImVec2& val, bool setX, bool setY);
	void SetSpriteRef(class SpriteNode* target, class ImageAsset* asset, const ImVec2& size, const ImVec2& frameMin, const ImVec2& frameMax);
	void SetFrameRef(class SpriteNode* target, class AtlasFrame* asset, const ImVec2& size, const ImVec2& frameMin, const ImVec2& frameMax);

	void AddFrameF(class Track* track, float data, int frame);
	void AddFrameI(class Track* track, int data, int frame);
	void AddFrameB(class Track* track, bool data, int frame);
	void AddFrameC(class Track* track, ImU32 data, int frame);

	void SelectNode(class SceneNode* oldSelection, class SceneNode* newSelection, bool oldFocus, bool newFocus);
	class SpriteNode* CreateSpriteNode(SceneNode* parent);
	class TransformNode* CreateTransformNode(SceneNode* parent);
	void DeleteNode(SceneNode* node);
	void ReparentNode(SceneNode* node, SceneNode* parent, SceneNode* nextSib);

	void SelectAsset(class Asset* oldAss, class Asset* newAss, bool oldFocus, bool newFocus, class AnimationAsset* oldAnim, class AnimationAsset* newAnim, int oldFrame, int newFrame);
	void NewAnimation(const std::string& name, int frameRate, int frmaeCount, bool looping);
	void NewImage(); // Select File
	class AtlasAsset* NewAtlas(const char* name);
	AtlasAsset* NewAtlasFromMemory(const char* name, const char* fileName, unsigned char* data, unsigned int size);
	void DeleteAsset(Asset* ass);

	void SetName(class AnimationAsset* target, const std::string& name);
	void SetFrameRate(class AnimationAsset* target, int fps);
	void SetFrameCount(class AnimationAsset* target, int count);
	void SetLooping(class AnimationAsset* target, bool loop);

	void SetName(class ImageAsset* target, const std::string& name);
	void SetName(class AtlasAsset* target, const std::string& name);
	void SetSize(class AtlasAsset* target, const ImVec2& size);
	void AddFrame(class AtlasAsset* target);

	void SetName(class AtlasFrame* target, const std::string& newName);
	void SetPoint(class AtlasFrame* target, int index, const ImVec2& point);
	void SetRotated(class AtlasFrame* target, bool val);

	void DeleteFrame(class Track* track, int index);
	void DeleteTrack(class Track* track);
	void DeleteTracks(class Track* head);

	void Imgui();
};