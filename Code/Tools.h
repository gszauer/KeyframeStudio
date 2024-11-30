#pragma once

#include "Libraries/imgui.h"
#include "TransformNode.h"

enum class ToolMouseButton {
	LEFT = 0,
	MIDDLE = 1,
	RIGHT = 2
};

enum class ToolAxisConstraint {
	NONE = 0,
	X = 1,
	Y = 2
};

enum class ToolNodeType {
	BASE = 0,
	TRANSLATE,
	ROTATE,
	SCALE,
	PAN,
	PIVOT
};

class Tool {
public:
	SceneNode* target;	// Set every frame
	class AnimationAsset* lastDrawnAsset; // TODO: Rename to something else
	int lastFrame; // TODO: Rename to something else

	bool isGlobal;
	bool isActive;		// True if the move tool is moving for example
	ToolNodeType rtti;	// who am i?
	ToolAxisConstraint constraint; // None, x or y
	ImVec2 constraintAxis;

	static float toolZoom; // Zoom of pan tool for display port
	bool scaled;
protected:
	Tool(const Tool&) = delete;
	Tool& operator=(const Tool&) = delete;
public:
	Tool();
	virtual ~Tool();

	virtual void ImGuiEditorBar();

	virtual void OnMouseDown(float x, float y, ToolMouseButton button);
	virtual void OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button);
	virtual void OnMouseUp(ToolMouseButton button);

	virtual void OnScroll(float wheelDelta);

	virtual void Draw(ImDrawList* list, ImVec2 offset, float zoom, class AnimationAsset* anim = 0, int frame = -1);
};

class TranslateTool : public Tool {
protected:
	float arrowLength;
	float arrowWidth;
	float arrowHeight;
	float rectWidth;
	float omniBoxSize;
public:
	bool snapEnabled;
	int snapValue;
	
	// Actual tool state
	ImVec2 mouseStart;
	ImVec2 objectStart;
	ImVec2 mouseCurrent;
	ImVec2 objectCurrent;

	// These are set in ::Draw, but read back in ::MouseDown for next frame.
	ImVec2 lastUpArrow[3];
	ImVec2 lastRightArrow[3];
	ImVec2 lastUpSquare[4];
	ImVec2 lastRightSquare[4];
	ImVec2 lastOmniSquare[4];
protected:
	TranslateTool(const TranslateTool&) = delete;
	TranslateTool& operator=(const TranslateTool&) = delete;
public:
	TranslateTool();
	~TranslateTool();

	void OnMouseDown(float x, float y, ToolMouseButton ToolMouseButton);
	void OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton ToolMouseButton);
	void OnMouseUp(ToolMouseButton ToolMouseButton);

	void ImGuiEditorBar();
	void Draw(ImDrawList* list, ImVec2 offset, float zoom, class AnimationAsset* anim = 0, int frame = -1);
};

class RotateTool : public Tool {
	float radius;
	float thickness;
	float arrowWidth;
	float arrowHeight;
protected:
	RotateTool(const RotateTool&) = delete;
	RotateTool& operator=(const RotateTool&) = delete;
public:
	RotateTool();
	~RotateTool();

	bool snapEnabled;
	int snapValue;

	float rotateStart;
	float rotateCurrent;

	ImVec2 mouseStart;
	ImVec2 mouseCurrent;

	ImVec2 lastOffset;
	float lastZoom;

	void ImGuiEditorBar();
	void OnMouseDown(float x, float y, ToolMouseButton button);
	void OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button);
	void OnMouseUp(ToolMouseButton button);
	void Draw(ImDrawList* list, ImVec2 offset, float zoom, class AnimationAsset* anim = 0, int frame = -1);
};

class ScaleTool : public Tool {
	float handleSize;
	float handleWidth;
	float nibSize;
	float omniSize;
public:
	bool snapEnabled;
	int snapValue;
	float sensitivity; // Default is 64

	ImVec2 mouseStart;
	ImVec2 mouseCurrent;
	float extraLength;
	ImVec2 startScale;  // TransformNode.scale
	float currentScale;// add to startScale. Local axis only

	ImVec2 lastUpHandle[4];
	ImVec2 lastUpNib[4];
	ImVec2 lastRightHandle[4];
	ImVec2 lastRightNib[4];
	ImVec2 lastOmni[4];
protected:
	ScaleTool(const ScaleTool&) = delete;
	ScaleTool& operator=(const ScaleTool&) = delete;
public:
	ScaleTool();
	~ScaleTool();

	void ImGuiEditorBar();

	void OnMouseDown(float x, float y, ToolMouseButton button);
	void OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button);
	void OnMouseUp(ToolMouseButton button);
	void Draw(ImDrawList* list, ImVec2 offset, float zoom, class AnimationAsset* anim = 0, int frame = -1);
};

class PanTool : public Tool {
protected:
	float minZoom;
	float maxZoom;
public:
	ImVec2 pan;
	float zoom;
	float gridSize;
	bool gridVisible;
protected:
	PanTool(const PanTool&) = delete;
	PanTool& operator=(const PanTool&) = delete;
public:
	PanTool();
	~PanTool();

	void ImGuiEditorBar();

	void OnMouseDown(float x, float y, ToolMouseButton button);
	void OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button);
	void OnMouseUp(ToolMouseButton button);
	void OnScroll(float wheelDelta);
};

class PivotTool : public Tool {
public:
	float radius;
	float thickness;
public:
	// Actual tool state
	ImVec2 mouseStart;
	ImVec2 mouseCurrent;
	ImVec2 objectStartPos;
	ImVec2 objectCurrentPos;
	ImVec2 objectStartPivot;
	ImVec2 objectCurrentPivot;

	ImVec2 lastOffset;
	float lastZoom;
	class SpriteNode* outline;
	bool allowDrag;
protected:
	PivotTool(const PivotTool&) = delete;
	PivotTool& operator=(const PivotTool&) = delete;
public:
	PivotTool();
	~PivotTool();

	void ImGuiEditorBar();
	void OnMouseDown(float x, float y, ToolMouseButton button);
	void OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button);
	void OnMouseUp(ToolMouseButton button);
	void Draw(ImDrawList* list, ImVec2 offset, float zoom, class AnimationAsset* anim = 0, int frame = -1);
};