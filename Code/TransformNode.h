#pragma once

#include "SceneNode.h"
#include "Libraries/ImMath.h"

class TransformNode : public SceneNode {
public:
	ImVec2 position;
	float rotation;
	ImVec2 scale;
protected:
	TransformNode(const TransformNode&) = delete;
	TransformNode& operator=(const TransformNode&) = delete;
	virtual ~TransformNode();
public:
	TransformNode(const char* name = 0);
	virtual ImMat3 GetMatrix(ImMat3* outPos, ImMat3* outRot, ImMat3* outScl);
	virtual float GetRotation();

	static float NormalizedRotation(float rot);
	float NormalizeRotation();
	void Reset();
	void SetRotation(float val);

public:
	virtual void EditorImmediateInspector(class AnimationAsset* selectedAnimation, int selectedFrame);
protected:
	virtual void SerializeInner(std::stringstream& out, int indent);
};