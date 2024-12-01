#include "UndoManager.h"
#include "SceneNode.h"
#include "TransformNode.h"
#include "SpriteNode.h"
#include "Action.h"
#include "Libraries/imgui.h"
#include "AssetManager.h"
#include "Track.h"
#include "Application.h"
#include "Platform.h"

#include <iostream>


UndoManager* UndoManager::instance = 0;
bool UndoManager::IsActive = true;

extern "C" const char* UI_SelectFile();

UndoManager* UndoManager::GetInstance() {
	if (instance == 0) {
		instance = new UndoManager();
	}
	return instance;
}

void UndoManager::DestroyAll() {
	delete instance;
	instance = 0;
}

UndoManager::UndoManager() {
	allActionsMemory = (unsigned char*)malloc(sizeof(OmniAction) * (MAX_UNDO_STEPS));
	memset(allActionsMemory, 0, sizeof(OmniAction) * (MAX_UNDO_STEPS));
	actions = (OmniAction*)allActionsMemory;

	for (int i = 0; i < MAX_UNDO_STEPS; ++i) {
		OmniAction* action = &actions[i];
		new (action) InvalidAction();
	}

	cursor = -1;
	current = 0;
}

UndoManager::~UndoManager() {
	for (int i = 0; i < MAX_UNDO_STEPS; ++i) {
		InvokeDestructor(&actions[i]);
	}
	::free(allActionsMemory);
	allActionsMemory = 0;
	actions = 0;
}

OmniAction* UndoManager::GetAction() {
	if (!UndoManager::IsActive) {
		OmniAction* result = &actions[0];
		InvokeDestructor(result); 
		memset(result, 0, sizeof(OmniAction));
		return result;
	}
	
	if (current > 0) {
		int startIndex = (cursor - (current - 1)) % MAX_UNDO_STEPS;
		while (startIndex < 0) { startIndex += MAX_UNDO_STEPS; }
		
		for (int i = 0; i < MAX_UNDO_STEPS; ++i) {
			int idx = (startIndex + i) % MAX_UNDO_STEPS;

			InvokeDestructor(&actions[idx]);
			memset(&actions[idx], 0, sizeof(OmniAction));
			new (&actions[idx]) InvalidAction();

			if (idx == cursor) {
				break;
			}
		}


		cursor = (cursor - current) % MAX_UNDO_STEPS;
		while (cursor < 0) { cursor += MAX_UNDO_STEPS; }
		current = 0;
	}

	cursor = (cursor + 1) % MAX_UNDO_STEPS;
	OmniAction* result = &actions[cursor];
	InvokeDestructor(result); // Assume all actions are valid at this point
	memset(result, 0, sizeof(OmniAction));
	return result;
}

Action* UndoManager::Tail() {
	int idx = (cursor - current) % MAX_UNDO_STEPS;
	while (idx < 0) { idx += MAX_UNDO_STEPS; }
	return &actions[idx].asAction;
}

void UndoManager::Imgui() {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	
	ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
	canvas_sz.y -= 28.0f * windowDevicePixelRatio;

	if (!UndoManager::IsActive) {
		ImGui::BeginDisabled();
	}

	Action* setCurrent = 0; // if the undo item was clicked
	int currentIndex = (cursor - current) % MAX_UNDO_STEPS;
	while (currentIndex < 0) currentIndex += MAX_UNDO_STEPS;

	bool sawSelected = false;
	bool isOnLeft = true;

	bool isDisabled = false;

	ImGui::BeginChild("##History", canvas_sz, 0, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	for (int i = MAX_UNDO_STEPS - 1; i >= 0; --i) {
		int idx = (cursor - i) % MAX_UNDO_STEPS;
		while (idx < 0) idx += MAX_UNDO_STEPS;

		Action* iter = &actions[idx].asAction;
		if (iter->rtti == ActionType::InvalidAction) {
			continue;
		}

		if (idx == currentIndex) {
			sawSelected = true;
		}

		if (!isDisabled && current != 0 && i < current) {
			//ImGui::BeginDisabled();
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // Gray text color
			isDisabled = true;
		}

		ImGui::PushID(iter);
		if (ImGui::Selectable(iter->GetName(), idx == currentIndex)) {
			if (currentIndex != idx) {
				IM_ASSERT(setCurrent == 0);
				setCurrent = iter;

				if (sawSelected) {
					isOnLeft = false;
				}
			}
		}
		ImGui::PopID();
	}
	if (isDisabled) {
		//ImGui::EndDisabled();
		ImGui::PopStyleColor();
	}
	ImGui::EndChild();

	if (!UndoManager::IsActive) {
		ImGui::EndDisabled();
	}


	ImGui::SetCursorPosX(canvas_sz.x - 195.0f * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f * windowDevicePixelRatio);
	ImGui::Text("Enable Undo History");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f * windowDevicePixelRatio);
	bool undoState = UndoManager::IsActive;
	if (ImGui::Checkbox("##EnableUndoHistory", &undoState)) {
		if (undoState != UndoManager::IsActive) {
			UndoManager::IsActive = undoState;
			UndoManager::DestroyAll();
			return; // Undo Manager Object No longer exists
		}
	}

	if (setCurrent != 0) {
		if (isOnLeft) {
			int startIndex = (cursor - current) % MAX_UNDO_STEPS;
			while (startIndex < 0) startIndex += MAX_UNDO_STEPS;

			for (int i = 0; i < MAX_UNDO_STEPS; ++i) {
				int idx = (startIndex - i) % MAX_UNDO_STEPS;
				while (idx < 0) idx += MAX_UNDO_STEPS;

				Action* iter = &actions[idx].asAction;
				if (iter == setCurrent) {
					break;
				}
				iter->Undo();
				//std::cout << "Undo " << iter->GetName();
			}
		}
		else {
			int startIndex = (cursor - current) % MAX_UNDO_STEPS;
			while (startIndex < 0) startIndex += MAX_UNDO_STEPS;
			Action* cursorAction = &actions[cursor].asAction;

			for (int i = 0; i < MAX_UNDO_STEPS; ++i) {
				int idx = (startIndex + i) % MAX_UNDO_STEPS;

				Action* iter = &actions[idx].asAction;
				iter->Do();
				if (iter == cursorAction || iter == setCurrent) {
					break;
				}
			}
		}
	}

	if (setCurrent != 0) {
		current = 0;
		for (int i = 0; i < MAX_UNDO_STEPS; ++i) {
			int idx = (cursor - i) % MAX_UNDO_STEPS;
			while (idx < 0) idx += MAX_UNDO_STEPS;

			if (&actions[idx].asAction == setCurrent) {
				break;
			}
			current += 1;
		}
	}
}

bool UndoManager::CanUndo() {
	// Too many steps
	if (current >= MAX_UNDO_STEPS - 1) {
		return false;
	}

	// Am i valid?
	int idx = (cursor - current) % MAX_UNDO_STEPS;
	while (idx < 0) { idx += MAX_UNDO_STEPS; }
	if (actions[idx].asAction.rtti == ActionType::InvalidAction) {
		return false;
	}

	// Check if stepping back would be invalid
	idx = (cursor - (current + 1)) % MAX_UNDO_STEPS;
	while (idx < 0) { idx += MAX_UNDO_STEPS; }
	if (actions[idx].asAction.rtti == ActionType::InvalidAction) {
		return false;
	}

	return true; 
}

bool UndoManager::CanRedo() {
	return current > 0;
}

void UndoManager::Undo() {
	if (!CanUndo()) {
		return;
	}

	int idx = (cursor - current) % MAX_UNDO_STEPS;
	while (idx < 0) { idx += MAX_UNDO_STEPS; }

	if (actions[idx].asAction.rtti != ActionType::InvalidAction) {
		actions[idx].asAction.Undo();
	}

	current += 1;
	if (current >= MAX_UNDO_STEPS) {
		current = MAX_UNDO_STEPS - 1;
	}
}

void UndoManager::Redo() {
	if (!CanRedo()) {
		return;
	}
	
	current -= 1;
	if (current < 0) {
		current = 0;
	}

	int idx = (cursor - current) % MAX_UNDO_STEPS;
	while (idx < 0) { idx += MAX_UNDO_STEPS; }

	if (actions[idx].asAction.rtti != ActionType::InvalidAction) {
		actions[idx].asAction.Do();
	}
}

void UndoManager::SetName(SceneNode* target, const std::string& name) {
	// Don't make frivolous copies
	if (Tail()->rtti == ActionType::SetNameAction) {
		SetNameAction* _tail = (SetNameAction*)Tail();
		if (_tail->target == target) {
			_tail->newName = name;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetNameAction* action = new(o) SetNameAction(target, name);
}

void UndoManager::SetPosition(class TransformNode* target, const ImVec2& pos, bool setX, bool setY) {
	if (Tail()->rtti == ActionType::SetPositionAction) {
		SetPositionAction* _tail = (SetPositionAction*)Tail();
		if (_tail->target == target && _tail->setX == setX && _tail->setY == setY) {
			_tail->newPosition = pos;
			_tail->Do();
			return;
		}
	}
	
	OmniAction* o = GetAction();
	SetPositionAction* action = new(o) SetPositionAction(target, pos, setX, setY);
}

void UndoManager::SetScale(class TransformNode* target, const ImVec2& scl, bool setX, bool setY) {
	if (Tail()->rtti == ActionType::SetScaleAction) {
		SetScaleAction* _tail = (SetScaleAction*)Tail();
		if (_tail->target == target && _tail->setX == setX && _tail->setY == setY) {
			_tail->newScale = scl;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetScaleAction* action = new(o) SetScaleAction(target, scl, setX, setY);
}

void UndoManager::SetSort(class SpriteNode* target, int sort) {
	if (Tail()->rtti == ActionType::SetSortAction) {
		SetSortAction* _tail = (SetSortAction*)Tail();
		if (_tail->target == target) {
			_tail->newSort = sort;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetSortAction* action = new(o) SetSortAction(target, sort);
}

void UndoManager::SetPivotTool(class SpriteNode* target, const ImVec2& pivot, const ImVec2& pos) {
	/*if (Tail()->rtti == ActionType::SetPivotToolAction) {
		SetPivotToolAction* _tail = (SetPivotToolAction*)Tail();
		if (_tail->target == target) {
			_tail->newPivot = pivot;
			_tail->newPos = pos;
			_tail->Do();
			return;
		}
	}*/

	OmniAction* o = GetAction();
	SetPivotToolAction* action = new(o) SetPivotToolAction(target, pivot, pos);
}

void UndoManager::SetSize(class SpriteNode* target, const ImVec2& size, bool setX, bool setY) {
	if (Tail()->rtti == ActionType::SetSpriteSizeAction) {
		SetSpriteSizeAction* _tail = (SetSpriteSizeAction*)Tail();
		if (_tail->target == target && _tail->setX == setX && _tail->setY == setY) {
			_tail->newSize = size;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetSpriteSizeAction* action = new(o) SetSpriteSizeAction(target, size, setX, setY);
}

void UndoManager::SetSpriteRef(class SpriteNode* target, class ImageAsset* asset, const ImVec2& size, const ImVec2& frameMin, const ImVec2& frameMax) {
	// No neede to edit the last one
	
	OmniAction* o = GetAction();
	SetSpriteRefAction* action = new(o) SetSpriteRefAction(target, asset, size, frameMin, frameMax);
}

void UndoManager::SetFrameRef(class SpriteNode* target, class AtlasFrame* asset, const ImVec2& size, const ImVec2& frameMin, const ImVec2& frameMax) {
	// No need to edit the last one
	
	OmniAction* o = GetAction();
	SetFrameRefAction* action = new(o) SetFrameRefAction(target, asset, size, frameMin, frameMax);
}

void UndoManager::SetFrameMin(class SpriteNode* target, const ImVec2& val, bool setX, bool setY) {
	if (Tail()->rtti == ActionType::SetFrameMinAction) {
		SetFrameMinAction* _tail = (SetFrameMinAction*)Tail();
		if (_tail->target == target && _tail->setX == setX && _tail->setY == setY) {
			_tail->newValue = val;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetFrameMinAction* action = new(o) SetFrameMinAction(target, val, setX, setY);
}

void UndoManager::SetFrameMax(class SpriteNode* target, const ImVec2& val, bool setX, bool setY) {
	if (Tail()->rtti == ActionType::SetFrameMaxAction) {
		SetFrameMaxAction* _tail = (SetFrameMaxAction*)Tail();
		if (_tail->target == target && _tail->setX == setX && _tail->setY == setY) {
			_tail->newValue = val;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetFrameMaxAction* action = new(o) SetFrameMaxAction(target, val, setX, setY);
}

void UndoManager::SetPivot(class SpriteNode* target, const ImVec2& pivot, bool setX, bool setY) {
	if (Tail()->rtti == ActionType::SetSpritePivotAction) {
		SetSpritePivotAction* _tail = (SetSpritePivotAction*)Tail();
		if (_tail->target == target && _tail->setX == setX && _tail->setY == setY) {
			_tail->newPivot = pivot;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetSpritePivotAction* action = new(o) SetSpritePivotAction(target, pivot, setX, setY);
}

void UndoManager::SetVisibility(class SpriteNode* target, bool vis) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	SetVisibleAction* action = new(o) SetVisibleAction(target, vis);
}

void UndoManager::SetTint(class SpriteNode* target, ImU32 tnt) {
	if (Tail()->rtti == ActionType::SetTintAction) {
		SetTintAction* _tail = (SetTintAction*)Tail();
		if (_tail->target == target) {
			_tail->newTint = tnt;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetTintAction* action = new(o) SetTintAction(target, tnt);
}

void UndoManager::SetRotation(class TransformNode* target, float rot) {
	if (Tail()->rtti == ActionType::SetRotationAction) {
		SetRotationAction* _tail = (SetRotationAction*)Tail();
		if (_tail->target == target) {
			_tail->newRotation = rot;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetRotationAction* action = new(o) SetRotationAction(target, rot);
}

void UndoManager::AddFrameF(Track* track, float data, int frame) {
	if (Tail()->rtti == ActionType::AddFrameFAction) {
		AddFrameFAction* _tail = (AddFrameFAction*)Tail();
		if (_tail->target == track && _tail->frame == frame && _tail->existing != 0) {
			_tail->newData = data;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	AddFrameFAction* action = new(o) AddFrameFAction(track, data, frame);
}

void UndoManager::AddFrameI(class Track* track, int data, int frame) {
	if (Tail()->rtti == ActionType::AddFrameIAction) {
		AddFrameIAction* _tail = (AddFrameIAction*)Tail();
		if (_tail->target == track && _tail->frame == frame && _tail->existing != 0) {
			_tail->newData = data;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	AddFrameIAction* action = new(o) AddFrameIAction(track, data, frame);
}

void UndoManager::AddFrameB(class Track* track, bool data, int frame) {
	if (Tail()->rtti == ActionType::AddFrameBAction) {
		AddFrameBAction* _tail = (AddFrameBAction*)Tail();
		if (_tail->target == track && _tail->frame == frame && _tail->existing != 0) {
			_tail->newData = data;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	AddFrameBAction* action = new(o) AddFrameBAction(track, data, frame);
}

void  UndoManager::AddFrameC(class Track* track, ImU32 data, int frame) {
	if (Tail()->rtti == ActionType::AddFrameCAction) {
		AddFrameCAction* _tail = (AddFrameCAction*)Tail();
		if (_tail->target == track && _tail->frame == frame && _tail->existing != 0) {
			_tail->newTint = data;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	AddFrameCAction* action = new(o) AddFrameCAction(track, data, frame);
}

void  UndoManager::SelectNode(class SceneNode* oldSelection, class SceneNode* newSelection, bool oldFocus, bool newFocus) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	HierarchySelectionAction* action = new(o) HierarchySelectionAction(oldSelection, newSelection, oldFocus, newFocus);
}

TransformNode* UndoManager::CreateTransformNode(SceneNode* parent) {
	// No need to edit the last one
	TransformNode* result = new TransformNode("Transform Node");
	
	OmniAction* o = GetAction();
	NewTransformNodeAction* action = new(o) NewTransformNodeAction(result, parent);
	return result;
}

SpriteNode* UndoManager::CreateSpriteNode(SceneNode* parent) {
	// No need to edit the last one
	SpriteNode* result = new SpriteNode("Sprite Node");
	
	OmniAction* o = GetAction();
	NewSpriteNodeAction* action = new(o) NewSpriteNodeAction(result, parent);
	return result;
}

void UndoManager::DeleteNode(SceneNode* node) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	DeleteNodeAction* action = new(o) DeleteNodeAction(node);
}

void UndoManager::ReparentNode(SceneNode* node, SceneNode* parent, SceneNode* nextSib) {
	// No need to edit the last one
	bool isValid = parent != 0 && parent->CanHaveChildren();

	if (isValid) {
		OmniAction* o = GetAction();
		ReparentNodeAction* action = new(o) ReparentNodeAction(node, parent, nextSib);
	}
}

void UndoManager::SelectAsset(class Asset* oldAss, class Asset* newAss, bool oldFocus, bool newFocus, class AnimationAsset* oldAnim, class AnimationAsset* newAnim, int oldFrame, int newFrame) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	AssetSelectionAction* action = new(o) AssetSelectionAction(oldAss, newAss, oldFocus, newFocus, oldAnim, newAnim, oldFrame, newFrame);
}

void UndoManager::NewImage() {
	PlatformSelectFile("png\0*.png\0All\0*.*\0\0",
		[](const char* selectedFile, unsigned char* data, unsigned int size) {
			//const char* selectedFile = UI_SelectFile();
			if (selectedFile != 0 && selectedFile[0] != '\0' && data != 0 && size != 0) {
				ImageAsset* result = AssetManager::GetInstance()->LoadImageFromMemory(selectedFile, data, size);
				OmniAction* o = UndoManager::GetInstance()->GetAction();
				NewImageAction* action = new(o) NewImageAction(result);
			}
		}
	);
}

AtlasAsset* UndoManager::NewAtlasFromMemory(const char* name, const char* fileName, unsigned char* data, unsigned int size) {
	AtlasAsset* newAtlas = AssetManager::GetInstance()->LoadAtlasFromMemory(name, fileName, data, size);
	OmniAction* o = UndoManager::GetInstance()->GetAction();
	NewAtlasAction* action = new(o) NewAtlasAction(newAtlas);
	return newAtlas;
}

#if 0
void UndoManager::NewAtlasFromFile(const char* name, const char* file) {
	// No need to edit the last one
	AssetManager::GetInstance()->LoadAtlasFromFile(name, file,
		[](const char* fileName, Asset* resultAsset) {
			if (resultAsset != 0) {
				AtlasAsset* newAtlas = (AtlasAsset*)resultAsset;
				OmniAction* o = UndoManager::GetInstance()->GetAction();
				NewAtlasAction* action = new(o) NewAtlasAction(newAtlas);
			}
		}
	);
}
#endif

class AtlasAsset* UndoManager::NewAtlas(const char* name) {
	// No need to edit the last one
	AtlasAsset* newAtlas = AssetManager::GetInstance()->NewAtlas(name);
	if (newAtlas != 0) {
		OmniAction* o = GetAction();
		NewAtlasAction* action = new(o) NewAtlasAction(newAtlas);
	}
	return newAtlas;
}

void UndoManager::NewAnimation(const std::string& name, int frameRate, int frmaeCount, bool looping) {
	// No need to edit the last one
	AnimationAsset* createdAnim = AssetManager::GetInstance()->NewAnimation(name, frameRate, frmaeCount, looping);
	OmniAction* o = GetAction();
	NewAnimationAction* action = new(o) NewAnimationAction(createdAnim);
}

void UndoManager::DeleteAsset(Asset* ass) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	DeleteAssetAction* action = new(o) DeleteAssetAction(ass);
}

void UndoManager::SetName(class AtlasAsset* target, const std::string& name) {
	if (Tail()->rtti == ActionType::RenameAtlasAction) {
		RenameAtlasAction* _tail = (RenameAtlasAction*)Tail();
		if (_tail->target == target) {
			_tail->newName = name;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	RenameAtlasAction* action = new(o) RenameAtlasAction(target, name);
}

void UndoManager::AddFrame(class AtlasAsset* target) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	AddAtlasFrameAction* action = new(o) AddAtlasFrameAction(target);
}

void UndoManager::SetSize(class AtlasAsset* target, const ImVec2& size) {
	if (Tail()->rtti == ActionType::SetAtlasSizeAction) {
		SetAtlasSizeAction* _tail = (SetAtlasSizeAction*)Tail();
		if (_tail->target == target) {
			_tail->newSize = size;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetAtlasSizeAction* action = new(o) SetAtlasSizeAction(target, size);
}

void UndoManager::SetName(class ImageAsset* target, const std::string& name) {
	if (Tail()->rtti == ActionType::RenameImageAction) {
		RenameImageAction* _tail = (RenameImageAction*)Tail();
		if (_tail->target == target) {
			_tail->newName = name;
			_tail->Do();
			return;
		}
	}


	OmniAction* o = GetAction();
	RenameImageAction* action = new(o) RenameImageAction(target, name);
}

void UndoManager::SetName(class AnimationAsset* target, const std::string& name) {
	if (Tail()->rtti == ActionType::RenameAnimationAction) {
		RenameAnimationAction* _tail = (RenameAnimationAction*)Tail();
		if (_tail->target == target) {
			_tail->newName = name;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	RenameAnimationAction* action = new(o) RenameAnimationAction(target, name);
}

void UndoManager::SetFrameRate(class AnimationAsset* target, int fps) {
	if (Tail()->rtti == ActionType::SetFrameRateAction) {
		SetFrameRateAction* _tail = (SetFrameRateAction*)Tail();
		if (_tail->target == target) {
			_tail->newFPS = fps;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetFrameRateAction* action = new(o) SetFrameRateAction(target, fps);
}

void UndoManager::SetFrameCount(class AnimationAsset* target, int count) {
	if (Tail()->rtti == ActionType::SetFrameCountAction) {
		SetFrameCountAction* _tail = (SetFrameCountAction*)Tail();
		if (_tail->target == target) {
			_tail->newCount = count;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	SetFrameCountAction* action = new(o) SetFrameCountAction(target, count);
}

void UndoManager::SetLooping(class AnimationAsset* target, bool loop) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	SetLoopingAction* action = new(o) SetLoopingAction(target, loop);
}

void UndoManager::SetName(class AtlasFrame* target, const std::string& newName) {
	if (Tail()->rtti == ActionType::RenameAtlasFrameAction) {
		RenameAtlasFrameAction* _tail = (RenameAtlasFrameAction*)Tail();
		if (_tail->target == target) {
			_tail->newName = newName;
			_tail->Do();
			return;
		}
	}
	
	OmniAction* o = GetAction();
	RenameAtlasFrameAction* action = new(o) RenameAtlasFrameAction(target, newName);
}

void UndoManager::SetPoint(class AtlasFrame* target, int index, const ImVec2& point) {
	if (Tail()->rtti == ActionType::AtlasFrameSetPointAction) {
		AtlasFrameSetPointAction* _tail = (AtlasFrameSetPointAction*)Tail();
		if (_tail->target == target && _tail->index == index) {
			_tail->newPoint = point;
			_tail->Do();
			return;
		}
	}

	OmniAction* o = GetAction();
	AtlasFrameSetPointAction* action = new(o) AtlasFrameSetPointAction(target, point, index);
}

void UndoManager::SetRotated(class AtlasFrame* target, bool val) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	AtlasFrameSetRotatedAction* action = new(o) AtlasFrameSetRotatedAction(target, val);
}

void UndoManager::DeleteFrame(class Track* track, int index) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	DeleteAnimFrameAction* action = new(o) DeleteAnimFrameAction(track, index);
}

void UndoManager::DeleteTrack(class Track* track) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	DeleteTrackAction* action = new(o) DeleteTrackAction(track);
}

void UndoManager::DeleteTracks(class Track* head) {
	// No need to edit the last one
	OmniAction* o = GetAction();
	DeleteTrackGroupAction* action = new(o) DeleteTrackGroupAction(head);
}