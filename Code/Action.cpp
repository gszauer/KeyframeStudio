#include "Action.h"
#include "SceneNode.h"
#include "TransformNode.h"
#include "SpriteNode.h"
#include "Track.h"
#include "Application.h"
#include "Asset.h"
#include "Platform.h"
#include "UndoManager.h"

#include "Libraries/imgui.h"

Action::Action() { 
	rtti = ActionType::Action;
}

Action::~Action() { }
void Action::Do() { IM_ASSERT(false); }
void Action::Undo() { IM_ASSERT(false); }

FirstAction::FirstAction() { rtti = ActionType::FirstAction; }
void FirstAction::Do() { }
void FirstAction::Undo() { }

SetNameAction::SetNameAction(SceneNode* target, const std::string& name) {
	rtti = ActionType::SetNameAction;
	this->target = target;
	this->oldName = target->GetName();
	this->newName = name;
	Do();
}

void SetNameAction::Do() {
	target->SetName(newName);
}

void SetNameAction::Undo() {
	target->SetName(oldName);
}

SetTintAction::SetTintAction(class SpriteNode* _target, ImU32 _tint) {
	rtti = ActionType::SetTintAction;
	target = _target;
	oldTint = ImGui::ColorConvertFloat4ToU32(target->tint);
	newTint = _tint;
	Do();
}
void SetTintAction::Do() {
	target->tint = ImGui::ColorConvertU32ToFloat4(newTint);
}

void SetTintAction::Undo() {
	target->tint = ImGui::ColorConvertU32ToFloat4(oldTint);
}

SetVisibleAction::SetVisibleAction(class SpriteNode* _target, bool vis) {
	rtti = ActionType::SetVisibleAction;
	target = _target;
	oldVisible = target->visible;
	newVisible = vis;
	Do();
}

void SetVisibleAction::Do() {
	target->visible = newVisible;
}

void SetVisibleAction::Undo() {
	target->visible = oldVisible;
}

SetSortAction::SetSortAction(class SpriteNode* _target, int sort) {
	rtti = ActionType::SetSortAction;
	target = _target;
	oldSort = target->sort;
	newSort = sort;
	Do();
}
void SetSortAction::Do() {
	target->sort = newSort;
}

void SetSortAction::Undo() {
	target->sort = oldSort;
}

SetRotationAction::SetRotationAction(TransformNode* _target, float newRot) {
	rtti = ActionType::SetRotationAction;
	IM_ASSERT(_target != 0);
	target = _target;
	oldRotation = target->rotation;
	newRotation = newRot;
	Do();
}

void SetRotationAction::Do() {
	target->rotation = newRotation;
	target->NormalizeRotation();
}

void SetRotationAction::Undo() {
	target->rotation = oldRotation;
	target->NormalizeRotation();
}

SetScaleAction::SetScaleAction(TransformNode* _target, const ImVec2& _newScale, bool _setX, bool _setY) {
	rtti = ActionType::SetScaleAction;
	IM_ASSERT(_target != 0);
	target = _target;
	setX = _setX;
	setY = _setY;
	newScale = _newScale;
	oldScale = target->scale;

	Do();
}
void SetScaleAction::Do() {
	if (setX) {
		target->scale.x = newScale.x;
	}
	if (setY) {
		target->scale.y = newScale.y;
	}
}
void SetScaleAction::Undo() {
	if (setX) {
		target->scale.x = oldScale.x;
	}
	if (setY) {
		target->scale.y = oldScale.y;
	}
}

SetPositionAction::SetPositionAction(TransformNode* _target, const ImVec2& _newPos, bool _setX, bool _setY) {
	rtti = ActionType::SetPositionAction;
	IM_ASSERT(_target != 0);
	target = _target;
	setX = _setX;
	setY = _setY;
	newPosition = _newPos;
	oldPosition = target->position;

	Do();
}

void SetPositionAction::Do() {
	if (setX) {
		target->position.x = newPosition.x;
	}
	if (setY) {
		target->position.y = newPosition.y;
	}
}

void SetPositionAction::Undo() {
	if (setX) {
		target->position.x = oldPosition.x;
	}
	if (setY) {
		target->position.y = oldPosition.y;
	}
}

AddFrameIAction::AddFrameIAction(class Track* _track, int _data, int _frame) {
	rtti = ActionType::AddFrameIAction;
	target = _track;
	newData = _data;
	frame = _frame;

	existing = target->GetFrame(frame);
	oldData = (existing == 0) ? 0 : existing->data.asInt;

	Do();
}

void AddFrameIAction::Do() {
	if (existing != 0) {
		existing->data.asInt = newData;
	}
	else {
		target->AddFrameI(newData, frame);
	}
}

void AddFrameIAction::Undo() {
	if (existing != 0) {
		existing->data.asInt = oldData;
	}
	else {
		target->RemoveFrame(frame);
	}
}

AddFrameCAction::AddFrameCAction(class Track* _track, ImU32 _data, int _frame) {
	rtti = ActionType::AddFrameCAction;
	target = _track;
	newTint = _data;
	frame = _frame;

	existing = target->GetFrame(frame);
	oldTint = (existing == 0) ? ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1)) : existing->data.asColor;

	Do();
}

void AddFrameCAction::Do() {
	if (existing != 0) {
		existing->data.asColor = newTint;
	}
	else {
		target->AddFrameC(newTint, frame);
	}
}

void AddFrameCAction::Undo() {
	if (existing != 0) {
		existing->data.asColor = oldTint;
	}
	else {
		target->RemoveFrame(frame);
	}
}

AddFrameFAction::AddFrameFAction(Track* _track, float _data, int _frame) {
	rtti = ActionType::AddFrameFAction;
	target = _track;
	newData = _data;
	frame = _frame;

	existing = target->GetFrame(frame);
	oldData = (existing == 0) ? 0.0f : existing->data.asFloat;

	Do();
}

void AddFrameFAction::Do() {
	if (existing != 0) {
		existing->data.asFloat = newData;
	}
	else {
		target->AddFrameF(newData, frame);
	}
}

void AddFrameFAction::Undo() {
	if (existing != 0) {
		existing->data.asFloat = oldData;
	}
	else {
		target->RemoveFrame(frame);
	}
}

AddFrameBAction::AddFrameBAction(class Track* _track, bool _data, int _frame) {
	rtti = ActionType::AddFrameBAction;
	target = _track;
	newData = _data;
	frame = _frame;

	existing = target->GetFrame(frame);
	oldData = (existing == 0) ? false : existing->data.asBool;

	Do();
}

void AddFrameBAction::Do() {
	if (existing != 0) {
		existing->data.asBool = newData;
	}
	else {
		target->AddFrameB(newData, frame);
	}
}

void AddFrameBAction::Undo() {
	if (existing != 0) {
		existing->data.asBool = oldData;
	}
	else {
		target->RemoveFrame(frame);
	}
}

AssetSelectionAction::AssetSelectionAction(class Asset* oldAss, class Asset* newAss, bool oldFocus, bool newFocus, class AnimationAsset* oldSelectedAnim, AnimationAsset* newSelectedAnim, int oldFrame, int newFrame) {
	rtti = ActionType::AssetSelectionAction;
	oldSelection = oldAss;
	newSelection = newAss;
	oldFocusAsset = oldFocus;
	newFocusAsset = newFocus;
	oldAnim = oldSelectedAnim;
	newAnim = newSelectedAnim;
	oldAnimFrame = oldFrame;
	newAnimFrame = newFrame;

	Do();
}

void AssetSelectionAction::Do() {
	Application::GetInstance()->SetSelectedAsset(newSelection, newFocusAsset, newAnim, newAnimFrame);
}

void AssetSelectionAction::Undo() {
	Application::GetInstance()->SetSelectedAsset(oldSelection, oldFocusAsset, oldAnim, oldAnimFrame);
}

HierarchySelectionAction::HierarchySelectionAction(class SceneNode* oldNode, class SceneNode* newNode, bool oldFocus, bool newFocus) {
	rtti = ActionType::HierarchySelectionAction;
	oldSelection = oldNode;
	newSelection = newNode;
	oldFocusNode = oldFocus;
	newFocusNode = newFocus;

	Do();
}

void HierarchySelectionAction::Do() {
	Application::GetInstance()->SetSelectedNode(newSelection, newFocusNode);
}

void HierarchySelectionAction::Undo() {
	Application::GetInstance()->SetSelectedNode(oldSelection, oldFocusNode);
}

NewTransformNodeAction::NewTransformNodeAction(TransformNode* _target, SceneNode* _parent) {
	rtti = ActionType::NewTransformNodeAction;
	target = _target;
	parent = _parent;

	Do();
}

void NewTransformNodeAction::Do() {
	target->isDeleted = false;
	target->SetParent(parent);
}

void NewTransformNodeAction::Undo() {
	target->isDeleted = true;
	target->ClearParent();
}

NewSpriteNodeAction::NewSpriteNodeAction(class SpriteNode* _target, class SceneNode* _parent) {
	rtti = ActionType::NewSpriteNodeAction;
	target = _target;
	parent = _parent;

	Do();
}

void NewSpriteNodeAction::Do() {
	target->isDeleted = false;
	target->SetParent(parent);
}

void NewSpriteNodeAction::Undo() {
	target->isDeleted = true;
	target->ClearParent();
}

DeleteNodeAction::DeleteNodeAction(class SceneNode* node) {
	rtti = ActionType::DeleteNodeAction;
	target = node;
	parent = node->GetParent();
	nextSibling = node->GetNextSibling();;

	Do();
}

void DeleteNodeAction::Do() {
	target->isDeleted = true;
	target->ClearParent();
}

void DeleteNodeAction::Undo() {
	target->isDeleted = false;
	if (parent == 0) {
		target->ClearParent();
	}
	else {
		parent->AddChildBefore(target, nextSibling);
	}
}

ReparentNodeAction::ReparentNodeAction(class SceneNode* node, class SceneNode* parent, class SceneNode* nextSibling) {
	rtti = ActionType::ReparentNodeAction;
	target = node;
	oldParent = node->GetParent();
	oldNextSibling = node->GetNextSibling();
	newParent = parent;
	newNextSibling = nextSibling;
	isValid = newParent != 0 && newParent->CanHaveChildren();

	Do();
}

void ReparentNodeAction::Do() {
	if (newParent == 0) {
		target->ClearParent();
	}
	else {
		newParent->AddChildBefore(target, newNextSibling);
	}
}

void ReparentNodeAction::Undo() {
	if (oldParent == 0) {
		target->ClearParent();
	}
	else {
		oldParent->AddChildBefore(target, oldNextSibling);
	}
}

NewImageAction::NewImageAction(ImageAsset* asset) {
	rtti = ActionType::NewImageAction;
	tracking = asset;
	Do();
}

void NewImageAction::Do() {
	tracking->deleted = false;
}

void NewImageAction::Undo() {
	tracking->deleted = true;
}

NewAnimationAction::NewAnimationAction(class AnimationAsset* asset) {
	rtti = ActionType::NewAnimationAction;
	tracking = asset;
	Do();
}

void NewAnimationAction::Do() {
	tracking->deleted = false;
}
void NewAnimationAction::Undo() {
	tracking->deleted = true;
}

NewAtlasAction::NewAtlasAction(class AtlasAsset* asset) {
	rtti = ActionType::NewAtlasAction;
	tracking = asset;
	Do();
}

void NewAtlasAction::Do() {
	tracking->deleted = false;
}
void NewAtlasAction::Undo() {
	tracking->deleted = true;
}

DeleteAssetAction::DeleteAssetAction(class Asset* node) {
	rtti = ActionType::DeleteAssetAction;
	target = node;
	Do();
}
void DeleteAssetAction::Do() {
	target->deleted = true;
}

void DeleteAssetAction::Undo() {
	target->deleted = false;
}


SetSpriteSizeAction::SetSpriteSizeAction(class SpriteNode* _target, const ImVec2& _newSize, bool _setX, bool _setY) {
	rtti = ActionType::SetSpriteSizeAction;
	IM_ASSERT(_target != 0);
	target = _target;
	setX = _setX;
	setY = _setY;
	newSize = _newSize;
	oldSize = target->size;

	Do();
}

void SetSpriteSizeAction::Do() {
	if (setX) {
		target->size.x = newSize.x;
	}
	if (setY) {
		target->size.y = newSize.y;
	}
}

void SetSpriteSizeAction::Undo() {
	if (setX) {
		target->size.x = oldSize.x;
	}
	if (setY) {
		target->size.y = oldSize.y;
	}
}


SetSpritePivotAction::SetSpritePivotAction(class SpriteNode* _target, const ImVec2& _newPivot, bool _setX, bool _setY) {
	rtti = ActionType::SetSpritePivotAction;
	IM_ASSERT(_target != 0);
	target = _target;
	setX = _setX;
	setY = _setY;
	newPivot = _newPivot;
	oldPivot = target->pivot;

	Do();
}

void SetSpritePivotAction::Do() {
	if (setX) {
		target->pivot.x = newPivot.x;
	}
	if (setY) {
		target->pivot.y = newPivot.y;
	}
}

void SetSpritePivotAction::Undo() {
	if (setX) {
		target->pivot.x = oldPivot.x;
	}
	if (setY) {
		target->pivot.y = oldPivot.y;
	}
}

SetFrameMinAction::SetFrameMinAction(class SpriteNode* _target, const ImVec2& newVal, bool _setX, bool _setY) {
	rtti = ActionType::SetFrameMinAction;
	IM_ASSERT(_target != 0);
	target = _target;
	setX = _setX;
	setY = _setY;
	newValue = newVal;
	oldValue = target->frameMin;

	Do();
}
void SetFrameMinAction::Do() {
	if (setX) {
		target->frameMin.x = newValue.x;
	}
	if (setY) {
		target->frameMin.y = newValue.y;
	}
}
void SetFrameMinAction::Undo() {
	if (setX) {
		target->frameMin.x = oldValue.x;
	}
	if (setY) {
		target->frameMin.y = oldValue.y;
	}
}

SetFrameMaxAction::SetFrameMaxAction(class SpriteNode* _target, const ImVec2& newVal, bool _setX, bool _setY) {
	rtti = ActionType::SetFrameMaxAction;
	IM_ASSERT(_target != 0);
	target = _target;
	setX = _setX;
	setY = _setY;
	newValue = newVal;
	oldValue = target->frameMax;

	Do();
}
void SetFrameMaxAction::Do() {
	if (setX) {
		target->frameMax.x = newValue.x;
	}
	if (setY) {
		target->frameMax.y = newValue.y;
	}
}
void SetFrameMaxAction::Undo() {
	if (setX) {
		target->frameMax.x = oldValue.x;
	}
	if (setY) {
		target->frameMax.y = oldValue.y;
	}
}

SetSpriteRefAction::SetSpriteRefAction(class SpriteNode* _target, ImageAsset* asset, const ImVec2& size_, const ImVec2& min_, const ImVec2& max_) {
	rtti = ActionType::SetSpriteRefAction;
	target = _target;
	oldAsset = target->image;
	newAsset = asset;
	oldSize = target->size;
	newSize = size_;
	oldMin = target->frameMin;
	newMin = min_;
	oldMax = target->frameMax;
	newMax = max_;

	Do();
}
void SetSpriteRefAction::Do() {
	target->frameMin = newMin;
	target->frameMax = newMax;
	target->size = newSize;
	target->image = newAsset;
}

void SetSpriteRefAction::Undo() {
	target->frameMin = oldMin;
	target->frameMax = oldMax;
	target->size =  oldSize;
	target->image = oldAsset;
}

SetFrameRefAction::SetFrameRefAction(class SpriteNode* _target, AtlasFrame* asset, const ImVec2& size_, const ImVec2& min_, const ImVec2& max_) {
	rtti = ActionType::SetFrameRefAction;
	target = _target;
	oldAsset = target->frame;
	newAsset = asset;
	oldSize = target->size;
	newSize = size_;
	oldMin = target->frameMin;
	newMin = min_;
	oldMax = target->frameMax;
	newMax = max_;

	Do();
}

void SetFrameRefAction::Do() {
	target->frameMin = newMin;
	target->frameMax = newMax;
	target->size =	   newSize;
	target->frame =	   newAsset;
}

void SetFrameRefAction::Undo() {
	target->frameMin = oldMin;
	target->frameMax = oldMax;
	target->size =	   oldSize;
	target->frame =    oldAsset;
}

RenameAnimationAction::RenameAnimationAction(class AnimationAsset* _target, const std::string& _newName) {
	rtti = ActionType::RenameAnimationAction;
	target = _target;
	oldName = target->name;
	newName = _newName;

	Do();
}

void RenameAnimationAction::Do() {
	target->name = target->nameInput = newName;
}

void RenameAnimationAction::Undo() {
	target->name = target->nameInput = oldName;
}

SetFrameRateAction::SetFrameRateAction(class AnimationAsset* _target, int fps) {
	rtti = ActionType::SetFrameRateAction;
	if (fps < 0) { fps = 0; }
	target = _target;
	oldFPS = target->frameRate;
	newFPS = fps;

	Do();
}

void SetFrameRateAction::Do() {
	target->frameRate = newFPS;
}

void SetFrameRateAction::Undo() {
	target->frameRate = oldFPS;
}

SetFrameCountAction::SetFrameCountAction(class AnimationAsset* _target, int count) {
	rtti = ActionType::SetFrameCountAction;
	if (count < 2) { count = 2; }
	target = _target;
	oldCount = target->GetFrameCount();
	newCount = count;

	Do();
}

void SetFrameCountAction::Do() {
	target->SetFrameCount(newCount);
}

void SetFrameCountAction::Undo() {
	target->SetFrameCount(oldCount);
}

SetLoopingAction::SetLoopingAction(class AnimationAsset* _target, bool val) {
	rtti = ActionType::SetLoopingAction;
	target = _target;
	oldLoop = target->looping;
	newLoop = val;

	Do();
}

void SetLoopingAction::Do() {
	target->looping = newLoop;
}

void SetLoopingAction::Undo() {
	target->looping = oldLoop;
}

RenameImageAction::RenameImageAction(class ImageAsset* _target, const std::string& _newName) {
	rtti = ActionType::RenameImageAction;
	target = _target;
	oldName = target->name;
	newName = _newName;

	Do();
}

void RenameImageAction::Do() {
	target->name = target->nameInput = newName;
}

void RenameImageAction::Undo() {
	target->name = target->nameInput = oldName;
}

RenameAtlasAction::RenameAtlasAction(class AtlasAsset* _target, const std::string& _newName) {
	rtti = ActionType::RenameAtlasAction;
	target = _target;
	oldName = target->name;
	newName = _newName;

	Do();
}
void RenameAtlasAction::Do() {
	target->name = target->nameInput = newName;
}

void RenameAtlasAction::Undo() {
	target->name = target->nameInput = oldName;
}

SetAtlasSizeAction::SetAtlasSizeAction(class AtlasAsset* _target, const ImVec2& size) {
	rtti = ActionType::SetAtlasSizeAction;
	target = _target;
	oldSize = target->sourceSize;
	newSize = size;

	Do();
}

void SetAtlasSizeAction::Do() {
	target->sourceSize = newSize;
}

void SetAtlasSizeAction::Undo() {
	target->sourceSize = oldSize;
}

AddAtlasFrameAction::AddAtlasFrameAction(class AtlasAsset* _target) {
	rtti = ActionType::AddAtlasFrameAction;
	target = _target;

	frame = target->AddFrame(MakeNewGuid(), ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), false);
	Do();
}

void AddAtlasFrameAction::Do() {
	frame->deleted = false;
}

void AddAtlasFrameAction::Undo() {
	frame->deleted = true;
}

RenameAtlasFrameAction::RenameAtlasFrameAction(class AtlasFrame* _target, const std::string& _newName) {
	target = _target;
	oldName = target->name;
	newName = _newName;
	rtti = ActionType::RenameAtlasFrameAction;


	Do();
}

void RenameAtlasFrameAction::Do() {
	target->name = target->nameInput = newName;
}

void RenameAtlasFrameAction::Undo() {
	target->name = target->nameInput = oldName;
}


AtlasFrameSetPointAction::AtlasFrameSetPointAction(class AtlasFrame* _target, const ImVec2& point, int _index) {
	target = _target;
	oldPoint = target->GetRawPoint(_index);
	newPoint = point;
	index = _index;
	rtti = ActionType::AtlasFrameSetPointAction;

	Do();
}

void AtlasFrameSetPointAction::Do() {
	target->SetRawPoint(index, newPoint);
}

void AtlasFrameSetPointAction::Undo() {
	target->SetRawPoint(index, oldPoint);
}

AtlasFrameSetRotatedAction::AtlasFrameSetRotatedAction(class AtlasFrame* _target, bool val) {
	target = _target;
	oldVal = target->rotated;
	newVal = val;
	rtti = ActionType::AtlasFrameSetRotatedAction;

	Do();
}

void AtlasFrameSetRotatedAction::Do() {
	target->rotated = newVal;
}

void AtlasFrameSetRotatedAction::Undo() {
	target->rotated = oldVal;
}

DeleteAnimFrameAction::DeleteAnimFrameAction(class Track* _target, int rawFrameIndex) {
	IM_ASSERT(_target != 0);
	IM_ASSERT(rawFrameIndex >= 0 && rawFrameIndex < _target->frames.size());
	target = _target;
	frame = target->frames[rawFrameIndex];
	index = rawFrameIndex;

	deleteTarget = target->frames.size() <= 1;
	reviveTarget = !target->deleted;
	rtti = ActionType::DeleteAnimFrameAction;

	Do();
}

void DeleteAnimFrameAction::Do() {
	target->frames.erase(target->frames.begin() + index);

	if (deleteTarget) {
		target->deleted = true;
	}
}

void DeleteAnimFrameAction::Undo() {
	target->frames.insert(target->frames.begin() + index, frame);

	if (reviveTarget) {
		target->deleted = false;
	}
}

DeleteTrackAction::DeleteTrackAction(class Track* _target) {
	target = _target;
	rtti = ActionType::DeleteTrackAction;

	Do();
}

void DeleteTrackAction::Do() {
	target->SetDeleted(true);
}

void DeleteTrackAction::Undo() {
	if (target->HasFrames()) {
		target->SetDeleted(false);
	}
}

DeleteTrackGroupAction::DeleteTrackGroupAction(class Track* _target) {
	head = _target;
	rtti = ActionType::DeleteTrackGroupAction;

	Do();
}

void DeleteTrackGroupAction::Do() {
	for (Track* iter = head; iter != 0; iter = iter->GetNext()) {
		iter->SetDeleted(true);
	}
}

void DeleteTrackGroupAction::Undo() {
	for (Track* iter = head; iter != 0; iter = iter->GetNext()) {
		if (iter->HasFrames()) {
			iter->SetDeleted(false);
		}
	}
}

void InvokeDestructor(OmniAction* omni) {
	Action* a = (Action*)omni;
	ActionType t = a->GetType();

	if (t == ActionType::Action) {
		omni->asAction.~Action();
	}
	else if (t == ActionType::SetPivotToolAction) {
		omni->asSetPivotToolAction.~SetPivotToolAction();
	}
	else if (t == ActionType::FirstAction) {
		omni->asFirstAction.~FirstAction();
	}
	else if (t == ActionType::SetNameAction) {
		omni->asSetNameAction.~SetNameAction();
	}
	else if (t == ActionType::SetPositionAction) {
		omni->asSetPositionAction.~SetPositionAction();
	}
	else if (t == ActionType::SetRotationAction) {
		omni->asSetRotationAction.~SetRotationAction();
	}
	else if (t == ActionType::SetScaleAction) {
		omni->asSetScaleAction.~SetScaleAction();
	}
	else if (t == ActionType::AddFrameFAction) {
		omni->asAddFrameFAction.~AddFrameFAction();
	}
	else if (t == ActionType::AddFrameCAction) {
		omni->asAddFrameCAction.~AddFrameCAction();
	}
	else if (t == ActionType::AddFrameBAction) {
		omni->asAddFrameBAction.~AddFrameBAction();
	}
	else if (t == ActionType::AddFrameIAction) {
		omni->asAddFrameIAction.~AddFrameIAction();
	}
	else if (t == ActionType::SetSortAction) {
		omni->asSetSortAction.~SetSortAction();
	}
	else if (t == ActionType::SetVisibleAction) {
		omni->asSetVisibleAction.~SetVisibleAction();
	}
	else if (t == ActionType::SetTintAction) {
		omni->asSetTintAction.~SetTintAction();
	}
	else if (t == ActionType::HierarchySelectionAction) {
		omni->asSetHierarchyAction.~HierarchySelectionAction();
	}
	else if (t == ActionType::AssetSelectionAction) {
		omni->asAssetSelectionAction.~AssetSelectionAction();
	}
	else if (t == ActionType::NewTransformNodeAction) {
		omni->asNewTransformNodeAction.~NewTransformNodeAction();
	}
	else if (t == ActionType::NewSpriteNodeAction) {
		omni->asNewSpriteNodeAction.~NewSpriteNodeAction();
	}
	else if (t == ActionType::DeleteNodeAction) {
		omni->asDeleteNodeAction.~DeleteNodeAction();
	}
	else if (t == ActionType::DeleteAssetAction) {
		omni->asDeleteAssetAction.~DeleteAssetAction();
	}
	else if (t == ActionType::ReparentNodeAction) {
		omni->asReparentNodeAction.~ReparentNodeAction();
	}
	else if (t == ActionType::NewImageAction) {
		omni->asNewImageAction.~NewImageAction();
	}
	else if (t == ActionType::NewAnimationAction) {
		omni->asNewAnimationAction.~NewAnimationAction();
	}
	else if (t == ActionType::NewAtlasAction) {
		omni->asNewAtlasAction.~NewAtlasAction();
	}
	else if (t == ActionType::SetSpriteSizeAction) {
		omni->asSetSpriteSizeAction.~SetSpriteSizeAction();
	}
	else if (t == ActionType::SetSpritePivotAction) {
		omni->asSetSpritePivotAction.~SetSpritePivotAction();
	}
	else if (t == ActionType::SetFrameMinAction) {
		omni->asSetFrameMinAction.~SetFrameMinAction();
	}
	else if (t == ActionType::SetFrameMaxAction) {
		omni->asSetFrameMaxAction.~SetFrameMaxAction();
	}
	else if (t == ActionType::SetSpriteRefAction) {
		omni->asSetSpriteRefAction.~SetSpriteRefAction();
	}
	else if (t == ActionType::SetFrameRefAction) {
		omni->asSetFrameRefAction.~SetFrameRefAction();
	}
	else if (t == ActionType::RenameAnimationAction) {
		omni->asSetRenameAnimationAction.~RenameAnimationAction();
	}
	else if (t == ActionType::SetFrameRateAction) {
		omni->asSetFrameRateAction.~SetFrameRateAction();
	}
	else if (t == ActionType::SetFrameCountAction) {
		omni->asSetFrameCountAction.~SetFrameCountAction();
	}
	else if (t == ActionType::SetLoopingAction) {
		omni->asSetLoopingAction.~SetLoopingAction();
	}
	else if (t == ActionType::RenameImageAction) {
		omni->asRenameImageAction.~RenameImageAction();
	}
	else if (t == ActionType::RenameAtlasAction) {
		omni->asRenameAtlasAction.~RenameAtlasAction();
	}
	else if (t == ActionType::SetAtlasSizeAction) {
		omni->asSetAtlasSizeAction.~SetAtlasSizeAction();
	}
	else if (t == ActionType::AddAtlasFrameAction) {
		omni->asAddAtlasFrameAction.~AddAtlasFrameAction();
	}
	else if (t == ActionType::RenameAtlasFrameAction) {
		omni->asRenameAtlasFrameAction.~RenameAtlasFrameAction();
	}
	else if (t == ActionType::AtlasFrameSetPointAction) {
		omni->asAtlasFrameSetPointAction.~AtlasFrameSetPointAction();
	}
	else if (t == ActionType::AtlasFrameSetRotatedAction) {
		omni->asAtlasFrameSetRotatedAction.~AtlasFrameSetRotatedAction();
	}
	else if (t == ActionType::DeleteAnimFrameAction) {
		omni->asDeleteAnimationFrameAction.~DeleteAnimFrameAction();
	}
	else if (t == ActionType::DeleteTrackAction) {
		omni->asDeleteTrackAction.~DeleteTrackAction();
	}
	else if (t == ActionType::DeleteTrackGroupAction) {
		omni->asDeleteTrackGroupAction.~DeleteTrackGroupAction();
	}
	else if (t == ActionType::InvalidAction) {
		omni->asInvalidAction.~InvalidAction();
	}
	else {
		IM_ASSERT(false);
	}
}

SetPivotToolAction::SetPivotToolAction(class SpriteNode* _target, const ImVec2& _newPivot, const ImVec2& _newPos) {
	rtti = ActionType::SetPivotToolAction;

	target = _target;
	oldPivot = target->pivot;
	newPivot = _newPivot;
	oldPos = target->position;
	newPos = _newPos;

	Do();
}

void SetPivotToolAction::Do() {
	target->pivot = newPivot;
	target->position = newPos;
}

void SetPivotToolAction::Undo() {
	target->pivot = oldPivot;
	target->position = oldPos;
}