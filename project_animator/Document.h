#ifndef _H_DOCUMENT_
#define _H_DOCUMENT_

#include "Node2D.h"
#include "../framework/draw2d.h"
#include "../framework/Transform.h"

struct Resource {
    u32 uid;

    char* name;
    u32 nameLen;
    void* data;
    u32 size;
    u32 image;
    Draw2D::Interpolation filter;
    u32 width;
    u32 height;

    Resource* next;
    Resource* prev;
};

typedef void (*OnResourceLoaded)(const char* path, const Resource& resource, void* userData);



inline const char* TrackTypeToString(TrackType trackType) {
    if (trackType == TrackType::TransformPositionX) {
        return ".Position.X";
    }
    else if (trackType == TrackType::TransformPositionY) {
        return ".Position.Y";
    }
    else if (trackType == TrackType::TransformRotation) {
        return ".Rotation";
    }
    else if (trackType == TrackType::TransformScaleX) {
        return ".ScaleX";
    }
    else if (trackType == TrackType::TransformScaleY) {
        return ".ScaleY";
    }
    else if (trackType == TrackType::SpriteTintR) {
        return ".Tint.R";
    }
    else if (trackType == TrackType::SpriteTintG) {
        return ".Tint.G";
    }
    else if (trackType == TrackType::SpriteTintB) {
        return ".Tint.B";
    }
    else if (trackType == TrackType::SpriteVisibility) {
        return ".Visibility";
    }
    else if (trackType == TrackType::SpriteSourceX) {
        return ".Sprite.X";
    }
    else if (trackType == TrackType::SpriteSourceY) {
        return ".Sprite.Y";
    }
    else if (trackType == TrackType::SpriteSourceW) {
        return ".Sprite.W";
    }
    else if (trackType == TrackType::SpriteSourceH) {
        return ".Sprite.H";
    }
    else if (trackType == TrackType::SortIndex) {
        return ".SortIndex";
    }
    
    return "[NULL]";
}


struct Document {
    friend struct CreateNodeAction;
    friend struct DeleteNodeAction;
    friend struct HierarchySelectAction;
    friend struct RearrangeNodeAction;
    friend struct CreateAnimationAction;
    friend struct DeleteAnimationAction;
    friend struct StaticActionSize* GrabNextUndoSlot(Document*);
public:
    u32 nodeUidGenerator;
    u32 resourceUidGenerator;
    u32 animationUidGenerator;

    Node2D* rootNode;
    u32 numNodes;
    Node2D* selectedNode;

    Resource* resources;
    u32 numResources;
    Resource* selectedResource;

    struct StaticActionSize* undoStack;
    u32 undoNumSteps;
    u32 undoStackTop;
    u32 undoStackCurrent; // 1 past the last undo
    void ClearUndoHistory(u32 firstIndex, u32 lastIndex);

    Animation* allAnimations;
    Animation* selectedAnimation;
    u32 numAnimations;

    Animation* timelineAnimation;
    Track* selectedTrack;
    i32 timelineSelectedFrame;
    i32 timelineLastSelectedFrame;

    i32 needsEndOfFrameUpdate;
    i32 selectedInterpolationType;

    float GetLocalProp(Node2D* node, TrackType trackType);
public:
    Document(u32 numUndoSteps);
    ~Document();

    Node2D* FindNodeById(u32 nodeId);
    Resource* FindResourceById(u32 resourceId);
    Animation* FindAnimationById(u32 animId);

    void Undo();
    void Redo();
    const char* GetUndoStepName(u32 index);

    Node2D* CreateNode(Node2D* parent);
    void DeleteNode(Node2D* node); // 
    Node2D* SelectNode(Node2D* node); // Returns previous selection
    void RenameNode(Node2D* node, const char* newName);
    bool UpdateNodeTransform(Node2D* node, const vec2& pos, float rot, const vec2& scl);
    bool UpdateNodeTransformSingleF(Node2D* node, float val, TrackType type);
    bool UpdateNodeTransformSingleI(Node2D* node, i32 val, TrackType type);

    Animation* CreateAnimation(const char* optName = 0);
    void SelectAnimation(Animation* anim);
    void DeleteAnimation(Animation* anim);
    void RenameAnimation(Animation* anim, const char* name);
    void UpdateAnimation(Animation* anim, u32 frameCount, u32 frameRate, AnimationLoopMode loop);
    Track* AddTrack(Animation* anim, Node2D* target, TrackType type);
    void RemoveTrack(Track* track);
    void SetFrame(Track* t, u32 frame, InterpolationType interp);
    void AutoKeyFrameValue(Track* t, u32 frame, InterpolationType interp, u32 frame_value_as_u32);
    void ClearFrame(Track* t, u32 frame);
    void SetSelectedFrame(i32 frame);

    void RearrangeNode(Node2D& nodeToInsert, Node2D& parent, Node2D* prevSibling);

    //void UpdateSprite(Node2D* node, struct Resource* resource, float sourceX, float sourceY, float sourceW, float sourceH, float pivotX, float pivotY, float tintR, float tintG, float tintB, float tintA);
    void UpdateSprite(Node2D* node, struct Resource* resource);
    bool UpdateSprite(Node2D* node, i32 sX, i32 sY, i32 sW, i32 sH, f32 pX, f32 pY);
    bool UpdateSpriteVisibility(Node2D* node, bool visible, i32 sortIndex);
    bool UpdateSpriteTint(Node2D* node, float r, float g, float b, float a);

    void RequestResource(OnResourceLoaded onLoad, void* userData);
    Resource* LoadResource(const char* displayPath, void* resourceData, u32 bytes);
    void DestroyResource(Resource* resource);
    u32 GetReferenceCount(Resource* resource);

    Node2D* DepthFirst(Node2D* iter);
    Node2D* DepthFirstExpandedOnly(Node2D* iter);
    Node2D* Sorted(Node2D* iter);
    Node2D* SortedVisibleOnly(Node2D* iter);

    void EndOfFrame(bool autoKey);
    void NewDocument();

    Transform GetLocalTransform(Node2D* node);
    Transform GetWorldTransform(Node2D* node);
public:
    inline Track* GetSelectedTrack() {
        return selectedTrack;
    }

    void SelectTrack(Track* track) {
        if (track != 0) {
            PlatformAssert(timelineAnimation != 0, __LOCATION__);
        }
        selectedTrack = track;
    }

    inline int GetInterpolationIndex() {
        return selectedInterpolationType;
    }

    inline void SetInterpolationIndex(i32 val) {
        selectedInterpolationType = val;
        PlatformAssert(val >= 0, __LOCATION__);
        PlatformAssert(val <= 4, __LOCATION__);
    }

    inline InterpolationType GetSelectedInterpolationType() {
        if (selectedInterpolationType == 0) {
            return InterpolationType::Linear;
        }
        else if (selectedInterpolationType == 1) {
            return InterpolationType::Step;
        }
        else if (selectedInterpolationType == 2) {
            return InterpolationType::EaseIn;
        }
        else if (selectedInterpolationType == 3) {
            return InterpolationType::EaseOut;
        }
        else if (selectedInterpolationType == 4) {
            return InterpolationType::EaseInOut;
        }
        PlatformAssert(false, __LOCATION__);
        return InterpolationType::Linear;
    }

    inline Animation* GetTimelineAnimation() { return timelineAnimation; }

    inline void SelectTimeline(Animation* anim) {
        if (anim != timelineAnimation) {
            selectedTrack = 0;
        }
        timelineAnimation = anim;
    }

    inline Node2D* GetSelectedNode() {
        return selectedNode;
    }

    inline u32 GetNodeCount() {
        return numNodes;
    }

    inline i32 GetSelectedFrame() {
        return timelineSelectedFrame;
    }

    inline i32 GetLastSelectedFrame() {
        return timelineLastSelectedFrame;
    }

    inline u32 GetVisibleNodeCount() {
        u32 count = 0;
        Node2D* iter = DepthFirstExpandedOnly(0);
        iter = DepthFirstExpandedOnly(iter);
        while (iter != 0) {
            count += 1;
            iter = DepthFirstExpandedOnly(iter);
        }
        return count;
    }

    inline bool CanRedo() {
        return undoStackCurrent < undoStackTop;
    }

    inline bool CanUndo() {
        u32 undoStackBottom = undoStackTop < undoNumSteps ? 0 : undoStackTop - undoNumSteps;
        PlatformAssert(undoStackCurrent >= undoStackBottom, __LOCATION__);
        u32 current = undoStackCurrent - undoStackBottom;
        return current != 0;
    }

    inline u32 GetNumUndoSteps() {
        return (undoStackTop > undoNumSteps) ? undoNumSteps : undoStackTop;
    }

    inline u32 GetUndoStackCurrent()  {
        u32 undoStackBottom = undoStackTop < undoNumSteps? 0 : undoStackTop - undoNumSteps;
        PlatformAssert(undoStackCurrent >= undoStackBottom, __LOCATION__);
        return undoStackCurrent - undoStackBottom;
    }

    inline u32 GetMaxUndoSteps() {
        return undoNumSteps;
    }

    inline u32 GetNumResources() {
        return numResources;
    }

    inline Resource* GetSelectedResource() {
        return selectedResource;
    }

    inline Animation* GetSelectedAnimation() {
        return selectedAnimation;
    }

    inline Resource* ResourceIterator(Resource* iter) {
        if (iter == 0) {
            return resources;
        }

        return iter->next;
    }

    inline Track* TrackIterator(Track* iter) {
        if (iter == 0) {
            return selectedTrack;
        }

        return iter->next;
    }

    inline Track* TrackIterator(Animation* iter) {
        return iter->tracks;
    }

    inline Animation* AnimationIterator(Animation* anim) {
        if (anim == 0) {
            return allAnimations;
        }

        return anim->next;
    }

    inline u32 GetNumAnimations() {
        return numAnimations;
    }

    

    inline void SelectResource(Resource* resource) {
        selectedResource = resource;
    }

    inline void ClearUndoHistory() {
        ClearUndoHistory(0, undoStackTop);
        undoStackTop = 0;
        undoStackCurrent = 0;
    }

    u32 SaveInto(void* dest);
    inline u32 GetSaveSizeBytes() {
        u32 HEADER_SIZE = (8 * 4 + 6 * 8);
        u32 NODE_SIZE = (22 * 4 + 1 * 8);
        u32 RESOURCE_SIZE = (4 * 4 + 2 * 8);
        u32 ANIMATION_SIZE = (6 * 4 + 2 * 8);
        u32 TRACK_SIZE = (4 * 4 + 2 * 8);
        u32 FRAME_SIZE = (4 * 4);


        u32 totalTracks = 0;
        u32 totalFrames = 0;
        for (Animation* anim = allAnimations; anim != 0; anim = anim->next) {
            for (Track* track = anim->tracks; track != 0; track = track->next) {
                if (track->avtive) {
                    totalTracks += 1;
                    totalFrames += track->numKeyFrames;
                }
            }
        }

        u32 stringBytes = 0;
        for (Node2D* iter = DepthFirst(0); iter != 0; iter = DepthFirst(iter)) {
            stringBytes = iter->nameLength + 1;
            stringBytes += 5; // Potential padding
        }
        for (Resource* iter = resources; iter != 0; iter = iter->next) {
            PlatformAssert(iter->name != 0, __LOCATION__);
            u32 len = 0;
            for (; iter->name[len] != '\0'; ++len);
            stringBytes += len;
            stringBytes += 5; // Potential padding
        }
        for (Animation* iter = allAnimations; iter != 0; iter = iter->next) {
            PlatformAssert(iter->name != 0, __LOCATION__);
            u32 len = 0;
            for (; iter->name[len] != '\0'; ++len);
            stringBytes += len;
            stringBytes += 5; // Potential padding
        }

        u32 resourceDataBytes = 0;
        for (Resource* iter = resources; iter != 0; iter = iter->next) {
            resourceDataBytes += iter->size;
        }

        u32 stampSize = 16;

        return stampSize + HEADER_SIZE +
            (numNodes + 1) * NODE_SIZE +
            RESOURCE_SIZE * numResources +
            ANIMATION_SIZE * numAnimations +
            TRACK_SIZE * totalTracks +
            FRAME_SIZE * totalFrames +
            stringBytes + resourceDataBytes + stampSize + 128;
    }
};

#endif