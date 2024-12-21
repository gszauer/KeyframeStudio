#pragma once

#include "SceneNode.h"
#include "TransformNode.h"
#include "SpriteNode.h"
#include "AssetManager.h"
#include "Tools.h"
#include <sstream>
#include "Libraries/gabor_json.h"

enum class RotationDisplayType {
	RADIAN_SLIDER = 0,
	RADIAN_TEXT,
	ANGLE_TEXT
};

class Application {
protected:
	bool createNewSceneAfterFrame;
	bool saveSceneAs;
	bool openScene;

	bool resetDockSpace;

	bool showHierarchy;
	bool showAssets;
	bool showInspector;
	bool showSequencer;
	bool showHistory;

	bool leftClicked;
	bool middleClicked;
	bool rightClicked;

	Tool* activeTool;
	Tool* lastActiveTool;
	TranslateTool translateTool;
	RotateTool rotateTool;
	ScaleTool scaleTool;
	PanTool panTool;
	PivotTool pivotTool;

	SceneNode* rootNode;
	SceneNode* selectedNode;
	Asset* selectedAsset;
	bool focusNode; // If true, inspector target is selectedNode, otherwise its selectedAsset

	bool isPlaying;
	float playbackTime;

	AnimationAsset* selectedAnimation;
	int selectedFrame;

	ImFont* argon;
	ImFont* xenon;
	ImFont* icon;
	
	SceneNode::DragAndDropData dragAndDrop;
	
	std::vector<SpriteNode*> visibleSplires;

	bool guiLoadAtlasFromFile; // No need to give an initial value
	void* guiAtlasFileData;
	unsigned int guiAtlasFileSize;

	std::string guiAtlasTargetFileName;

	RotationDisplayType rotationDisplay;
	SceneNode* deleteAtTheEndOfFrame;

	float windowDevicePixelRatio;
	ImVec2 defaultFramePadding;
	ImVec2 defaultWindowPadding;
	ImGuiTextFilter filter; // Scene filter

	std::string newThingName;
	int newAnimFrameRate;
	int newAnimFrameCount;
	bool newAnimLooping;

	bool setDebugPan;
	bool firstUpdate;
public:
	inline void PushDefaultFramePadding() {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);
	}
	inline bool DoingDragDrop() {
		return dragAndDrop.source != 0;
	}
	inline float WindowDevicePixelRatio() {
		return windowDevicePixelRatio;
	}
	inline void PushXenon() {
		ImGui::PushFont(xenon);
	}
	inline void PushArgon() {
		ImGui::PushFont(argon);
	}
	inline void PushIcon() {
		ImGui::PushFont(icon);
	}
	void SetSelectedNode(SceneNode* node, bool focus);
	void SetSelectedAsset(Asset* node, bool focus, AnimationAsset* selectedAnim, int frame);
protected:
	static Application* instance;
	Application();
	~Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	void ResetDockSpace();
	void FillWithDebugData();

	void SkinImGui(float dpi);

	void ImguiHistory();
	void ImguiHierarchy();
	void ImguiInspector();
	void ImguiAssets();
	void ImguiScene();
	void ImguiSequencer();

	void _NewScene();
	void _SaveAs();
	void _OpenScene();

	void _DeserializeNode(SceneNode* parent, JsonValue* object);

	
public:
	void Initialize(float dpi);
	void Update();

	static Application* GetInstance();
	static void DestroyInstance();

	void Serialize(std::stringstream& output);
	void Deserialize(const char* json, int jsonLen);

	inline RotationDisplayType GetRotationDisplay() {
		return rotationDisplay;
	}
	inline void SetRotationDisplay(RotationDisplayType type) {
		rotationDisplay = type;
	}
	inline void DeleteAtEndOfFrame(SceneNode* nd) {
		IM_ASSERT(nd != 0);
		IM_ASSERT(deleteAtTheEndOfFrame == 0);
		deleteAtTheEndOfFrame = nd;
	}

	inline void _FileOperationStarted() { }
	inline void _FileOperationFinished() { }
};