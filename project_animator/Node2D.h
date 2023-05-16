#ifndef _H_TRANSFORM2D_
#define _H_TRANSFORM2D_

#include "../platform/memory.h"
#include "../platform/assert.h"
#include "../platform/math.h"
#include "../framework/vec2.h"

#define NODE2D_DEFAULT_NAME_CAPACITY 36

enum class TrackType {
	TransformPositionX = 0,
	TransformPositionY = 1,
	TransformRotation = 2,
	TransformScaleX = 3,
	TransformScaleY = 4,
	SpriteTintR = 5,
	SpriteTintG = 6,
	SpriteTintB = 7,
	SpriteVisibility = 8,
	SpriteSourceX = 9,
	SpriteSourceY = 10,
	SpriteSourceW = 11,
	SpriteSourceH = 12,
	SortIndex = 13,
};

enum class InterpolationType {
	Linear = 0,
	Step = 1,
	EaseIn = 2,
	EaseOut = 3,
	EaseInOut = 4
};

enum class AnimationLoopMode {
	None = 0,
	Looping = 1
};

struct Frame {
	union {
		f32 fValue;
		u32 uValue;
		i32 iValue;
		bool bValue;
	};

	bool key;
	InterpolationType interp;
};

struct TrackKarrat {
	struct {
		union {
			f32 prevF;
			i32 prevI;
			u32 prevU;
			bool prevB;
		};
	};
	i32 prev_frame;
	struct {
		union {
			f32 nextF;
			i32 nextI;
			u32 nextU;
			bool nextB;
		};
	};
	i32 next_frame;
	float t;
};

struct Track {
	u32 targetNode;
	TrackType targetProp;
	bool avtive; // Deleting does not clear memory

	Frame* frames;
	u32 frameCount;
	u32 frameCapacity;

	u32 numKeyFrames;

	Track* prev;
	Track* next;

private:
	TrackKarrat Seek(i32 _frame, u32 valueAt0, AnimationLoopMode looping) {
		TrackKarrat result = { 0 };
		// Initialize prev to the value of the last key frame. Then search.
							// If search finds a last keyframe, replace frame 0.
		result.prevU = valueAt0;
		if (frames[0].key) {
			result.prevU = frames[0].uValue;
		}

		if (frames[_frame].key) {
			result.prev_frame = _frame;
			result.prevU = frames[_frame].uValue;
		}
		else {
			result.prev_frame = 0;
			for (i32 f = _frame - 1; f >= 0; --f) {
				if (frames[f].key) {
					result.prevU = frames[f].uValue;
					result.prev_frame = f;
					break;
				}
			}
		}

		// next is a bit different. We start by setting the else case as the default
		// then replacing the value if either of the if cases are true
		// if (the animation is looping && frame 0 is a keyframe)
		//     Initial value is the keyframe
		// else if (the animation is looping && frame 0 is not a keyframe)
		//     Initial value is the hierarchy value
		// else (the animation is not looping)
		//     Initial value is the same as the previous frame
		// But, that triage of if statements is just the INITIAL VALUE. After that,
		// we search every frame, and if there is a real next one, override all that work
		result.nextU = result.prevU;
		if (looping == AnimationLoopMode::Looping) {
			if (frames[0].key) {
				result.nextU = frames[0].uValue;
			}
			else {
				result.nextU = valueAt0;
			}
		}
		result.next_frame = frameCount - 1;
		for (i32 f = _frame + 1; f < frameCount; ++f) {
			if (frames[f].key) {
				result.nextU = frames[f].uValue;
				result.next_frame = f;
				break;
			}
		}

		result.t = 0.0f;
		if (result.next_frame != result.prev_frame) {
			result.t = (float)(_frame - result.prev_frame) / (float)(result.next_frame - result.prev_frame);
		}

		return result;
	}
public:
	inline bool InterpolateB(i32 _frame, bool valueAt0, AnimationLoopMode looping) {
		Frame tmp = { 0 };
		tmp.bValue = valueAt0;
		TrackKarrat playhead = Seek(_frame, tmp.uValue, looping);
		PlatformAssert(playhead.t >= 0.0f && playhead.t <= 1.0f, __LOCATION__);
		return playhead.prevB;
	}

	inline i32 InterpolateI(i32 _frame, i32 valueAt0, AnimationLoopMode looping) {
		Frame tmp = { 0 };
		tmp.iValue = valueAt0;
		TrackKarrat playhead = Seek(_frame, tmp.uValue, looping);

		PlatformAssert(playhead.t >= 0.0f && playhead.t <= 1.0f, __LOCATION__);

		InterpolationType interp = InterpolationType::Linear;
		if (frames[playhead.prev_frame].key) {
			interp = frames[playhead.prev_frame].interp;
		}
		if (targetProp == TrackType::SpriteVisibility || targetProp == TrackType::SortIndex) {
			interp = InterpolationType::Step;
		}

		float t = playhead.t;
		if (interp == InterpolationType::Step) {
			t = 0.0f;
		}
		else if (interp == InterpolationType::EaseIn) {
			t = playhead.t * playhead.t * playhead.t;
		}
		else if (interp == InterpolationType::EaseOut) {
			t = 1.0f - MathPow(1.0f - playhead.t, 3.0f);
		}
		else if (interp == InterpolationType::EaseInOut) {
			t = playhead.t < 0.5 ?
				4.0f * playhead.t * playhead.t * playhead.t :
				1.0f - MathPow(-2.0f * playhead.t + 2.0f, 3.0f) / 2.0f;
		}
		else if (interp != InterpolationType::Linear) {
			PlatformAssert(false, __LOCATION__);
		}

		float interpolated_value = (float)playhead.prevI + (float)(playhead.nextI - playhead.prevI) * t;

		return interpolated_value + 0.5f;
	}

	// valueAt0 = selectedNode->position.x
	inline float InterpolateF(i32 _frame, float valueAt0, AnimationLoopMode looping) {
		Frame tmp = { 0 };
		tmp.fValue = valueAt0;
		TrackKarrat playhead = Seek(_frame, tmp.uValue, looping);
		
		PlatformAssert(playhead.t >= 0.0f && playhead.t <= 1.0f, __LOCATION__);

		InterpolationType interp = InterpolationType::Linear;
		if (frames[playhead.prev_frame].key) {
			interp = frames[playhead.prev_frame].interp;
		}
		if (targetProp == TrackType::SpriteVisibility || targetProp == TrackType::SortIndex) {
			interp = InterpolationType::Step;
		}

		float t = playhead.t;
		if (interp == InterpolationType::Step) {
			t = 0.0f;
		}
		else if (interp == InterpolationType::EaseIn) {
			t = playhead.t * playhead.t * playhead.t;
		}
		else if (interp == InterpolationType::EaseOut) {
			t = 1.0f - MathPow(1.0f - playhead.t, 3.0f);
		}
		else if (interp == InterpolationType::EaseInOut) {
			t = playhead.t < 0.5 ? 
				4.0f * playhead.t * playhead.t * playhead.t : 
				1.0f - MathPow(-2.0f * playhead.t + 2.0f, 3.0f) / 2.0f;
		}
		else if (interp != InterpolationType::Linear) {
			PlatformAssert(false, __LOCATION__);
		}

		// Linear
		float interpolated_value = playhead.prevF + (playhead.nextF - playhead.prevF) * t;

		return interpolated_value;
	}

protected:
	inline void ReserveFrames(u32 cap) {
		if (cap == 0) {
			cap = 1;
		}
		if (cap > frameCapacity) {
			frames = (Frame*)MemRealloc(frames, sizeof(Frame) * cap);
			frameCapacity = cap;
		}
	}
public:
	inline void Resize(u32 size) {
		ReserveFrames(size);

		if (size > frameCapacity) {
			for (u32 i = frameCount; i < size; ++i) {
				frames[i].key = false;
			}
		}
		else {
			int iSize = size;
			int iCount = frameCount;
			for (int i = iSize; i >= iCount; --i) {
				//frames[i].Deactivate();
			}
		}
		frameCount = size;
	}
};

struct SpriteComponent {
	float sourceX;
	float sourceY;
	float sourceW;
	float sourceH;
	float pivotX;
	float pivotY;
	float tintR;
	float tintG;
	float tintB;
	float tintA;
	bool  visible;
	u32   resourceUID;
};

class Document;
typedef  int (Document::* SetFramePtrBecauseIDidntThinkThisTrough)(Track* t, u32 frame, InterpolationType interp);  // Please do this!

struct Node2D {
	u32 uid;

	char* name;
	u32 nameLength;
	u32 nameCapacity;

	Node2D* parent;
	Node2D* firstChild;
	Node2D* next;
	u32 depth;

	u32 refCount;

	i32 sortIndex;
	i32 anim_sortIndex;
	bool anim_sortIndex_dirty;
	Node2D* sortedPrev;
	Node2D* sortedNext;

	vec2 position;
	float rotationAngles;
	vec2 scale;

	float minRotation;
	float maxRotation;

	vec2 anim_position;
	float anim_rotationAngles;
	vec2 anim_scale;

	bool anim_positionX_dirty;
	bool anim_positionY_dirty;
	bool anim_rotation_dirty;
	bool anim_scaleX_dirty;
	bool anim_scaleY_dirty;

	SpriteComponent sprite;
	SpriteComponent anim_sprite;

	bool anim_sourceX_dirty;
	bool anim_sourceY_dirty;
	bool anim_sourceW_dirty;
	bool anim_sourceH_dirty;
	bool anim_tintR_dirty;
	bool anim_tintG_dirty;
	bool anim_tintB_dirty;
	bool anim_tintA_dirty;
	bool anim_visible_dirty;

	bool transformToolActive;
	bool uiVisible;
	bool uiExpanded;

	template<typename T>
	inline void SetBuffered(TrackType type, T v) {
		if (type == TrackType::TransformPositionX) {
			anim_position.x = v;
			anim_positionX_dirty = true;
		}
		else if (type == TrackType::TransformPositionY) {
			anim_position.y = v;
			anim_positionY_dirty = true;
		}
		else if (type == TrackType::TransformRotation) {
			
			anim_rotationAngles = v;
			anim_rotation_dirty = true;
		}
		else if (type == TrackType::TransformScaleX) {
			anim_scale.x = v;
			anim_scaleX_dirty = true;
		}
		else if (type == TrackType::TransformScaleY) {
			anim_scale.y = v;
			anim_scaleY_dirty = true;
		}
		else if (type == TrackType::SpriteTintR) {
			anim_sprite.tintR = v;
			anim_tintR_dirty = true;
		}
		else if (type == TrackType::SpriteTintG) {
			anim_sprite.tintG = v;
			anim_tintG_dirty = true;
		}
		else if (type == TrackType::SpriteTintB) {
			anim_sprite.tintB = v;
			anim_tintB_dirty = true;
		}
		else if (type == TrackType::SpriteVisibility) {
			anim_sprite.visible = v;
			anim_visible_dirty = true;
		}
		else if (type == TrackType::SpriteSourceX) {
			anim_sprite.sourceX = v;
			anim_sourceX_dirty = true;
		}
		else if (type == TrackType::SpriteSourceY) {
			anim_sprite.sourceY = v;
			anim_sourceY_dirty = true;
		}
		else if (type == TrackType::SpriteSourceW) {
			anim_sprite.sourceW = v;
			anim_sourceW_dirty = true;
		}
		else if (type == TrackType::SpriteSourceH) {
			anim_sprite.sourceH = v;
			anim_sourceH_dirty = true;
		}
		else if (type == TrackType::SortIndex) {
			anim_sortIndex = v;
			anim_sortIndex_dirty = true;
		}
		else {
			PlatformAssert(false, __LOCATION__);
		}
	}

	template<typename T>
	inline T GetPropertyBuffer(TrackType type) {
		if (type == TrackType::TransformPositionX) {
			return anim_position.x;
		}
		else if (type == TrackType::TransformPositionY) {
			return anim_position.y;
		}
		else if (type == TrackType::TransformRotation) {
			return anim_rotationAngles;
		}
		else if (type == TrackType::TransformScaleX) {
			return anim_scale.x;
		}
		else if (type == TrackType::TransformScaleY) {
			return anim_scale.y;
		}
		else if (type == TrackType::SpriteTintG) {
			return anim_sprite.tintG;
		}
		else if (type == TrackType::SpriteTintR) {
			return anim_sprite.tintR;
		}
		else if (type == TrackType::SpriteTintB) {
			return anim_sprite.tintB;
		}
		else if (type == TrackType::SpriteVisibility) {
			return anim_sprite.visible;
		}
		else if (type == TrackType::SpriteSourceX) {
			return anim_sprite.sourceX;
		}
		else if (type == TrackType::SpriteSourceY) {
			return anim_sprite.sourceY;
		}
		else if (type == TrackType::SpriteSourceW) {
			return anim_sprite.sourceW;
		}
		else if (type == TrackType::SpriteSourceH) {
			return anim_sprite.sourceH;
		}
		else if (type == TrackType::SortIndex) {
			return anim_sortIndex;
		}
		else {
			PlatformAssert(false, __LOCATION__);
		}
		
		return (T)0;
	}

	template<typename T>
	inline float GetProperty(TrackType type) {
		if (type == TrackType::TransformPositionX) {
			return position.x;
		}
		else if (type == TrackType::TransformPositionY) {
			return position.y;
		}
		else if (type == TrackType::TransformRotation) {
			return rotationAngles;
		}
		else if (type == TrackType::TransformScaleX) {
			return scale.x;
		}
		else if (type == TrackType::TransformScaleY) {
			return scale.y;
		}
		else if (type == TrackType::SpriteTintR) {
			return sprite.tintR;
		}
		else if (type == TrackType::SpriteTintG) {
			return sprite.tintG;
		}
		else if (type == TrackType::SpriteTintB) {
			return sprite.tintB;
		}
		else if (type == TrackType::SpriteVisibility) {
			return sprite.visible;
		}
		else if (type == TrackType::SpriteSourceX) {
			return sprite.sourceX;
		}
		else if (type == TrackType::SpriteSourceY) {
			return sprite.sourceY;
		}
		else if (type == TrackType::SpriteSourceW) {
			return sprite.sourceW;
		}
		else if (type == TrackType::SpriteSourceH) {
			return sprite.sourceH;
		}
		else if (type == TrackType::SortIndex) {
			return sortIndex;
		}
		else {
			PlatformAssert(false, __LOCATION__);
		}

		return (T)0;
	}

	// Resets cached flag, call only once!
	inline Frame GetIneterpolatedFrame(Track* t, u32 f, bool _looping) {
		AnimationLoopMode looping = _looping? AnimationLoopMode::Looping : AnimationLoopMode::None;
		Frame unionFrame = { 0 };
		TrackType trackType = t->targetProp;

		if (trackType == TrackType::TransformPositionX) {
			if (anim_positionX_dirty) {
				unionFrame.fValue = anim_position.x;
				anim_positionX_dirty = false;
			}

			else {
				unionFrame.fValue = t->InterpolateF(f, position.x, looping);
			}
		}
		else if (trackType == TrackType::TransformPositionY) {
			if (anim_positionY_dirty) {
				unionFrame.fValue = anim_position.y;
				anim_positionY_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, position.y, looping);
			}
		}
		else if (trackType == TrackType::TransformRotation) {
			if (anim_rotation_dirty) {
				unionFrame.fValue = anim_rotationAngles;
				anim_rotation_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, rotationAngles, looping);
			}
		}
		else if (trackType == TrackType::TransformScaleX) {
			if (anim_scaleX_dirty) {
				unionFrame.fValue = anim_scale.x;
				anim_scaleX_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, scale.x, looping);
			}
		}
		else if (trackType == TrackType::TransformScaleY) {
			if (anim_scaleY_dirty) {
				unionFrame.fValue = anim_scale.y;
				anim_scaleY_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, scale.y, looping);
			}
		}
		
		else if (trackType == TrackType::SpriteTintR) {
			if (anim_tintR_dirty) {
				unionFrame.fValue = anim_sprite.tintR;
				anim_tintR_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, sprite.tintR, looping);
			}
		}
		else if (trackType == TrackType::SpriteTintG) {
			if (anim_tintG_dirty) {
				unionFrame.fValue = anim_sprite.tintG;
				anim_tintG_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, sprite.tintG, looping);
			}
		}
		else if (trackType == TrackType::SpriteTintB) {
			if (anim_tintB_dirty) {
				unionFrame.fValue = anim_sprite.tintB;
				anim_tintB_dirty = false;
			}
			else {
				unionFrame.fValue = t->InterpolateF(f, sprite.tintB, looping);
			}
		}
		else if (trackType == TrackType::SpriteVisibility) {
			if (anim_visible_dirty) {
				unionFrame.bValue = anim_sprite.visible;
				anim_visible_dirty = false;
			}
			else {
				unionFrame.bValue = t->InterpolateB(f, sprite.visible, looping);
			}
		}
		else if (trackType == TrackType::SpriteSourceX) {
			if (anim_sourceX_dirty) {
				unionFrame.iValue = anim_sprite.sourceX;
				anim_sourceX_dirty = false;
			}
			else {
				unionFrame.iValue = t->InterpolateI(f, sprite.sourceX, looping);
			}
		}
		else if (trackType == TrackType::SpriteSourceY) {
			if (anim_sourceY_dirty) {
				unionFrame.iValue = anim_sprite.sourceY;
				anim_sourceY_dirty = false;
			}
			else {
				unionFrame.iValue = t->InterpolateI(f, sprite.sourceY, looping);
			}
		}
		else if (trackType == TrackType::SpriteSourceW) {
			if (anim_sourceW_dirty) {
				unionFrame.iValue = anim_sprite.sourceW;
				anim_sourceW_dirty = false;
			}
			else {
				unionFrame.iValue = t->InterpolateI(f, sprite.sourceW, looping);
			}
		}
		else if (trackType == TrackType::SpriteSourceH) {
			if (anim_sourceH_dirty) {
				unionFrame.iValue = anim_sprite.sourceH;
				anim_sourceH_dirty = false;
			}
			else {
				unionFrame.iValue = t->InterpolateI(f, sprite.sourceH, looping);
			}
		}
		else if (trackType == TrackType::SortIndex) {
			if (anim_sortIndex_dirty) {
				unionFrame.iValue = anim_sortIndex;
				anim_sortIndex_dirty = false;
			}
			else {
				unionFrame.iValue = t->InterpolateI(f, sortIndex, looping);
			}
		}
		else {
			PlatformAssert(false, __LOCATION__);
		}

		return unionFrame;
	}

	inline bool IsPropertyDirty(TrackType type) {
		if (type == TrackType::TransformPositionX) {
			return anim_positionX_dirty;
		}
		else if (type == TrackType::TransformPositionY) {
			return anim_positionY_dirty;
		}
		else if (type == TrackType::TransformRotation) {
			return anim_rotation_dirty;
		}
		else if (type == TrackType::TransformScaleX) {
			return anim_scaleX_dirty;
		}
		else if (type == TrackType::TransformScaleY) {
			return anim_scaleY_dirty;
		}
		else if (type == TrackType::SpriteTintR) {
			return anim_tintR_dirty;
		}
		else if (type == TrackType::SpriteTintG) {
			return anim_tintG_dirty;
		}
		else if (type == TrackType::SpriteTintB) {
			return anim_tintB_dirty;
		}
		else if (type == TrackType::SpriteVisibility) {
			return anim_visible_dirty;
		}
		else if (type == TrackType::SpriteSourceX) {
			return anim_sourceX_dirty;
		}
		else if (type == TrackType::SpriteSourceY) {
			return anim_sourceY_dirty;
		}
		else if (type == TrackType::SpriteSourceW) {
			return anim_sourceW_dirty;
		}
		else if (type == TrackType::SpriteSourceH) {
			return anim_sourceH_dirty;
		}
		else if (type == TrackType::SortIndex) {
			return anim_sortIndex_dirty;
		}
		else {
			PlatformAssert(false, __LOCATION__);
		}
		return false;
	}
};

struct Animation {
	u32 uid;
	char* name;
	i32 refCount;

	u32 frameCount;
	u32 frameRate;

	AnimationLoopMode loop;

	Track* tracks;
	u32 numTracks;

	Animation* prev;
	Animation* next;

	inline f32 GetDuration() {
		return (f32)frameCount / (f32)frameRate;
	}

	inline f32 FrameDuration() {
		return 1.0f / (f32)frameRate;
	}

	inline f32 FrameDurationMs() {
		return 1000.0f / (f32)frameRate;
	}

	void ReleaseMemory();

	inline Track* Contains(Node2D* node, TrackType type) {
		if (node == 0) {
			return 0;
		}
		u32 nodeId = node->uid;
		for (Track* i = tracks; i != 0; i = i->next) {
			if (i->targetNode == nodeId && i->avtive && i->targetProp == type) {
				return i;
			}
		}
		return 0;
	}
};

struct SerializedHierarchy {
	Node2D* root;
	void* data;
};

#if 0
SerializedHierarchy Serialize(Node2D* node);
Node2D* Deserialize(SerializedHierarchy* data);
#endif

typedef void (NodeVisitor)(Node2D* node, void* userData);
void RecursivleyDestroyNode(Node2D* node); // Unhook & delete. Recursive in name only.
void ForEachNode(Node2D* node, NodeVisitor callback, void* userData);

Node2D* GetLastChild(Node2D& node);
Node2D* GetChildById(Node2D& node, u32 uid);
bool RemoveChild(Node2D& parent, Node2D& child);
// Made these into private APIs
//bool AddChild(Node2D& parent, Node2D& child);
//bool AddChild(Node2D& parent, Node2D& child, u32 lastSibling);
void SetParent(Node2D* parent, Node2D& child);
void SetParent(Node2D* parent, Node2D& child, u32 lastSibling);
void SetName(Node2D& node, const char* name);

u32 CountDescendants(Node2D* node);

// Returns true if released
bool RecursivleyDestroyNodeIfItHasNoReferences(Node2D* node);

#endif // !_H_TRANSFORM2D_
