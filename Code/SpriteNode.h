#pragma once

#include "TransformNode.h"
#include <string>

class SpriteNode : public TransformNode {
public:
	union {
		ImVec2 size;
		struct {
			float width;
			float height;
		};
	};

	union {
		ImVec4 tint;
		struct {
			float r;
			float g;
			float b;
			float a;
		};
	};

	class ImageAsset* image;
	class AtlasFrame* frame;
	ImVec2 frameMin;
	ImVec2 frameMax;

	bool visible;
	int sort;
	ImVec2 pivot;
protected:
	SpriteNode(const SpriteNode&) = delete;
	SpriteNode& operator=(const SpriteNode& other) = delete;
	virtual ~SpriteNode();
public:
	ImVec2 GetPivot(); // Does not return a reference, could be a temp var
	void SetPivot(const ImVec2& _pivot);

	SpriteNode(const char* name = 0);

	virtual bool CanHaveChildren();
	virtual void AddChild(SceneNode* newChild);
	virtual void AddChildAfter(SceneNode* prevOrNull, SceneNode* newChild);
	virtual void AddChildBefore(SceneNode* newChild, SceneNode* nextOrNull);
	virtual bool RemoveChild(SceneNode* child);
public:
	virtual void EditorImmediateInspector(class AnimationAsset* selectedAnimation, int selectedFrame);
protected:
	virtual void SerializeInner(std::stringstream& out, int indent);
};