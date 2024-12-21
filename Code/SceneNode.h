#pragma once

#include <string>
#include "Libraries/ImMath.h"
#include <sstream>

enum NodeType {
	SCENE_NODE = 0,
	TRANSFORM_NODE = 1,
	SPRITE_NODE = 2,
};

class SceneNode {
public:
	enum DragDropInsertMode {
		UNSET = 0,
		CHILD = 1,
		BEFORE = 2,
		AFTER = 3
	};
	struct DragAndDropData {
		SceneNode* source;
		SceneNode* target;
		DragDropInsertMode mode;
		void Reset();
	};
	static SceneNode* clicked;
private: // Static linked list of all nodes
	static SceneNode* trackingHead;
	SceneNode* trackingNext;
	std::string name;
	std::string nameInput;
protected:
	SceneNode* parent;
	SceneNode* firstChild;
	SceneNode* nextSibling;
public:
	NodeType rtti;
	bool isDeleted;
	std::string uuid;
protected:
	SceneNode(const SceneNode&) = delete;
	SceneNode& operator=(const SceneNode&) = delete;
	virtual ~SceneNode();
public:
	std::string& GetName();
	void SetName(const std::string& name);
public:
	static class SpriteNode* DeserializeSprite(const char* _name, const char* guid, const ImVec2& pos, float rot, const ImVec2& scl,
		const ImVec2& size, const ImVec2& pivot, const ImVec2& frameMin, const ImVec2& frameMax,
		const ImVec4& tint, int sort, bool visible, class ImageAsset* image, class AtlasFrame* frame);
	static class TransformNode* DeserializeTransform(const char* _name, const char* guid, const ImVec2& pos, float rot, const ImVec2& scl);

	SceneNode(const char* name = 0);

	SceneNode* GetParent();
	SceneNode* GetFirstChild();
	SceneNode* GetPrevSibling(); // Slow
	SceneNode* GetNextSibling();

	void ClearParent();
	void SetParent(SceneNode* newParent);
	bool CheckParent(SceneNode* newParent);

	// Override these in any leaf node
	virtual bool CanHaveChildren(); // Controls CheckParent early out
	virtual void AddChild(SceneNode* newChild);
	virtual void AddChildAfter(SceneNode* prevOrNull, SceneNode* newChild);
	virtual void AddChildBefore(SceneNode* newChild, SceneNode* nextOrNull);
	virtual bool RemoveChild(SceneNode* child);

	void DepthFirst(void(*callback)(SceneNode* node, void* userData), void* userData);
	static SceneNode* DepthFirst(SceneNode* iter, SceneNode& root);
public: // These actually belong to TransformNode
	virtual ImMat3 GetMatrix(ImMat3* outPos, ImMat3* outRot, ImMat3* outScl);
	virtual float GetRotation();
	virtual ImMat3 GetWorldMatrix(class Tool* tool = 0, class AnimationAsset* anim = 0, int frame = -1);
	virtual float GetWorldRotation(class Tool* tool = 0, class AnimationAsset* anim = 0, int frame = -1);
public:
	static void DestroyAll();
	bool EditorIsLeaf();
	SceneNode* EditorImguiHierarchy(SceneNode* selected, DragAndDropData* dragDropInfo/* = 0*/, ImGuiTextFilter* filter/* = 0*/, bool* outActivated /* = 0 */, bool showDragDrop);
	virtual void EditorImmediateInspector(class AnimationAsset* selectedAnimation, int selectedFrame);
	int EditorGetDepth();
	SceneNode* EditorFindRecursive(const char* guid);
	
	virtual void Serialize(std::stringstream& out, int indent);
protected:
	virtual void SerializeInner(std::stringstream& out, int indent);
	SceneNode* _EditorFindRecursive(const char* guid, SceneNode* root);
};