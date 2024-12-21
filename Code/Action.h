#pragma once

#include <string>
#include "Libraries/ImMath.h"
#include "Libraries/imgui.h"
#include "Track.h"

enum class ActionType {
	Action = 0,
	InvalidAction,
	FirstAction,
	SetNameAction,
	SetPositionAction,
	SetRotationAction,
	SetScaleAction,
	AddFrameFAction,
	AddFrameCAction,
	AddFrameBAction,
	AddFrameIAction,
	SetSortAction,
	SetVisibleAction,
	SetTintAction,
	HierarchySelectionAction,
	AssetSelectionAction,
	NewTransformNodeAction,
	NewSpriteNodeAction,
	DeleteNodeAction,
	DeleteAssetAction,
	ReparentNodeAction,
	NewImageAction,
	NewAnimationAction,
	NewAtlasAction,
	SetSpriteSizeAction,
	SetSpritePivotAction,
	SetPivotToolAction,
	SetFrameMinAction,
	SetFrameMaxAction,
	SetSpriteRefAction,
	SetFrameRefAction,
	RenameAnimationAction,
	SetFrameRateAction,
	SetFrameCountAction,
	SetLoopingAction,
	RenameImageAction,
	RenameAtlasAction,
	SetAtlasSizeAction,
	AddAtlasFrameAction,
	RenameAtlasFrameAction,
	AtlasFrameSetPointAction,
	AtlasFrameSetRotatedAction,
	DeleteAnimFrameAction,
	DeleteTrackAction,
	DeleteTrackGroupAction
};

class Action {
	friend class UndoManager;
protected: // Intrusive linked list stuff
	ActionType rtti;
protected: 
	Action();
	Action(const Action&) = delete;
	Action& operator=(const Action&) = delete;
public:
	virtual ~Action();
	virtual void Do();
	virtual void Undo();

	virtual const char* GetName();
	virtual ActionType GetType();
};

class InvalidAction : public Action {
	friend class UndoManager;
public:
	InvalidAction();
	virtual ~InvalidAction();
	virtual void Do();
	virtual void Undo();

	virtual const char* GetName();
};

class FirstAction : public Action {
	friend class UndoManager;
public:
	FirstAction();
	virtual ~FirstAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Initial State";
	}
};

class SetNameAction : public Action {
	friend class UndoManager;
protected:
	class SceneNode* target;
	std::string oldName;
	std::string newName;

	//static void* operator new(std::size_t) = delete;
public:
	SetNameAction(class SceneNode* target, const std::string& name);
	virtual ~SetNameAction();
	
	//static void* operator new(std::size_t size, class UndoManager& udm);
	//void operator delete(void* memory, <your parameters here>);

	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Rename Node";
	}
};

class SetPositionAction : public Action {
	friend class UndoManager;
protected:
	bool setX;
	bool setY;
	ImVec2 oldPosition;
	ImVec2 newPosition;
	class TransformNode* target;
public:
	SetPositionAction(class TransformNode* target, const ImVec2& newPos, bool setX, bool setY);
	virtual ~SetPositionAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (setX && setY) {
			return "Set Position";
		}
		else if (setX) {
			return "Set Position X";
		}
		else if (setY) {
			return "Set Position Y";
		}
		IM_ASSERT(false);
		return "INVALID";
	}
};

class SetRotationAction : public Action {
	friend class UndoManager;
protected:
	class TransformNode* target;
	float oldRotation;
	float newRotation;
public:
	SetRotationAction(class TransformNode* target, float newRot);
	virtual ~SetRotationAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Set Rotation";
	}
};

class SetScaleAction : public Action {
	friend class UndoManager;
protected:
	bool setX;
	bool setY;
	ImVec2 oldScale;
	ImVec2 newScale;
	class TransformNode* target;
public:
	SetScaleAction(class TransformNode* target, const ImVec2& newScale, bool setX, bool setY);
	virtual ~SetScaleAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (setX && setY) {
			return "Set Scale";
		}
		else if (setX) {
			return "Set Scale X";
		}
		else if (setY) {
			return "Set Scale Y";
		}
		IM_ASSERT(false);
		return "INVALID";
	}
};

class AddFrameFAction : public Action {
	friend class UndoManager;
protected:
	class Track* target;
	class Frame* existing;
	float oldData; // Only valid if existing != null
	float newData;
	int frame;
public:
	AddFrameFAction(class Track* _track, float _data, int _frame);
	virtual ~AddFrameFAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (existing == 0) {
			return "Add Animation Frame";
		}
		return "Update Animation Frame";
	}
};

class AddFrameCAction : public Action {
	friend class UndoManager;
protected:
	class Track* target;
	class Frame* existing;
	ImU32 oldTint; // Only valid if existing != null
	ImU32 newTint;
	int frame;
public:
	AddFrameCAction(class Track* _track, ImU32 _data, int _frame);
	virtual ~AddFrameCAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (existing == 0) {
			return "Add Animation Frame";
		}
		return "Update Animation Frame";
	}
};

class AddFrameBAction : public Action {
	friend class UndoManager;
protected:
	class Track* target;
	class Frame* existing;
	bool oldData; // Only valid if existing != null
	bool newData;
	int frame;
public:
	AddFrameBAction(class Track* _track, bool _data, int _frame);
	virtual ~AddFrameBAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (existing == 0) {
			return "Add Animation Frame";
		}
		return "Update Animation Frame";
	}
};

class AddFrameIAction : public Action {
	friend class UndoManager;
protected:
	class Track* target;
	class Frame* existing;
	int oldData; // Only valid if existing != null
	int newData;
	int frame;
public:
	AddFrameIAction(class Track* _track, int _data, int _frame);
	virtual ~AddFrameIAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (existing == 0) {
			return "Add Animation Frame";
		}
		return "Update Animation Frame";
	}
};

class SetSortAction : public Action {
	friend class UndoManager;
protected:
	class SpriteNode* target;
	int oldSort;
	int newSort;
public:
	SetSortAction(class SpriteNode* _target, int sort);
	virtual ~SetSortAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (oldSort < newSort) {
			return "Increase Sort Index";
		}
		else if (oldSort > newSort) {
			return "Decrease Sort Index";
		}
		return "Adjust Sort Index";
	}
};

class SetVisibleAction : public Action {
	friend class UndoManager;
protected:
	class SpriteNode* target;
	bool oldVisible;
	bool newVisible;
public:
	SetVisibleAction(class SpriteNode* _target, bool vis);
	virtual ~SetVisibleAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (newVisible) {
			return "Set Visibility (true)";
		}
		return "Set Visibility (false)";
	}
};

class SetTintAction : public Action {
	friend class UndoManager;
protected:
	class SpriteNode* target;
	ImU32 oldTint;
	ImU32 newTint;
public:
	SetTintAction(class SpriteNode* _target, ImU32 tint);
	virtual ~SetTintAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Set Color / Tint";
	}
};

class HierarchySelectionAction : public Action {
	friend class UndoManager;
protected:
	class SceneNode* oldSelection;
	class SceneNode* newSelection;
	bool oldFocusNode;
	bool newFocusNode;
public:
	HierarchySelectionAction(class SceneNode* oldNode, class SceneNode* newNode, bool oldFocus, bool newFocus);
	virtual ~HierarchySelectionAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Hierarchy Selection";
	}
};

class AssetSelectionAction : public Action {
	friend class UndoManager;
protected:
	class Asset* oldSelection;
	class Asset* newSelection;
	bool oldFocusAsset;
	bool newFocusAsset;
	class AnimationAsset* oldAnim;
	class AnimationAsset* newAnim;
	int oldAnimFrame;
	int newAnimFrame;
public:
	AssetSelectionAction(class Asset* oldAss, class Asset* newAss, bool oldFocus, bool newFocus, class AnimationAsset* oldSelectedAnim, AnimationAsset* newSelectedAnim, int oldFrame, int newFrame);
	virtual ~AssetSelectionAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Asset Selection";
	}
};

class NewTransformNodeAction : public Action {
	friend class UndoManager;
protected:
	class TransformNode* target;
	class SceneNode* parent;
public:
	NewTransformNodeAction(class TransformNode* _target, class SceneNode* _parent);
	virtual ~NewTransformNodeAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "New Transform Node";
	}
};

class NewSpriteNodeAction : public Action {
	friend class UndoManager;
protected:
	class SpriteNode* target;
	class SceneNode* parent;
public:
	NewSpriteNodeAction(class SpriteNode* _target, class SceneNode* _parent);
	virtual ~NewSpriteNodeAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "New Sprite Node";
	}
};

class DeleteNodeAction : public Action {
	friend class UndoManager;
protected:
	class SceneNode* target;
	class SceneNode* parent;
	class SceneNode* nextSibling;
public:
	DeleteNodeAction(class SceneNode* node);
	virtual ~DeleteNodeAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Delete Node";
	}
};

class DeleteAssetAction : public Action {
	friend class UndoManager;
protected:
	class Asset* target;
public:
	DeleteAssetAction(class Asset* node);
	virtual ~DeleteAssetAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Delete Asset";
	}
};

class ReparentNodeAction : public Action {
	friend class UndoManager;
protected:
	class SceneNode* target;
	class SceneNode* oldParent;
	class SceneNode* newParent;
	class SceneNode* oldNextSibling;
	class SceneNode* newNextSibling;
	bool isValid;
public:
	ReparentNodeAction(class SceneNode* node, class SceneNode* parent, class SceneNode* nextSibling);
	virtual ~ReparentNodeAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Reparent Node";
	}
};

class NewImageAction : public Action {
	friend class UndoManager;
protected:
	class ImageAsset* tracking;
public:
	NewImageAction(class ImageAsset* asset);
	virtual ~NewImageAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Import Image";
	}
};

class NewAnimationAction : public Action {
	friend class UndoManager;
protected:
	class AnimationAsset* tracking;
public:
	NewAnimationAction(class AnimationAsset* asset);
	virtual ~NewAnimationAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Create Animation";
	}
};

class NewAtlasAction : public Action {
	friend class UndoManager;
protected:
	class AtlasAsset* tracking;
public:
	NewAtlasAction(class AtlasAsset* asset);
	virtual ~NewAtlasAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Create Atlas";
	}
};

class SetSpriteSizeAction : public Action {
	friend class UndoManager;
protected:
	bool setX;
	bool setY;
	ImVec2 oldSize;
	ImVec2 newSize;
	class SpriteNode* target;
public:
	SetSpriteSizeAction(class SpriteNode* target, const ImVec2& newSize, bool setX, bool setY);
	virtual ~SetSpriteSizeAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (setX && setY) {
			return "Set Size";
		}
		else if (setX) {
			return "Set Size W";
		}
		else if (setY) {
			return "Set Size H";
		}
		IM_ASSERT(false);
		return "INVALID";
	}
};

class SetSpritePivotAction : public Action {
	friend class UndoManager;
protected:
	bool setX;
	bool setY;
	ImVec2 oldPivot;
	ImVec2 newPivot;
	class SpriteNode* target;
public:
	SetSpritePivotAction(class SpriteNode* target, const ImVec2& newPivot, bool setX, bool setY);
	virtual ~SetSpritePivotAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (setX && setY) {
			return "Set Pivot";
		}
		else if (setX) {
			return "Set Pivot X";
		}
		else if (setY) {
			return "Set Pivot Y";
		}
		IM_ASSERT(false);
		return "INVALID";
	}
};

class SetFrameMinAction : public Action {
	friend class UndoManager;
protected:
	bool setX;
	bool setY;
	ImVec2 oldValue;
	ImVec2 newValue;
	class SpriteNode* target;
public:
	SetFrameMinAction(class SpriteNode* target, const ImVec2& newVal, bool setX, bool setY);
	virtual ~SetFrameMinAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (setX && setY) {
			return "Set Frame Min";
		}
		else if (setX) {
			return "Set Frame Min X";
		}
		else if (setY) {
			return "Set Frame Min Y";
		}
		IM_ASSERT(false);
		return "INVALID";
	}
};

class SetFrameMaxAction : public Action {
	friend class UndoManager;
protected:
	bool setX;
	bool setY;
	ImVec2 oldValue;
	ImVec2 newValue;
	class SpriteNode* target;
public:
	SetFrameMaxAction(class SpriteNode* target, const ImVec2& newVal, bool setX, bool setY);
	virtual ~SetFrameMaxAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (setX && setY) {
			return "Set Frame Max";
		}
		else if (setX) {
			return "Set Frame Max X";
		}
		else if (setY) {
			return "Set Frame Max Y";
		}
		IM_ASSERT(false);
		return "INVALID";
	}
};

class SetSpriteRefAction : public Action {
	friend class UndoManager;
protected:
	class ImageAsset* oldAsset;
	class ImageAsset* newAsset;
	ImVec2 oldSize;
	ImVec2 newSize;
	ImVec2 oldMin;
	ImVec2 newMin;
	ImVec2 oldMax;
	ImVec2 newMax;
	class SpriteNode* target;
public:
	SetSpriteRefAction(class SpriteNode* target, ImageAsset* asset, const ImVec2& size_, const ImVec2& min_, const ImVec2& max_);
	virtual ~SetSpriteRefAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (newAsset != 0) {
			return "Set Sprite Reference";
		}
		return "Clear Sprite Reference";
	}
};

class SetFrameRefAction : public Action {
	friend class UndoManager;
protected:
	class AtlasFrame* oldAsset;
	class AtlasFrame* newAsset;
	ImVec2 oldSize;
	ImVec2 newSize;
	ImVec2 oldMin;
	ImVec2 newMin;
	ImVec2 oldMax;
	ImVec2 newMax;
	class SpriteNode* target;
public:
	SetFrameRefAction(class SpriteNode* target, AtlasFrame* asset, const ImVec2& size_, const ImVec2& min_, const ImVec2& max_);
	virtual ~SetFrameRefAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (newAsset != 0) {
			return "Set Frame Reference";
		}
		return "Clear Frame Reference";
	}
};

class RenameAnimationAction : public Action {
	friend class UndoManager;
protected:
	class AnimationAsset* target;
	std::string oldName;
	std::string newName;
public:
	RenameAnimationAction(class AnimationAsset* target, const std::string& newName);
	virtual ~RenameAnimationAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Rename Animation";
	}
}; 

class SetFrameRateAction : public Action {
	friend class UndoManager;
protected:
	class AnimationAsset* target;
	int newFPS;
	int oldFPS;
public:
	SetFrameRateAction(class AnimationAsset* target, int fps);
	virtual ~SetFrameRateAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Set Framerate";
	}
};

class SetFrameCountAction : public Action {
	friend class UndoManager;
protected:
	class AnimationAsset* target;
	int newCount;
	int oldCount;
public:
	SetFrameCountAction(class AnimationAsset* target, int count);
	virtual ~SetFrameCountAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Set Frame Count";
	}
};

class SetLoopingAction : public Action {
	friend class UndoManager;
protected:
	class AnimationAsset* target;
	bool newLoop;
	bool oldLoop;
public:
	SetLoopingAction(class AnimationAsset* target, bool val);
	virtual ~SetLoopingAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (newLoop) {
			return "Set Looping (true)";
		}
		return "Set Looping (false)";
	}
};

class RenameImageAction : public Action {
	friend class UndoManager;
protected:
	class ImageAsset* target;
	std::string oldName;
	std::string newName;
public:
	RenameImageAction(class ImageAsset* target, const std::string& newName);
	virtual ~RenameImageAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Rename Image";
	}
};

class RenameAtlasAction : public Action {
	friend class UndoManager;
protected:
	class AtlasAsset* target;
	std::string oldName;
	std::string newName;
public:
	RenameAtlasAction(class AtlasAsset* target, const std::string& newName);
	virtual ~RenameAtlasAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Rename Atlas";
	}
};

class SetAtlasSizeAction : public Action {
	friend class UndoManager;
protected:
	class AtlasAsset* target;
	ImVec2 oldSize;
	ImVec2 newSize;
public:
	SetAtlasSizeAction(class AtlasAsset* _target, const ImVec2& size);
	virtual ~SetAtlasSizeAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Set Atlas Size";
	}
};

class AddAtlasFrameAction : public Action {
	friend class UndoManager;
protected:
	class AtlasAsset* target;
	class AtlasFrame* frame;
public:
	AddAtlasFrameAction(class AtlasAsset* _target);
	virtual ~AddAtlasFrameAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Create Atlas Frame";
	}
};

class RenameAtlasFrameAction : public Action {
	friend class UndoManager;
protected:
	class AtlasFrame* target;
	std::string oldName;
	std::string newName;
public:
	RenameAtlasFrameAction(class AtlasFrame* _target, const std::string& _newName);
	virtual ~RenameAtlasFrameAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Rename Atlas Frame";
	}
};

class AtlasFrameSetPointAction : public Action {
	friend class UndoManager;
protected:
	class AtlasFrame* target;
	ImVec2 oldPoint;
	ImVec2 newPoint;
	int index;
public:
	AtlasFrameSetPointAction(class AtlasFrame* _target, const ImVec2& point, int _index);
	virtual ~AtlasFrameSetPointAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (index == 0) {
			return "Frame Set Point 1";
		}
		else if (index == 1) {
			return "Frame Set Point 2";
		}
		else if (index == 2) {
			return "Frame Set Point 3";
		}
		else if (index == 3) {
			return "Frame Set Point 4";
		}

		IM_ASSERT(false);
		return "AtlasFrameSetPointAction::INVALID";
	}
};

class AtlasFrameSetRotatedAction : public Action {
	friend class UndoManager;
protected:
	class AtlasFrame* target;
	bool oldVal;
	bool newVal;
public:
	AtlasFrameSetRotatedAction(class AtlasFrame* _target, bool val);
	virtual ~AtlasFrameSetRotatedAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		if (newVal) {
			return "Set Frame Rotation (true)";
		}
		return "Set Frame Rotation (false)";
	}
};

class DeleteAnimFrameAction : public Action {
	friend class UndoManager;
protected:
	class Track* target;
	Frame frame;
	int index;
	bool deleteTarget;
	bool reviveTarget;
public:
	DeleteAnimFrameAction(class Track* _target, int rawFrameIndex);
	virtual ~DeleteAnimFrameAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Delete Anim Frame";
	}
};

class DeleteTrackAction : public Action {
	friend class UndoManager;
protected:
	class Track* target;
public:
	DeleteTrackAction(class Track* _target);
	virtual ~DeleteTrackAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Delete Anim Track";
	}
};

class DeleteTrackGroupAction : public Action {
	friend class UndoManager;
protected:
	class Track* head;
public:
	DeleteTrackGroupAction(class Track* _target);
	virtual ~DeleteTrackGroupAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Delete Node Tracks";
	}
};

class SetPivotToolAction : public Action {
	friend class UndoManager;
protected:
	ImVec2 oldPos;
	ImVec2 newPos;
	ImVec2 oldPivot;
	ImVec2 newPivot;
	class SpriteNode* target;
public:
	SetPivotToolAction(class SpriteNode* target, const ImVec2& newPivot, const ImVec2& newPos);
	virtual ~SetPivotToolAction();
	virtual void Do();
	virtual void Undo();

	inline virtual const char* GetName() {
		return "Set Pivot";
	}
};

class OmniAction {
public:
	~OmniAction() = delete;
	union {
		Action asAction;
		InvalidAction asInvalidAction;
		FirstAction asFirstAction;
		SetNameAction asSetNameAction;
		SetPivotToolAction asSetPivotToolAction;
		SetPositionAction asSetPositionAction;
		SetRotationAction asSetRotationAction;
		SetScaleAction asSetScaleAction;
		AddFrameFAction asAddFrameFAction;
		AddFrameCAction asAddFrameCAction;
		AddFrameBAction asAddFrameBAction;
		AddFrameIAction asAddFrameIAction;
		SetSortAction asSetSortAction;
		SetVisibleAction asSetVisibleAction;
		SetTintAction asSetTintAction;
		HierarchySelectionAction asSetHierarchyAction;
		AssetSelectionAction asAssetSelectionAction;
		NewTransformNodeAction asNewTransformNodeAction;
		NewSpriteNodeAction asNewSpriteNodeAction;
		DeleteNodeAction asDeleteNodeAction;
		DeleteAssetAction asDeleteAssetAction;
		ReparentNodeAction asReparentNodeAction;
		NewImageAction asNewImageAction;
		NewAnimationAction asNewAnimationAction;
		NewAtlasAction asNewAtlasAction;
		SetSpriteSizeAction asSetSpriteSizeAction;
		SetSpritePivotAction asSetSpritePivotAction;
		SetFrameMinAction asSetFrameMinAction;
		SetFrameMaxAction asSetFrameMaxAction;
		SetSpriteRefAction asSetSpriteRefAction;
		SetFrameRefAction asSetFrameRefAction;
		RenameAnimationAction asSetRenameAnimationAction;
		SetFrameRateAction asSetFrameRateAction;
		SetFrameCountAction asSetFrameCountAction;
		SetLoopingAction asSetLoopingAction;
		RenameImageAction asRenameImageAction;
		RenameAtlasAction asRenameAtlasAction;
		SetAtlasSizeAction asSetAtlasSizeAction;
		AddAtlasFrameAction asAddAtlasFrameAction;
		RenameAtlasFrameAction asRenameAtlasFrameAction;
		AtlasFrameSetPointAction asAtlasFrameSetPointAction;
		AtlasFrameSetRotatedAction asAtlasFrameSetRotatedAction;
		DeleteAnimFrameAction asDeleteAnimationFrameAction;
		DeleteTrackAction asDeleteTrackAction;
		DeleteTrackGroupAction asDeleteTrackGroupAction;
	};
};

void InvokeDestructor(OmniAction* action);