#include "Document.h"
#include "../debt/stb_sprintf.h"
#include "../platform/memory.h"
#include "../platform/loader.h"
#include "../framework/draw2d.h"

#define NODE_TRANSFORM_DELTA 0.00001f

Node2D* Document::Sorted(Node2D* iter) {
    if (iter == 0) {
        Node2D* sortedList = DepthFirst(0);
        sortedList->sortedNext = 0;
        sortedList->sortedPrev = 0;
        
        sortedList = DepthFirst(sortedList); // Skip the root node
        if (sortedList != 0) {
            sortedList->sortedNext = 0;
            sortedList->sortedPrev = 0;
        }

        for (Node2D* iter = sortedList; iter != 0; iter = DepthFirst(iter)) {
            for (Node2D* insertPtr = sortedList; insertPtr != 0; insertPtr = insertPtr->sortedNext) {
                if (iter == insertPtr) {
                    continue;
                }
                if (insertPtr->sortIndex > iter->sortIndex) { // Add on left
                    if (insertPtr->sortedPrev != 0) {
                        insertPtr->sortedPrev->sortedNext = iter;
                    }
                    iter->sortedPrev = insertPtr->sortedPrev;

                    iter->sortedNext = insertPtr;
                    insertPtr->sortedPrev = iter;
                    break;
                }
                if (insertPtr->sortedNext == 0) { // Looking at tail
                    insertPtr->sortedNext = iter;
                    iter->sortedNext = 0;
                    iter->sortedPrev = insertPtr;
                    break;
                }
            }
            while (sortedList->sortedPrev != 0) {
                sortedList = sortedList->sortedPrev;
            }
        }

        return sortedList;
    }

    return iter->sortedNext;
}

Node2D* Document::SortedVisibleOnly(Node2D* iter) {
    if (iter == 0) {
        Node2D* rootIter = DepthFirst(0);
        rootIter->sortedNext = 0;
        rootIter->sortedPrev = 0;

        Node2D* sortedList = 0;

        while (rootIter != 0) {
            rootIter = DepthFirst(rootIter); 
            if (rootIter == 0) {
                break;
            }
            
            rootIter->sortedNext = 0;
            rootIter->sortedPrev = 0;

            if (rootIter->sprite.resourceUID != 0 && rootIter->sprite.visible) {
                sortedList = rootIter;
                break;
            }
        }

        for (Node2D* iter = sortedList; iter != 0; iter = DepthFirst(iter)) {
            for (Node2D* insertPtr = sortedList; insertPtr != 0; insertPtr = insertPtr->sortedNext) {
                if (iter == insertPtr) {
                    continue;
                }
                if (iter->sprite.resourceUID == 0 || !iter->sprite.visible) {
                    continue;
                }
                if (insertPtr->sortIndex > iter->sortIndex) { // Add on left
                    if (insertPtr->sortedPrev != 0) {
                        insertPtr->sortedPrev->sortedNext = iter;
                    }
                    iter->sortedPrev = insertPtr->sortedPrev;

                    iter->sortedNext = insertPtr;
                    insertPtr->sortedPrev = iter;
                    break;
                }
                if (insertPtr->sortedNext == 0) { // Looking at tail
                    insertPtr->sortedNext = iter;
                    iter->sortedNext = 0;
                    iter->sortedPrev = insertPtr;
                    break;
                }
            }
            while (sortedList->sortedPrev != 0) {
                sortedList = sortedList->sortedPrev;
            }
        }

        return sortedList;
    }

    return iter->sortedNext;
}


Node2D* Document::DepthFirst(Node2D* iter) {
    if (iter == 0) {
        return rootNode;
    }

    if (iter->firstChild) {
        iter = iter->firstChild;
    }
    else {
        while (iter->next == 0) {
            if (iter == rootNode) { // This should be iteration root, not document root
                return 0;
            }
            iter = iter->parent;
        }
        // This isn't 100% accurate, should stop if iter == root, but i don't have the iteration root
        iter = iter->next;
    }

    return iter;
}

Node2D* Document::DepthFirstExpandedOnly(Node2D* iter) {
    if (iter == 0) {
        return rootNode;
    }

    bool shouldDescend = iter->firstChild && iter->uiExpanded;
    if (iter == rootNode) {
        shouldDescend = true;
    }

    if (shouldDescend) {
        iter = iter->firstChild;
    }
    else {
        while (iter->next == 0) {
            if (iter == rootNode) { // This should be iteration root, not document root
                return 0;
            }
            iter = iter->parent;
        }
        // This isn't 100% accurate, should stop if iter == root, but i don't have the iteration root
        iter = iter->next;
    }

    return iter;
}

Animation* Document::FindAnimationById(u32 animId) {
    if (animId == 0) {
        return 0;
    }

    Animation* iter = allAnimations;
    while (iter != 0) {
        if (iter->uid == animId) {
            return iter;
        }
        iter = iter->next;
    }

    return 0;
}

Resource* Document::FindResourceById(u32 resourceId) {
    if (resourceId == 0) {
        return 0;
    }

    Resource* iter = resources;
    while (iter != 0) {
        if (iter->uid == resourceId) {
            return iter;
        }
        iter = iter->next;
    }

    return 0;
}


Node2D* Document::FindNodeById(u32 nodeId) { // Non intrusive, non extra memory DFS: https://stackoverflow.com/questions/5278580/non-recursive-depth-first-search-algorithm
    if (nodeId == 0) {
        return 0;
    }
    
    Node2D* itr = rootNode;
    bool traversing = true;
    while (traversing) {
        if (itr != rootNode) { // Process
            if (itr->uid == nodeId) {
                return itr;
            }
        }

        if (itr->firstChild) {
            itr = itr->firstChild;
        }
        else {
            while (itr->next == 0) {
                if (itr == rootNode) {
                    traversing = false;
                    break;
                }
                itr = itr->parent;
            }
            if (itr == rootNode) {
                traversing = false;
                break;
            }
            itr = itr->next;
        }
    }

    return 0;
}

static void _RecursivleyDestroyNodeIfItHasNoReferences(Node2D* node) {
    while (node->firstChild != 0) {
        Node2D* lastChild = GetLastChild(*node);
        PlatformAssert(lastChild->next == 0, __LOCATION__);
        _RecursivleyDestroyNodeIfItHasNoReferences(lastChild);
    }

    SetParent(0, *node);
    if (node->name != 0) {
        MemRelease(node->name);
        node->name = 0;
    }

    MemRelease(node);
}

bool RecursivleyDestroyNodeIfItHasNoReferences(Node2D* node) {
    if (node == 0) {
        return false;
    }

    if (node->refCount == 0) {
        ForEachNode(node, [](Node2D* childNode, void* userData) {
            PlatformAssert(childNode->refCount == 0, __LOCATION__);
            }, 0);

        _RecursivleyDestroyNodeIfItHasNoReferences(node);

        return true;
    }

    return false;
}

u32 WriteNode(Node2D* node, void* data, u32 writer) {
    u32* pu = (u32*)data;
    i32* ps = (i32*)data;
    f32* pf = (f32*)data;

    pu[writer++] = node->uid;
    pu[writer++] = node->parent != 0? node->parent->uid : 0;
    pu[writer++] = node->firstChild == 0 ? 0 : node->firstChild->uid;
    pu[writer++] = node->next == 0 ? 0 : node->next->uid;
    pu[writer++] = 0; // name offset
    pu[writer++] = 0; // name offset
    pf[writer++] = node->position.x;
    pf[writer++] = node->position.y;
    pf[writer++] = node->rotationAngles;
    pf[writer++] = node->scale.x;
    pf[writer++] = node->scale.y;
    pu[writer++] = node->sprite.resourceUID;
    pf[writer++] = node->sprite.tintR;
    pf[writer++] = node->sprite.tintG;
    pf[writer++] = node->sprite.tintB;
    pf[writer++] = node->sprite.tintA;
    pu[writer++] = node->sprite.visible;
    pf[writer++] = node->sprite.sourceX;
    pf[writer++] = node->sprite.sourceY;
    pf[writer++] = node->sprite.sourceW;
    pf[writer++] = node->sprite.sourceH;
    pf[writer++] = node->sprite.pivotX;
    pf[writer++] = node->sprite.pivotY;
    ps[writer++] = node->sortIndex;

    return writer;
}

u32 WriteResource(Resource* r, void* data, u32 writer) {
    u32* pu = (u32*)data;
    i32* ps = (i32*)data;
    f32* pf = (f32*)data;

    pu[writer++] = r->uid;
    pu[writer++] = r->size;
    pu[writer++] = 0; // name
    pu[writer++] = 0; // name
    pu[writer++] = r->width;
    pu[writer++] = r->height;
    pu[writer++] = 0; // data
    pu[writer++] = 0; // data

    return writer;
}

u32 WriteAnimation(Animation* a, void* data, u32 writer) {
    u32* pu = (u32*)data;
    i32* ps = (i32*)data;
    f32* pf = (f32*)data;

    u32 numTracks = 0;
    for (Track* t = a->tracks; t != 0; t = t->next) {
        if (t->avtive) {
            numTracks++;
        }
    }

    pu[writer++] = a->uid;
    pu[writer++] = (u32)(a->loop == AnimationLoopMode::Looping ? 1 : 0);
    pu[writer++] = a->frameRate;
    pu[writer++] = a->frameCount; 
    pu[writer++] = numTracks;
    pu[writer++] = 1234; // Padding
    pu[writer++] = 0; // name
    pu[writer++] = 0; // name
    pu[writer++] = 0; // first track
    pu[writer++] = 0; // first track

    return writer;
}

u32 WriteTrack(Track* t, void* data, u32 writer, u32 animUid) {
    u32* pu = (u32*)data;
    pu[writer++] = animUid;
    pu[writer++] = t->targetNode == 0 ? 0 : t->targetNode;
    pu[writer++] = (u32)t->targetProp;
    pu[writer++] = t->numKeyFrames;
    pu[writer++] = 0; // Frame offset
    pu[writer++] = 0; // It's a pointer
    pu[writer++] = 0; // Track offset
    pu[writer++] = 0; // It's a pointer
    return writer;
}

u32 WriteFrame(Frame* f, void* data, u32 writer, u32 index) {
    u32* pu = (u32*)data;
    pu[writer++] = index;
    pu[writer++] = f->uValue;
    pu[writer++] = 1234;
    pu[writer++] = (u32)f->interp;
    return writer;
}

u32 WriteString(void* data, const char* str, u32 writer) {
    u8* pu = (u8*)data;
    u32 char_writer = writer * 4;
    for (const char* iter = str; *iter != 0; ++iter) {
        pu[char_writer++] = *iter;
    }
    while (char_writer % 4 != 0) {
        pu[char_writer++] = 0;
    }
    return char_writer / 4;
}

u32 WriteTerminatedString(void* data, const char* str, u32 writer) {
    u8* pu = (u8*)data;
    u32 char_writer = writer * 4;
    for (const char* iter = str; *iter != 0; ++iter) {
        pu[char_writer++] = *iter;
    }

    // Write a null char
    pu[char_writer++] = 0;
    while (char_writer % 4 != 0) {
        pu[char_writer++] = 0;
    }

    return char_writer / 4;
}

u32 WriteNonPaddedString(void* data, const char* str, u32 writer) {
    u8* pu = (u8*)data;
    u32 char_writer = writer * 4;
    for (const char* iter = str; *iter != 0; ++iter) {
        pu[char_writer++] = *iter;
    }
    return char_writer - (writer * 4);
}

u32 WriteBlob(void* data, void* blob, u32 blobBytes, u32 writer) {
    u8* pu = (u8*)data;
    u32 char_writer = writer * 4;

    u8* blobIter = (u8*)blob;
    for (u32 i = 0; i < blobBytes; ++i) {
        pu[char_writer++] = *blobIter;
        blobIter += 1;
    }

    while (char_writer % 4 != 0) {
        pu[char_writer++] = 0;
    }

    return char_writer / 4;
}

u32 Document::SaveInto(void* dest) {
    u32 totalTracks = 0;
    u32 totalFrames = 0;
    u32 totalAnimations = 0;
    for (Animation* anim = allAnimations; anim != 0; anim = anim->next) {
        totalAnimations += 1;
        for (Track* track = anim->tracks; track != 0; track = track->next) {
            if (track->avtive) {
                totalTracks += 1;
                totalFrames += track->numKeyFrames;
            }
        }
    }

    u32* pu = (u32*)dest;
    i32* ps = (i32*)dest;

    u32 writer = 0;

    writer = WriteTerminatedString(dest, "Keyframe Studio", writer);

    pu[writer++] = numNodes + 1;
    pu[writer++] = numResources;
    pu[writer++] = totalAnimations;
    pu[writer++] = totalTracks;
    pu[writer++] = nodeUidGenerator;
    pu[writer++] = resourceUidGenerator;
    pu[writer++] = animationUidGenerator;
    pu[writer++] = 1234; // Padding

    u32 offsets = writer;
    writer += 2; // first node
    writer += 2; // first resource
    writer += 2; // first anim
    writer += 2; // first track
    writer += 2; // first frame
    writer += 2; // first resourceData

    pu[offsets++] = writer * 4; // Record first node offset
    pu[offsets++] = 0; // It's a pointer

    u32 first_node_name_offset = writer + 4;
    for (Node2D* iter = DepthFirst(0); iter != 0; iter = DepthFirst(iter)) {
        writer = WriteNode(iter, dest, writer);
    }

    pu[offsets++] = writer * 4; // Record first resource offset
    pu[offsets++] = 0; // It's a pointer

    u32 first_resource_data_offset = writer + 6;
    u32 first_resource_name_offset = writer + 2;
    for (Resource* iter = resources; iter != 0; iter = iter->next) {
        writer = WriteResource(iter, dest, writer);
    }

    pu[offsets++] = writer * 4; // Record first animation offset
    pu[offsets++] = 0; // It's a pointer

    u32 anim_track_offset = writer + 8;
    u32 first_anim_name_offset = writer + 6;
    for (Animation* iter = allAnimations; iter != 0; iter = iter->next) {
        writer = WriteAnimation(iter, dest, writer);
    }
    u32 animTrackStride = (6 * 4 + 2 * 8); // Sizeof anim, not track

    pu[offsets++] = writer * 4; // Record first track offset
    pu[offsets++] = 0; // It's a pointer

    u32 first_track_for_frames = 0;
    for (Animation* anim = allAnimations; anim != 0; anim = anim->next) {
        if (first_track_for_frames == 0) {
            first_track_for_frames = (writer + 4);
        }

        pu[anim_track_offset] = writer * 4;
        anim_track_offset += animTrackStride / 4;

        for (Track* track = anim->tracks; track != 0; track = track->next) {
            if (!track->avtive) {
                continue;
            }
            writer = WriteTrack(track, dest, writer, anim->uid);
            if (track->next != 0 && track->next->avtive) {
                pu[writer - 2] = writer * 4;
            }
            else{
                pu[writer - 2] = 0;
            }
        }
    }

    bool first_frame_written = false;
    u32 trackFrameStride = 4 + 4 + 4 + 4 + 8 + 8;
    for (Animation* anim = allAnimations; anim != 0; anim = anim->next) {
        for (Track* track = anim->tracks; track != 0; track = track->next) {
            if (!track->avtive) {
                continue;
            }

            pu[first_track_for_frames] = writer * 4;
            for (int i = 0; i < track->frameCount; ++i) {
                if (track->frames[i].key) {
                    if (!first_frame_written) {
                        pu[offsets++] = writer * 4; // Record first frame offset
                        pu[offsets++] = 0; // It's a pointer
                        first_frame_written = true;
                    }
                    writer = WriteFrame(&track->frames[i], dest, writer, i);
                }
            }
            
            first_track_for_frames += trackFrameStride / 4;
        }
    }

    if (!first_frame_written) {
        pu[offsets++] = 0;
        pu[offsets++] = 0;
    }

    // Write node names
    u32 node_name_stride = (22 * 4 + 1 * 8) / 4;
    for (Node2D* iter = DepthFirst(0); iter != 0; iter = DepthFirst(iter)) {
        pu[first_node_name_offset] = writer * 4;
        if (iter->name == 0) {
            pu[first_node_name_offset] = 0;
        }
        else {
            writer = WriteTerminatedString(dest, iter->name, writer);
        }

        first_node_name_offset += node_name_stride;
    }

    // Write resource names
    u32 resource_name_stride = (4 * 4 + 2 * 8) / 4;
    for (Resource* iter = resources; iter != 0; iter = iter->next) {
        if (iter->name != 0) {
            pu[first_resource_name_offset] = writer * 4;
            writer = WriteTerminatedString(dest, iter->name, writer);
            //PresentFile(iter->data, iter->size);
        }
        first_resource_name_offset += resource_name_stride;
    }

    // Write animation names
    u32 anim_name_stride = (6 * 4 + 2 * 8) / 4;
    for (Animation* iter = allAnimations; iter != 0; iter = iter->next) {
        if (iter->name != 0) {
            pu[first_anim_name_offset] = writer * 4;
            writer = WriteTerminatedString(dest, iter->name, writer);
        }
        first_anim_name_offset += anim_name_stride;
    }

    // Embed png data
    pu[offsets++] = writer * 4; // Record first file offset
    pu[offsets++] = 0; // It's a pointer
    u32 resource_data_stride = (4 * 4 + 2 * 8) / 4;
    for (Resource* iter = resources; iter != 0; iter = iter->next) {
        if (iter->data != 0) {
            pu[first_resource_data_offset] = writer * 4;
            writer = WriteBlob(dest, iter->data, iter->size, writer);
        }
        else {
            pu[first_resource_data_offset] = 0;
        }
        first_resource_data_offset += resource_data_stride;
    }

    u32 lastBytes = WriteNonPaddedString(dest, "\nKeyframe Studio", writer);

    return writer * 4 + lastBytes;
}


struct DocumentAction {
    virtual void Undo() = 0;
    virtual void Redo() = 0;
    virtual ~DocumentAction() = 0;
    virtual const char* Name() = 0;
};

DocumentAction::~DocumentAction() {  }

struct HierarchySelectAction : DocumentAction {
    Document* owner;
    u32 selectedId;
    u32 lastSelectedId;

    HierarchySelectAction(Document* doc, u32 newSelection, u32 oldSelection) {
        owner = doc;
        selectedId = newSelection;
        lastSelectedId = oldSelection;
    }

    ~HierarchySelectAction() {
    }

    void Redo() {
        Node2D* selection = owner->FindNodeById(selectedId);
        owner->selectedNode = selection;
    }

    void Undo() {
        Node2D* selection = owner->FindNodeById(lastSelectedId);
        owner->selectedNode = selection;
    }

    const char* Name()  {
        if (selectedId == 0) {
            return "Hierarchy deselect";
        }
        return "Hierarchy select";
    }
};

struct NullAction : DocumentAction {
    NullAction() { }

    void Redo() { }

    void Undo() { }

    ~NullAction() {
        PlatformAssert(false, __LOCATION__);
    }

    const char* Name() {
        return "Null action";
    }
};

struct SetFrameAction : DocumentAction {
    const char* actionName;
    Track* _track;
    u32 _frame;
    bool _wasKeyFrame;
    u32 _oldValue;
    u32 _newValue;

    InterpolationType oldInterp;
    InterpolationType newInterp;

    SetFrameAction(Document* d, Track* t, u32 f, InterpolationType interp, bool looping, const Frame& frm) {
        PlatformAssert(f < t->frameCount, __LOCATION__);
        _track = t;
        _frame = f;
        _wasKeyFrame = t->frames[f].key;
        _oldValue = t->frames[f].uValue;
        _newValue = frm.uValue;
        oldInterp = frm.interp;
        newInterp = interp;

        actionName = _wasKeyFrame ? "Update key frame" : "Create key frame";
    }

    void Redo() {
        Frame* frame = &_track->frames[_frame];
        if (!_wasKeyFrame) {
            frame->key = true;
            _track->numKeyFrames += 1;
        }
        frame->uValue = _newValue;
        frame->interp = newInterp;
    }

    void Undo() {
        Frame* frame = &_track->frames[_frame];
        if (!_wasKeyFrame) {
            frame->key = false;
            _track->numKeyFrames -= 1;
        }
        frame->uValue = _oldValue;
        frame->interp = oldInterp;
    }

    ~SetFrameAction() {
    }

    const char* Name() {
        return actionName;
    }
};

struct AutoKeyFrameAction : DocumentAction {
    const char* actionName;
    Track* _track;
    u32 _frame;
    bool _wasKeyFrame;
    u32 _oldValue;
    u32 _newValue;

    InterpolationType oldInterp;
    InterpolationType newInterp;

    AutoKeyFrameAction(Document* d, Track* t, u32 f, InterpolationType interp, const Frame& frm, u32 new_value_as_u32) {
        PlatformAssert(f < t->frameCount, __LOCATION__);
        _track = t;
        _frame = f;
        _wasKeyFrame = t->frames[f].key;
        _oldValue = t->frames[f].uValue;
        _newValue = new_value_as_u32;
        oldInterp = frm.interp;
        newInterp = interp;
    }

    void Redo() {
        Frame* frame = &_track->frames[_frame];
        if (!_wasKeyFrame) {
            frame->key = true;
            _track->numKeyFrames += 1;
        }
        frame->uValue = _newValue;
        frame->interp = newInterp;
    }

    void Undo() {
        Frame* frame = &_track->frames[_frame];
        if (!_wasKeyFrame) {
            frame->key = false;
            _track->numKeyFrames -= 1;
        }
        frame->uValue = _oldValue;
        frame->interp = oldInterp;
    }

    ~AutoKeyFrameAction() {
    }

    const char* Name() {
        return "Auto key frame";
    }
};

struct ClearFrameAction : DocumentAction {
    Document* owner;
    Track* track;
    u32 frame;

    ClearFrameAction(Document* d, Track* t, u32 f) { 
        track = t;
        owner = d;
        frame = f;
        PlatformAssert(track->frames[frame].key, __LOCATION__);
    }

    void Redo() { 
        track->frames[frame].key = false;
        track->numKeyFrames -= 1;
    }

    void Undo() { 
        track->frames[frame].key = true;
        track->numKeyFrames += 1;
    }

    ~ClearFrameAction() {
    }

    const char* Name() {
        return "Remove Keyframe";
    }
};

struct DeleteTrackAction : DocumentAction {
    Document* owner;
    Track* track;

    DeleteTrackAction(Document* doc, Track* t) {
        PlatformAssert(t->avtive, __LOCATION__);
        track = t;
        owner = doc;
    }

    void Redo() {
        track->avtive = false;
        for (i32 i = 0; i < track->frameCapacity; ++i) {
            track->frames[i].key = false;
        }
    }

    void Undo() {
        track->avtive = true;
    }

    ~DeleteTrackAction() {
    }

    const char* Name() {
        return "Remove track";
    }
};

struct ActivateTrackAction : DocumentAction {
    Document* owner;
    Track* track;

    ActivateTrackAction(Document* doc, Track* t) {
        track = t;
        owner = doc;
    }

    void Redo() {
        track->avtive = true;
        for (u32 i = 0; i < track->frameCount; ++i) {
            track->frames[i].key = false;
            track->frames[i].uValue = 0;
        }
    }

    void Undo() {
        track->avtive = false;
    }

    ~ActivateTrackAction() {
    }

    const char* Name() {
        return "Create track";
    }
};

struct CreateTrackAction : DocumentAction {
    Document* owner;
    Animation* animation;
    Node2D* node;
    TrackType trackType;

    Track* result;

    CreateTrackAction(Document* doc, Animation* anim, Node2D* target, TrackType type) {
        owner = doc;
        animation = anim;
        node = target;
        trackType = type;
        
        PlatformAssert(anim != 0, __LOCATION__);
        PlatformAssert(target != 0, __LOCATION__);

        u32 nodeId = target->uid;
        Track* tail = anim->tracks;
        if (tail != 0) {
            while (tail->next != 0) {
                if (tail->targetNode == nodeId && tail->targetProp == type) {
                    result = tail;
                    return;
                }
                tail = tail->next;
            }
            PlatformAssert(tail != 0, __LOCATION__);
            PlatformAssert(tail->next == 0, __LOCATION__);
            if (tail->targetNode == nodeId && tail->targetProp == type) {
                result = tail;
                return;
            }
        }

        result = (Track*)MemAlloc(sizeof(Track));
        result->targetNode = target->uid;
        result->targetProp = type;
        result->avtive = true;
        result->frameCapacity = result->frameCount = anim->frameCount;
        result->numKeyFrames = 0;
        result->prev = result->next = 0;

        result->frames = (Frame*)MemAlloc(sizeof(Frame) * anim->frameCount);
        MemClear(result->frames, sizeof(Frame) * anim->frameCount);
        
        Frame* prev = 0;
        for (u32 i = 0; i < anim->frameCount; ++i) {
            result->frames[i].key = false;
            result->frames[i].uValue = false;
            result->frames[i].interp = InterpolationType::Linear;
            if (type == TrackType::SpriteVisibility || type == TrackType::SortIndex) {
                result->frames[i].interp = InterpolationType::Step;
            }
        }

        if (tail == 0) {
            anim->tracks = result;
        }
        else {
            tail->next = result;
            result->prev = tail;
        }
    }

    void Redo() { 
        animation->numTracks += 1;
        result->avtive = true;
    }

    void Undo() {
        animation->numTracks -= 1;
        result->avtive = false;
    }

    ~CreateTrackAction() {
        // Nothing to clean up, when an animation is destroyed, it will
        // take all of its frames and tracks with it
    }

    const char* Name() {
        return "Create new track";
    }
};

struct UpdateSpriteTintAction : DocumentAction {
    Document* doc;
    u32 nodeID;

    float newR;
    float newG;
    float newB;
    float newA;

    float oldR;
    float oldG;
    float oldB;
    float oldA;

    UpdateSpriteTintAction(Document* owner, Node2D* node, float r, float g, float b, float a) {
        doc = owner;
        nodeID = node->uid;

        oldR = node->sprite.tintR;
        oldG = node->sprite.tintG;
        oldB = node->sprite.tintB;
        oldA = node->sprite.tintA;

        newR = r;
        newG = g;
        newB = b;
        newA = a;
    }

    void Redo() { 
        Node2D* node = doc->FindNodeById(nodeID);
        PlatformAssert(node != 0, __LOCATION__);
        node->sprite.tintR = newR;
        node->sprite.tintG = newG;
        node->sprite.tintB = newB;
        node->sprite.tintA = newA;
    }

    void Undo() { 
        Node2D* node = doc->FindNodeById(nodeID);
        PlatformAssert(node != 0, __LOCATION__);
        node->sprite.tintR = oldR;
        node->sprite.tintG = oldG;
        node->sprite.tintB = oldB;
        node->sprite.tintA = oldA;
    }

    ~UpdateSpriteTintAction() {
    }

    const char* Name() {
        return "Update sprite tint";
    }
};

struct ChangeNodeVisibilityAction : DocumentAction {
    Document* owner;
    u32 nodeId;
    bool visible;

    i32 oldSort;
    i32 newSort;

    ChangeNodeVisibilityAction(Document* doc, Node2D* node, bool vis, i32 sortIndex) { 
        owner = doc;
        nodeId = node->uid;
        visible = vis;
        oldSort = node->sortIndex;
        newSort = sortIndex;
    }

    void Redo() { 
        Node2D* node = owner->FindNodeById(nodeId);
        node->sprite.visible = visible;
        node->sortIndex = newSort;
    }

    void Undo() {
        Node2D* node = owner->FindNodeById(nodeId);
        node->sprite.visible = !visible;
        node->sortIndex = oldSort;
    }

    ~ChangeNodeVisibilityAction() {
    }

    const char* Name() {
        return "Toggle node visibility";
    }
};

struct UpdateResourceRectAction : DocumentAction {
    u32 nodeId;
    Document* owner;

    i32 sourceX;
    i32 oldSourceX;

    i32 sourceY;
    i32 oldSourceY;

    i32 sourceW;
    i32 oldSourceW;

    i32 sourceH;
    i32 oldSourceH;

    f32 pivotX;
    f32 oldPivotX;

    f32 pivotY;
    f32 oldPivotY;

    UpdateResourceRectAction(Document* doc, Node2D* node, i32 sx, i32 sy, i32 sw, i32 sh, f32 px, f32 py) { 
        owner = doc;
        nodeId = node->uid;

        oldSourceX = node->sprite.sourceX;
        oldSourceY = node->sprite.sourceY;
        oldSourceW = node->sprite.sourceW;
        oldSourceH = node->sprite.sourceH;
        oldPivotX = node->sprite.pivotX;
        oldPivotY = node->sprite.pivotY;

        sourceX = sx;
        sourceY = sy;
        sourceW = sw;
        sourceH = sh;
        pivotX = px;
        pivotY = py;
    }

    void Redo() { 
        Node2D* node = owner->FindNodeById(nodeId);
        PlatformAssert(node != 0, __LOCATION__);

        node->sprite.sourceX = sourceX;
        node->sprite.sourceY = sourceY;
        node->sprite.sourceW = sourceW;
        node->sprite.sourceH = sourceH;
        node->sprite.pivotX = pivotX;
        node->sprite.pivotY = pivotY;
    }

    void Undo() {
        Node2D* node = owner->FindNodeById(nodeId);
        PlatformAssert(node != 0, __LOCATION__);

        node->sprite.sourceX = oldSourceX;
        node->sprite.sourceY = oldSourceY;
        node->sprite.sourceW = oldSourceW;
        node->sprite.sourceH = oldSourceH;
        node->sprite.pivotX = oldPivotX;
        node->sprite.pivotY = oldPivotY;
    }

    ~UpdateResourceRectAction() {
    }

    const char* Name() {
        return "Update resource view";
    }
};

struct SetNodeResourceAction : DocumentAction {
    u32 nodeId;
    u32 resourceId;
    u32 oldResourceId;
    Document* owner;

    SetNodeResourceAction(Document* d, Node2D* n, Resource* r) { 
        if (r == 0) {
            resourceId = 0;
        }
        else {
            resourceId = r->uid;
        }

        if (n == 0) {
            nodeId = 0;
        }
        else {
            nodeId = n->uid;
        }
        oldResourceId = n->sprite.resourceUID;
        owner = d;
    }

    void Redo() {
        Node2D* node = owner->FindNodeById(nodeId);
        Resource* resource = owner->FindResourceById(resourceId);

        PlatformAssert(node != 0, __LOCATION__);
        if (node != 0) {
            if (resource != 0) {
                node->sprite.resourceUID = resource->uid;
            }
            else {
                node->sprite.resourceUID = 0;
            }
        }
    }

    void Undo() { 
        Node2D* node = owner->FindNodeById(nodeId);
        Resource* resource = owner->FindResourceById(oldResourceId);
    
        PlatformAssert(node != 0, __LOCATION__);
        if (node != 0) {
            if (resource != 0) {
                node->sprite.resourceUID = resource->uid;
            }
            else {
                node->sprite.resourceUID = 0;
            }
        }
    }

    ~SetNodeResourceAction() {
    }

    const char* Name() {
        return "Update node resource";
    }
};

struct RearrangeNodeAction : DocumentAction {
    Document* doc;
    
    u32 targetId;

    u32 newParentId;
    u32 newPrevId;

    u32 oldParentId;
    u32 oldPrevId;

    RearrangeNodeAction(Document* d, Node2D* target, Node2D* parent, Node2D* prevNode) {
        PlatformAssert(target != 0, __LOCATION__);
        PlatformAssert(parent != 0, __LOCATION__);

        doc = d;
        targetId = target->uid;
        oldParentId = 0;
        oldPrevId = 0;
        if (target->parent != 0) {
            oldParentId = target->parent->uid;

            Node2D* iter = target->parent->firstChild;
            Node2D* prev = 0;
            while (iter != 0) {
                if (iter == target) {
                    break;
                }
                prev = iter;
                iter = iter->next;
            }

            if (prev != 0) {
                oldPrevId = prev->uid;
            }
        }

        newParentId = parent->uid;
        newPrevId = 0;
        if (prevNode != 0) {
            newPrevId = prevNode->uid;
            PlatformAssert(prevNode->parent = parent, __LOCATION__);
        }
    }

    void Redo() {
        Node2D* newParent = doc->FindNodeById(newParentId);
        if (newParent == 0) {
            newParent = doc->rootNode;
        }
        Node2D* target = doc->FindNodeById(targetId);
        SetParent(newParent, *target, newPrevId);
    }

    void Undo() {
        Node2D* oldParent = doc->FindNodeById(oldParentId);
        if (oldParent == 0) {
            oldParent = doc->rootNode;
        }
        Node2D* target = doc->FindNodeById(targetId);
        SetParent(oldParent, *target, oldPrevId);
    }

    ~RearrangeNodeAction() {  }

    const char* Name() {
        return "Hierarchy Reorder";
    }
};

struct CreateNodeAction : DocumentAction {
    Document* owner;
    u32 parentUID;
    Node2D* createdInstance;
    bool undone;

    CreateNodeAction(Document* doc, u32 parent) {
        owner = doc;
        parentUID = parent;
        undone = false;

        Node2D* _parent = owner->FindNodeById(parentUID);
        if (_parent == 0) {
            _parent = owner->rootNode;
        }

        createdInstance = (Node2D*)MemAlloc(sizeof(Node2D));
        MemClear(createdInstance, sizeof(Node2D));
        createdInstance->uid = ++owner->nodeUidGenerator;
        createdInstance->sprite.visible = true;
        createdInstance->sprite.tintR = 1.0f;
        createdInstance->sprite.tintG = 1.0f;
        createdInstance->sprite.tintB = 1.0f;
        createdInstance->sprite.tintA = 1.0f;

        createdInstance->name = (char*)MemAlloc(NODE2D_DEFAULT_NAME_CAPACITY + 1);
        MemClear(createdInstance->name, NODE2D_DEFAULT_NAME_CAPACITY + 1);
        createdInstance->nameCapacity = NODE2D_DEFAULT_NAME_CAPACITY;
        stbsp_snprintf(createdInstance->name, createdInstance->nameCapacity, "Node %d", createdInstance->uid);

        createdInstance->refCount += 1;  // +1 for the create node action
        createdInstance->scale = vec2(1, 1);
        createdInstance->uiVisible = true;
        createdInstance->uiExpanded = true;

        SetParent(_parent, *createdInstance);
        SetName(*createdInstance, createdInstance->name);
    }

    ~CreateNodeAction() {
        createdInstance->refCount -= 1;
        if (undone) {
            PlatformAssert(createdInstance->firstChild == 0, __LOCATION__);
        }
        RecursivleyDestroyNodeIfItHasNoReferences(createdInstance);

        owner = 0;
    }

    void Undo() {
        undone = true;
        owner->numNodes -= 1;
        createdInstance->refCount -= 1;  // -1 since it's deleted
        PlatformAssert(createdInstance->refCount != 0, __LOCATION__); // This struct has to have +1 ref!

        if (createdInstance->parent != 0) {
            RemoveChild(*createdInstance->parent, *createdInstance);
        }

        PlatformAssert(createdInstance->firstChild == 0, __LOCATION__);
    }

    void Redo() {
        undone = false;
        owner->numNodes += 1;
        Node2D* parent = owner->FindNodeById(parentUID);
        if (parent == 0) {
            parent = owner->rootNode;
        }
        createdInstance->refCount += 1;  // +1 since it's in document

        SetParent(parent, *createdInstance);
    }

    const char* Name() {
        return "Hierarchy create node";
    }

    Node2D* RedoResult() {
        return createdInstance;
    }
};

struct DeleteNodeAction : DocumentAction {
    Document* owner;
    Node2D* deletedNode;
    u32 parentId;
    u32 lastSibling;

    DeleteNodeAction(Document* doc, Node2D* target) {
        PlatformAssert(target != 0, __LOCATION__);
        owner = doc;
        parentId = 0;
        deletedNode = target;


        // Increase the reference cound for every node that the delete action touches
        ForEachNode(deletedNode, [](Node2D* node, void* userData) { // Not sure if i should do this just to the root, or all nodes
            node->refCount += 1;
            PlatformAssert(node->refCount > 0, __LOCATION__);
        }, 0);

        lastSibling = 0;
        if (target->parent != 0) {
            parentId = target->parent->uid;
            Node2D* iter = target->parent->firstChild;
            while (iter != 0) {
                if (iter->uid == target->uid) {
                    break;
                }
                lastSibling = iter->uid;
                iter = iter->next;
            }
        }
    }

    ~DeleteNodeAction() {
        // Decrease the reference count, since the delete action is gone
        ForEachNode(deletedNode, [](Node2D* node, void* userData) {
            node->refCount -= 1;
        }, 0);

        RecursivleyDestroyNodeIfItHasNoReferences(deletedNode);
    }

    void Undo() {
        Node2D* parent = owner->FindNodeById(parentId);
        if (parent == 0) {
            parent = owner->rootNode;
        }
        SetParent(parent, *deletedNode, lastSibling);
        owner->selectedNode = deletedNode;

        // Increase the reference cound for every node when it is added to the hierarchy
        u32 nodesToRemove = 0;
        ForEachNode(deletedNode, [](Node2D* node, void* userData) {
            u32* nodesToRemove = (u32*)userData;
            *nodesToRemove += 1;
            node->refCount += 1;
            PlatformAssert(node->refCount > 0, __LOCATION__);
        }, &nodesToRemove);
        owner->numNodes += nodesToRemove;

    }

    void Redo() {
        PlatformAssert (owner->selectedNode->uid == deletedNode->uid, __LOCATION__);
        SetParent(0, *deletedNode);
        owner->selectedNode = 0;

        // Decrease the reference cound for every node when it is removed from the hierarchy
        u32 nodesToRemove = 0;
        ForEachNode(deletedNode, [](Node2D* node, void* userData) {
            u32* nodesToRemove = (u32*)userData;
            *nodesToRemove += 1; 
            node->refCount -= 1;
            PlatformAssert(node->refCount > 0, __LOCATION__);
        }, & nodesToRemove);
        owner->numNodes -= nodesToRemove;
    }

    const char* Name() {
        return "Hierarchy delete node";
    }
};

struct RenameAnimationAction : DocumentAction {
    Document* owner;
    u32 animID;
    char* oldName;
    u32 oldNameLen;
    char* newName;
    u32 newNameLen;

    RenameAnimationAction(Document* doc, Animation* anim, const char* name, u32 nameLen) { 
        PlatformAssert(anim != 0, __LOCATION__);
        PlatformAssert(name != 0, __LOCATION__);

        owner = doc;
        animID = anim->uid;

        oldName = 0;
        oldNameLen = 0;
        if (anim->name != 0) {
            oldNameLen = nameLen;
            oldName = (char*)MemAlloc(oldNameLen + 1);
            for (u32 i = 0; i < oldNameLen; ++i) {
                oldName[i] = anim->name[i];
            }
            oldName[oldNameLen] = 0;
        }

        newName = 0;
        newNameLen = 0;
        if (name) {
            u32 len = 0;
            const char* iter = name;
            while (*iter != 0) {
                len++;
                iter++;
            }

            newNameLen = len;
            newName = (char*)MemAlloc(newNameLen + 1);
            for (u32 i = 0; i < newNameLen; ++i) {
                newName[i] = name[i];
            }
            newName[newNameLen] = 0;
        }
    }

    void Redo() {
        Animation* anim = owner->FindAnimationById(animID);
        PlatformAssert(anim != 0, __LOCATION__);

        anim->name = (char*)MemRealloc(anim->name, newNameLen + 1);
        for (u32 i = 0; i < newNameLen; ++i) {
            anim->name[i] = newName[i];
        }
        anim->name[newNameLen] = 0;
    }

    void Undo() { 
        Animation* anim = owner->FindAnimationById(animID);
        PlatformAssert(anim != 0, __LOCATION__);

        anim->name = (char*)MemRealloc(anim->name, oldNameLen + 1);
        for (u32 i = 0; i < oldNameLen; ++i) {
            anim->name[i] = oldName[i];
        }
        anim->name[oldNameLen] = 0;
    }

    ~RenameAnimationAction() {
        if (oldName != 0) {
            MemRelease(oldName);
        }
        if (newName != 0) {
            MemRelease(newName);
        }
    }

    const char* Name() {
        return "Rename animation";
    }
};

struct RenameNodeAction : DocumentAction {
    Document* owner;
    u32 nodeId;
    char* oldName;
    u32 oldNameLen;
    char* newName;
    u32 newNameLen;

    RenameNodeAction(Document* doc, Node2D* node, const char* name) {
        PlatformAssert(node != 0, __LOCATION__);
        PlatformAssert(name != 0, __LOCATION__);

        owner = doc;
        nodeId = node->uid;

        oldName = 0;
        oldNameLen = 0;
        if (node->name != 0) {
            oldNameLen = node->nameLength;
            oldName = (char*)MemAlloc(oldNameLen + 1);
            for (u32 i = 0; i < oldNameLen; ++i) {
                oldName[i] = node->name[i];
            }
            oldName[oldNameLen] = 0;
        }

        newName = 0;
        newNameLen = 0;
        if (name) {
            u32 len = 0;
            const char* iter = name;
            while (*iter != 0) {
                len++;
                iter++;
            }

            newNameLen = len;
            newName = (char*)MemAlloc(newNameLen + 1);
            for (u32 i = 0; i < newNameLen; ++i) {
                newName[i] = name[i];
            }
            newName[newNameLen] = 0;
        }
    }

    void Redo() {
        Node2D* node = owner->FindNodeById(nodeId);
        PlatformAssert(node != 0, __LOCATION__);

        if (node->nameCapacity < newNameLen + 1) {
            node->nameCapacity = newNameLen + 1;
            node->name = (char*)MemRealloc(node->name, newNameLen + 1);
        }

        node->nameLength = newNameLen;
        for (u32 i = 0; i < newNameLen; ++i) {
            node->name[i] = newName[i];
        }
        node->name[newNameLen] = 0;
    }

    void Undo() {
        Node2D* node = owner->FindNodeById(nodeId);
        PlatformAssert(node != 0, __LOCATION__);

        if (node->nameCapacity < oldNameLen + 1) {
            node->nameCapacity = oldNameLen + 1;
            node->name = (char*)MemRealloc(node->name, oldNameLen + 1);
        }

        node->nameLength = oldNameLen;
        for (u32 i = 0; i < oldNameLen; ++i) {
            node->name[i] = oldName[i];
        }
        node->name[oldNameLen] = 0;
    }

    ~RenameNodeAction() {
        if (oldName != 0) {
            MemRelease(oldName);
        }
        if (newName != 0) {
            MemRelease(newName);
        }
    }

    const char* Name() {
        return "Rename node";
    }
};

struct EditTransformAction : DocumentAction {
    Document* owner;
    u32 nodeId;

    vec2 olsPos;
    vec2 newPos;

    float oldRot;
    float newRot;

    vec2 oldScale;
    vec2 newScale;

    EditTransformAction(Document* doc, Node2D* node, const vec2& nPos, float nRot, const vec2& nScale) {
        owner = doc;
        nodeId = node->uid;

        olsPos = node->position;
        oldRot = node->rotationAngles;
        oldScale = node->scale;

        newPos = nPos;
        newRot = nRot;
        newScale = nScale;
    }

    void Redo() {
        Node2D* node = owner->FindNodeById(nodeId);
        PlatformAssert(node != 0, __LOCATION__);

        node->position = newPos;
        node->rotationAngles = newRot;
        node->scale = newScale;
    }

    void Undo() {
        Node2D* node = owner->FindNodeById(nodeId);
        PlatformAssert(node != 0, __LOCATION__);

        node->position = olsPos;
        node->rotationAngles = oldRot;
        node->scale = oldScale;
    }

    ~EditTransformAction() {
        // Nothing to do
    }

    const char* Name() {
        return "Edit node transform";
    }
};

struct CreateAnimationAction : DocumentAction {
    Document* owner;
    Animation* anim;
    u32 animID;

    inline Animation* RedoResult() {
        return anim;
    }

    inline CreateAnimationAction(Document* doc, const char* optionalName) { 
        owner = doc;
        anim = (Animation*)MemAlloc(sizeof(Animation));
        MemClear(anim, sizeof(Animation));

        doc->numAnimations += 1;

        anim->uid = animID = ++doc->animationUidGenerator;
        anim->frameRate = 30; // 2 second default 
        anim->frameCount = 60;
        anim->loop = AnimationLoopMode::Looping;

        // Name the animation
        if (optionalName == 0) {
            anim->name = (char*)MemAlloc(20);
            stbsp_snprintf(anim->name, 20, "Animation %d", anim->uid);
        }
        else {
            u32 len = 0; // strlen(name)
            for (const char* n = optionalName; *n != 0; ++n, ++len);

            anim->name = (char*)MemAlloc(len + 1);
            MemCopy(anim->name, optionalName, len);
            anim->name[len] = 0;
        }

        
        anim->refCount = 1; // Because the action is holding onto it

    }

    void Redo() {
        // Add to document
        if (owner->allAnimations == 0) {
            owner->allAnimations = anim;
            anim->prev = anim->next = 0;
        }
        else {
            Animation* tail = owner->allAnimations;
            while (tail->next != 0) {
                tail = tail->next;
            }

            PlatformAssert(tail->next == 0, __LOCATION__);
            tail->next = anim;
            anim->prev = tail;
            anim->next = 0;
        }

        anim->refCount += 1; // Because it's in the document now
    }

    void Undo() {
        if (owner->allAnimations == anim) {
            PlatformAssert(anim->prev == 0, __LOCATION__);
            PlatformAssert(owner->allAnimations->prev == 0, __LOCATION__);

            if (owner->allAnimations->next != 0) {
                owner->allAnimations->next->prev = 0;
            }
            owner->allAnimations = owner->allAnimations->next;
        }
        else {
            if (anim->next != 0) {
                anim->next->prev = anim->prev;
            }
            if (anim->prev != 0) {
                anim->prev->next = anim->next;
            }
        }
        anim->prev = anim->next = 0;


        anim->refCount -= 1; // Because it's no longer in the document
        // ref count here can never be 0 because the constructor added 1
    }

    ~CreateAnimationAction() {
        anim->refCount -= 1; // Because it's no longer in the document
        if (anim->refCount <= 0) {
            anim->ReleaseMemory();
            MemRelease(anim);
            anim = 0;
        }
    }

    const char* Name() {
        return "Create Animation";
    }
};

struct DeleteAnimationAction : DocumentAction {
    Document* owner;
    Animation* anim;
    u32 animID;

    DeleteAnimationAction(Document* doc, Animation* animation) { 
        owner = doc;
        anim = animation;
        animID = animation->uid;

        anim->refCount += 1; // Because the action is holding onto it
    }

    void Redo() { 
        PlatformAssert(anim != 0, __LOCATION__);

        if (owner->allAnimations == anim) {
            PlatformAssert(anim->prev == 0, __LOCATION__);
            PlatformAssert(owner->allAnimations->prev == 0, __LOCATION__);

            if (owner->allAnimations->next != 0) {
                owner->allAnimations->next->prev = 0;
            }
            owner->allAnimations = owner->allAnimations->next;
        }
        else {
            if (anim->next != 0) {
                anim->next->prev = anim->prev;
            }
            if (anim->prev != 0) {
                anim->prev->next = anim->next;
            }
        }
        anim->prev = anim->next = 0;

        anim->refCount -= 1; // Since it's no longer in the documnet
    }

    void Undo() {
        if (owner->allAnimations == 0) {
            owner->allAnimations = anim;
            anim->prev = anim->next = 0;
        }
        else {
            Animation* tail = owner->allAnimations;
            while (tail->next != 0) {
                tail = tail->next;
            }

            PlatformAssert(tail->next == 0, __LOCATION__);
            tail->next = anim;
            anim->prev = tail;
            anim->next = 0;
        }

        anim->refCount += 1; // Because it's in the document now
    }

    ~DeleteAnimationAction() {
        anim->refCount -= 1; // Because it's no longer in the document
        if (anim->refCount <= 0) {
            anim->ReleaseMemory();
            MemRelease(anim);
            anim = 0;
        }
    }

    const char* Name() {
        return "Delete animation";
    }
};

struct UpdateAnimationAction : DocumentAction {
    Document* owner;
    u32 animId;

    u32 oldFrameCount;
    u32 oldFrameRate;
    AnimationLoopMode oldLoopMode;

    u32 newFrameCount;
    u32 newFrameRate;
    AnimationLoopMode newLoopMode;

    inline void MakeSureAnimationHasEnoughFrames(Animation* anim) {
        u32 frameCount = newFrameCount;
        if (oldFrameCount > newFrameCount) {
            frameCount = oldFrameCount;
        }

        for (Track* iter = anim->tracks; iter != 0; iter = iter->next) {
            iter->Resize(frameCount);
        }
    }

    UpdateAnimationAction(Document* doc, Animation* anim, u32 frameCount, u32 frameRAte, AnimationLoopMode loopMode) { 
        owner = doc;
        animId = anim->uid;

        newFrameCount = frameCount;
        newFrameRate = frameRAte;
        newLoopMode = loopMode;

        oldFrameCount = anim->frameCount;
        oldFrameRate = anim->frameRate;
        oldLoopMode = anim->loop;
    }

    void Redo() { 
        Animation* anim = owner->FindAnimationById(animId);
        PlatformAssert(anim != 0, __LOCATION__);

        MakeSureAnimationHasEnoughFrames(anim);
        anim->frameCount = newFrameCount;
        anim->frameRate = newFrameRate;
        anim->loop = newLoopMode;
    }

    void Undo() {
        Animation* anim = owner->FindAnimationById(animId);
        PlatformAssert(anim != 0, __LOCATION__);

        MakeSureAnimationHasEnoughFrames(anim);
        anim->frameCount = oldFrameCount;
        anim->frameRate = oldFrameRate;
        anim->loop = oldLoopMode;
    }

    ~UpdateAnimationAction() { }

    const char* Name() {
        return "Update animation";
    }
};

struct StaticActionSize {
    union {
        NullAction nullAction;
        HierarchySelectAction hierarchySelection;
        CreateNodeAction createNode;
        DeleteNodeAction deleteNode;
        RearrangeNodeAction arrangeNode;
        RenameNodeAction renameNode;
        EditTransformAction editTransform;
        SetNodeResourceAction setNodeResource;
        UpdateResourceRectAction updateResource;
        ChangeNodeVisibilityAction changeNodeVisibility;
        UpdateSpriteTintAction updateSpriteTint;
        CreateAnimationAction createAnimation;
        DeleteAnimationAction deleteAnimation;
        RenameAnimationAction renameAnimation;
        UpdateAnimationAction updateAnimations;
        CreateTrackAction createTrack;
        ClearFrameAction clearFrame;
        SetFrameAction setFrame;
        DeleteTrackAction deleteTrack;
        ActivateTrackAction activateTrack;
        AutoKeyFrameAction autoKeyAction;
    };
};

StaticActionSize* GrabNextUndoSlot(Document* d);

void Document::DeleteNode(Node2D* node) {
    StaticActionSize* action = GrabNextUndoSlot(this);
    DeleteNodeAction* dAction = new (action)DeleteNodeAction(this, node);
    dAction->Redo();
}

void Document::DeleteAnimation(Animation* anim) {
    if (selectedAnimation == anim) {
        selectedAnimation = 0;
    }
    if (timelineAnimation == anim) {
        timelineAnimation = 0;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    DeleteAnimationAction* dAction = new (action)DeleteAnimationAction(this, anim);
    dAction->Redo();
}

void Document::SelectAnimation(Animation* anim) {
    selectedAnimation = anim;
    timelineSelectedFrame = -1;
}

Animation* Document::CreateAnimation(const char* name) {
    StaticActionSize* action = GrabNextUndoSlot(this);
    CreateAnimationAction* cAction = new (action)CreateAnimationAction(this, name);
    cAction->Redo();
    return cAction->RedoResult();
}

Node2D* Document::CreateNode(Node2D* parent) {
    if (parent == 0) {
        parent = rootNode;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    CreateNodeAction* cAction = new (action)CreateNodeAction(this, parent->uid);
    cAction->Redo();
    return cAction->RedoResult();
}

namespace Document_Internal {
    struct RequestResourceUserData {
        Document* owner;
        void* userData;
        OnResourceLoaded callback;
    };
}

u32 Document::GetReferenceCount(Resource* resource) {
    u32 count = 0;
    for (Node2D* iter = DepthFirst(0); iter != 0; iter = DepthFirst(iter)) {
        if (iter->sprite.resourceUID == resource->uid) {
            count += 1;
        }
    }
    return count;
}

void Document::DestroyResource(Resource* resource) {
    PlatformAssert(numResources != 0, __LOCATION__);

    if (resource != 0) {
        for (Node2D* iter = DepthFirst(0); iter != 0; iter = DepthFirst(iter)) {
            if (iter->sprite.resourceUID == resource->uid) {
                iter->sprite.resourceUID = 0;
            }
        }

        if (resource->name != 0) {
            MemRelease(resource->name);
            resource->name = 0;
        }
        if (resource->data != 0) {
            MemRelease(resource->data);
        }
        Draw2D::DestroyImage(resource->image);
        resource->image = 0;
        MemRelease(resource);

        if (selectedResource == resource) {
            selectedResource = 0;
        }
        if (resources == resource) {
            resources = resources->next;
        }
        if (resource->prev) {
            resource->prev->next = resource->next;
        }
        if (resource->next) {
            resource->next->prev = resource->prev;
        }

        numResources -= 1;
    }
}

Resource* Document::LoadResource(const char* path, void* data, u32 bytes) {
    if (data == 0 || bytes == 0) {
        return 0;
    }
    
    Resource* result = (Resource*)MemAlloc(sizeof(Resource));
   
    {
        if (path != 0) {
            const char* lastSlash = path;
            const char* iter = path;
            u32 pathLen = 0;
            while (*iter != '\0') {
                pathLen += 1;
                if (*iter == '/' || *iter == '\\') {
                    lastSlash = ++iter;
                    pathLen = 0;
                }
                else {
                    ++iter;
                }
            }

            result->name = (char*)MemAlloc(pathLen + 1);
            int i = 0;
            for (iter = lastSlash; *iter != '\0'; ++iter) {
                result->name[i++] = *iter;
            }
            result->name[i] = 0;
            result->nameLen = pathLen;
        }
        else {
            int uid = resourceUidGenerator + 1;
            result->name = (char*)MemAlloc(16);
            stbsp_snprintf(result->name, 16, "Image %d", uid);
            result->nameLen = 16;
        }

        result->data = MemAlloc(bytes); 
        MemCopy(result->data, data, bytes);
        result->size = bytes;
        result->image = Draw2D::LoadImage(data, bytes, Draw2D::Interpolation::Linear);
        result->filter = Draw2D::Interpolation::Linear;
        result->width = Draw2D::GetImageSize(result->image).w;
        result->height = Draw2D::GetImageSize(result->image).h;

        result->prev = result->next = 0;
        if (resources == 0) {
            result->prev = result->next = 0;
            resources = result;
        }
        else {
            Resource* tail = resources;
            while (tail->next != 0) {
                tail = tail->next;
            }
            PlatformAssert(tail->next == 0, __LOCATION__);
            tail->next = result;
            result->prev = tail;
        }
        numResources += 1;
        result->uid = ++resourceUidGenerator;
    }

    return result;
}


void Document::RequestResource(OnResourceLoaded onLoad, void* userData) {
    Document_Internal::RequestResourceUserData* userPointer = (Document_Internal::RequestResourceUserData *)MemAlloc(sizeof(Document_Internal::RequestResourceUserData));
    userPointer->owner = this;
    userPointer->userData = userData;
    userPointer->callback = onLoad;

    RequestFileAsynch([](const char* path, void* data, unsigned int bytes, void* userData) {
        Document_Internal::RequestResourceUserData* userPointer = (Document_Internal::RequestResourceUserData*)userData;
        Resource* result = (Resource*)MemAlloc(sizeof(Resource));

        if (data == 0 || bytes == 0) {
            result->name = 0;
            result->data = 0;
            result->size = 0;
            result->image = 0;
            result->filter = Draw2D::Interpolation::Linear;
            result->width = 0;
            result->height = 0;
            result->prev = 0;
            result->next = 0;

            if (userPointer->callback != 0) {
                userPointer->callback(path, *result, userPointer->userData);
            }

            MemRelease(result);
            result = 0;
        }
        else {
            if (path != 0) {
                const char* lastSlash = path;
                const char* iter = path;
                u32 pathLen = 0;
                while (*iter != '\0') {
                    pathLen += 1;
                    if (*iter == '/' || *iter == '\\') {
                        lastSlash = ++iter;
                        pathLen = 0;
                    }
                    else {
                        ++iter;
                    }
                }

                result->name = (char*)MemAlloc(pathLen + 1);
                int i = 0;
                for (iter = lastSlash; *iter != '\0'; ++iter) {
                    result->name[i++] = *iter;
                }
                result->name[i] = 0;
                result->nameLen = pathLen;
            }
            else {
                int uid = userPointer->owner->resourceUidGenerator + 1;
                result->name = (char*)MemAlloc(16);
                stbsp_snprintf(result->name, 16, "Image %d", uid);
                result->nameLen = 16;
            }

            result->data = data;
            result->size = bytes;
            result->image = Draw2D::LoadImage(data, bytes, Draw2D::Interpolation::Linear);
            result->filter = Draw2D::Interpolation::Linear;
            result->width = Draw2D::GetImageSize(result->image).w;
            result->height = Draw2D::GetImageSize(result->image).h;

            result->prev = result->next = 0;
            if (userPointer->owner->resources == 0) {
                result->prev = result->next = 0;
                userPointer->owner->resources = result;
            }
            else {
                Resource* tail = userPointer->owner->resources;
                while (tail->next != 0) {
                    tail = tail->next;
                }
                PlatformAssert(tail->next == 0, __LOCATION__);
                tail->next = result;
                result->prev = tail;
            }
            userPointer->owner->numResources += 1;
            result->uid = ++userPointer->owner->resourceUidGenerator;

            if (userPointer->callback != 0) {
                userPointer->callback(path, *result, userPointer->userData);
            }
        }

        MemRelease(userPointer);
    }, userPointer);
}


void Document::RearrangeNode(Node2D& nodeToInsert, Node2D& parent, Node2D* prevSibling) {
    PlatformAssert(nodeToInsert.parent != 0, __LOCATION__);

    Node2D* iter = &parent;
    while (iter != 0) {
        if (iter == &nodeToInsert) {
            return; // Early out if causing a problem
        }
        iter = iter->parent;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    RearrangeNodeAction* rAction = new (action)RearrangeNodeAction(this, &nodeToInsert, &parent, prevSibling);
    rAction->Redo();
}

void Document::UpdateAnimation(Animation* anim, u32 frameCount, u32 frameRate, AnimationLoopMode loop) {
    if (anim->frameCount == frameCount && anim->frameRate == frameRate && anim->loop == loop) {
        return;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    UpdateAnimationAction* rAction = new (action)UpdateAnimationAction(this, anim, frameCount, frameRate, loop);
    rAction->Redo();
}

void Document::ClearFrame(Track* t, u32 f) {
    if (t == 0 || f >= t->frameCount) {
        return;
    }
    if (!t->frames[f].key) {
        return;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    ClearFrameAction* rAction = new (action)ClearFrameAction(this, t, f);
    rAction->Redo();
}

float Document::GetLocalProp(Node2D* node, TrackType trackType) {
    Animation* timelineAnimation = GetTimelineAnimation();
    i32 selectedFrame = GetSelectedFrame();
   
    Track* animatedTrack = 0;
    if (timelineAnimation != 0 && node != 0) {
        animatedTrack = timelineAnimation->Contains(node, trackType);
    }
    bool contained = animatedTrack != 0 && animatedTrack->numKeyFrames > 0;

    if (contained) {
        PlatformAssert(timelineAnimation != 0, __LOCATION__); // Safe if contained

        if (node->IsPropertyDirty(trackType)) {
            return node->GetPropertyBuffer<f32>(trackType);
        }
        else {
            if (selectedFrame == -1) { // No frame selected, show frame 0
                if (animatedTrack->frames[0].key) { // Frame 0 was a key, show it's value
                    return animatedTrack->frames[0].fValue;
                }
                else { // Nothing is keyed for frame 0, show hierarchy value
                    return node->GetProperty<f32>(trackType);
                }
            }
            else {
                PlatformAssert(selectedFrame < animatedTrack->frameCount, __LOCATION__);

                if (animatedTrack->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                    return animatedTrack->frames[selectedFrame].fValue;
                }
                else {
                    float interpolated_value = animatedTrack->InterpolateF(selectedFrame, node->GetProperty<f32>(trackType), timelineAnimation->loop);
                    return interpolated_value;
                }
            }
        }
    }
    // Else
    return node->GetProperty<f32>(trackType);
}

Transform Document::GetLocalTransform(Node2D* node) {
    Transform result;
    result.scale = vec3(1, 1, 1);
    
    if (node != 0) {
        if (node->transformToolActive) {
            result.position = vec3(node->position.x, node->position.y, 0.0f);
            result.rotation = angleAxis(node->rotationAngles * MATH_DEG2RAD, vec3(0, 0, 1));
            result.scale = vec3(node->scale.x, node->scale.y, 1.0f);
        }
        else {
            result.position.x = GetLocalProp(node, TrackType::TransformPositionX);
            result.position.y = GetLocalProp(node, TrackType::TransformPositionY);
            float rotationAngles = GetLocalProp(node, TrackType::TransformRotation);
            result.rotation = angleAxis(rotationAngles * MATH_DEG2RAD, vec3(0, 0, 1));
            result.scale.x = GetLocalProp(node, TrackType::TransformScaleX);
            result.scale.y = GetLocalProp(node, TrackType::TransformScaleY);
        }
    }
    return result;
}

Transform Document::GetWorldTransform(Node2D* node) {
    Transform result = GetLocalTransform(node);
    Node2D* iter = node->parent;

    while (iter != 0) {
        Transform p = GetLocalTransform(iter);
        result = combine(p, result);
        iter = iter->parent;
    }
    
    return result;
}

void Document::NewDocument() {
    ClearUndoHistory();
    nodeUidGenerator = 0;
    resourceUidGenerator = 0;
    animationUidGenerator = 0;
    selectedNode = 0;
    timelineSelectedFrame = -1;
    timelineLastSelectedFrame = -1;
    selectedTrack = 0;
    timelineAnimation = 0;
    needsEndOfFrameUpdate = 0;
    selectedInterpolationType = 0;
    numNodes = 0;
    selectedResource = 0;

    // Release the hierarchy
    ForEachNode(rootNode, [](Node2D* node, void* userData) {
        PlatformAssert(node->refCount == 1, __LOCATION__);
        node->refCount -= 1;
        }, 0);
    RecursivleyDestroyNodeIfItHasNoReferences(rootNode);
    rootNode = 0;

    // Release any resources
    if (resources != 0) {
        Resource* iter = resources;
        while (iter != 0) {
            Resource* release = iter;
            iter = iter->next;
            DestroyResource(release);
        }
    }
    resources = 0;
    numResources = 0;

    rootNode = (Node2D*)MemAlloc(sizeof(Node2D));
    MemClear(rootNode, sizeof(Node2D));
    rootNode->scale = vec2(1, 1);
    rootNode->refCount += 1;

    {
        Animation* anim = allAnimations;
        while (anim != 0) {
            Animation* to_delete = anim;
            if (to_delete->numTracks > 0) {
                Track* track = to_delete->tracks;
                while (track != 0) {
                    Track* to_delete_track = track;
                    if (to_delete_track->frames != 0) {
                        MemRelease(to_delete_track->frames);
                    }
                    MemRelease(to_delete_track);
                    track = track->next;
                }
            }
            if (to_delete->name != 0) {
                MemRelease(to_delete->name);
            }
            MemRelease(to_delete);
            anim = anim->next;
        }
    }
    allAnimations = 0;
    numAnimations = 0;
    selectedAnimation = 0;
}

void Document::EndOfFrame(bool autoKey) {
    if (needsEndOfFrameUpdate >= 0) {
        i32 frame = GetSelectedFrame();
        Animation* timelineAnimation = GetTimelineAnimation();

        for (Node2D* iter = DepthFirst(0); iter != 0; iter = DepthFirst(iter)) {
            // Default to whatever the hierarchy representation is
            iter->anim_position.x = iter->position.x;
            iter->anim_position.y = iter->position.y;
            iter->anim_rotationAngles = iter->rotationAngles;
            iter->anim_scale.x = iter->scale.x;
            iter->anim_scale.y = iter->scale.y;
            MemCopy(&iter->anim_sprite, &iter->sprite, sizeof(SpriteComponent));

            if (timelineAnimation != 0 && frame >= 0 && frame <= timelineAnimation->frameCount) {
                Track* currentTrack = timelineAnimation->tracks;
                while (currentTrack != 0) {
                    if (iter->uid == currentTrack->targetNode) {
                        if (currentTrack->targetProp == TrackType::TransformPositionX) {
                            iter->anim_position.x = currentTrack->InterpolateF(frame, iter->position.x, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::TransformPositionY) {
                            iter->anim_position.y = currentTrack->InterpolateF(frame, iter->position.y, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::TransformRotation) {
                            iter->anim_rotationAngles = currentTrack->InterpolateF(frame, iter->rotationAngles, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::TransformScaleX) {
                            iter->anim_scale.x = currentTrack->InterpolateF(frame, iter->scale.x, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::TransformScaleY) {
                            iter->anim_scale.y = currentTrack->InterpolateF(frame, iter->scale.y, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteTintR) {
                            iter->anim_sprite.tintR = currentTrack->InterpolateF(frame, iter->sprite.tintR, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteTintG) {
                            iter->anim_sprite.tintG = currentTrack->InterpolateF(frame, iter->sprite.tintG, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteTintB) {
                            iter->anim_sprite.tintB = currentTrack->InterpolateF(frame, iter->sprite.tintB, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteVisibility) {
                            iter->anim_sprite.visible = currentTrack->InterpolateB(frame, iter->sprite.visible, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteSourceX) {
                            iter->anim_sprite.sourceX = currentTrack->InterpolateI(frame, iter->sprite.sourceX, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteSourceY) {
                            iter->anim_sprite.sourceY = currentTrack->InterpolateI(frame, iter->sprite.sourceY, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteSourceW) {
                            iter->anim_sprite.sourceW = currentTrack->InterpolateI(frame, iter->sprite.sourceW, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SpriteSourceH) {
                            iter->anim_sprite.sourceH = currentTrack->InterpolateI(frame, iter->sprite.sourceH, timelineAnimation->loop);
                        }
                        else if (currentTrack->targetProp == TrackType::SortIndex) {
                            iter->anim_sortIndex = currentTrack->InterpolateI(frame, iter->sortIndex, timelineAnimation->loop);
                        }
                    }
                    currentTrack = currentTrack->next;
                }
            }

            iter->anim_positionX_dirty = false;
            iter->anim_positionY_dirty = false;
            iter->anim_rotation_dirty = false;
            iter->anim_scaleX_dirty = false;
            iter->anim_scaleY_dirty = false;
            iter->anim_sourceX_dirty = false;
            iter->anim_sourceY_dirty = false;
            iter->anim_sourceW_dirty = false;
            iter->anim_sourceH_dirty = false;
            iter->anim_tintR_dirty = false;
            iter->anim_tintG_dirty = false;
            iter->anim_tintB_dirty = false;
            iter->anim_tintA_dirty = false;
            iter->anim_visible_dirty = false;
        }
    }
    needsEndOfFrameUpdate = -1;
    timelineLastSelectedFrame = timelineSelectedFrame;
}

void Document::SetSelectedFrame(i32 frame) {
    if (timelineSelectedFrame != frame) {
        needsEndOfFrameUpdate = timelineSelectedFrame;
    }
    timelineSelectedFrame = frame;
}

void Document::AutoKeyFrameValue(Track* t, u32 frame, InterpolationType interp, u32 frameVal_as_u32) {
    PlatformAssert(frame < t->frameCount, __LOCATION__);

    Animation* timeline = GetTimelineAnimation();
    if (timeline == 0) {
        return;
    }
    bool looping = timeline->loop == AnimationLoopMode::Looping;

    Node2D* node = FindNodeById(t->targetNode);
    PlatformAssert(node != 0, __LOCATION__);
    Frame fr = node->GetIneterpolatedFrame(t, frame, looping);

    if (t->targetProp == TrackType::SpriteVisibility) {
        interp = InterpolationType::Step;
    }

    if (t->frames[frame].key && t->frames[frame].uValue == fr.uValue) {
        if (t->frames[frame].interp == interp) {
            return;
        }
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    AutoKeyFrameAction* rAction = new (action)AutoKeyFrameAction(this, t, frame, interp, fr, frameVal_as_u32);
    rAction->Redo();
}

void Document::SetFrame(Track* t, u32 f, InterpolationType interp) {
    PlatformAssert(f < t->frameCount, __LOCATION__);
    
    Animation* timeline = GetTimelineAnimation();
    if (timeline == 0) {
        return;
    }
    bool looping = timeline->loop == AnimationLoopMode::Looping;

    Node2D* node = FindNodeById(t->targetNode);
    PlatformAssert(node != 0, __LOCATION__);
    Frame fr = node->GetIneterpolatedFrame(t, f, looping);

    if (t->targetProp == TrackType::SpriteVisibility) {
        interp = InterpolationType::Step;
    }

    if (t->frames[f].key && t->frames[f].uValue == fr.uValue) {
        if (t->frames[f].interp == interp) {
            return;
        }
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    SetFrameAction* rAction = new (action)SetFrameAction(this, t, f, interp, looping, fr);
    rAction->Redo();
}

void Document::RemoveTrack(Track* track) {
    if (!track->avtive) {
        return;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    DeleteTrackAction* rAction = new (action)DeleteTrackAction(this, track);
    rAction->Redo();
}

Track* Document::AddTrack(Animation* anim, Node2D* target, TrackType type) {
    Track* iter = anim->tracks;
    while (iter != 0) {
        if (target->uid == iter->targetNode) {
            if (type == iter->targetProp) {
                if (!iter->avtive) {
                    StaticActionSize* action = GrabNextUndoSlot(this);
                    ActivateTrackAction* rAction = new (action)ActivateTrackAction(this, iter);
                    rAction->Redo();
                }

                return iter;
            }
        }
        iter = iter->next;
    }

    StaticActionSize* action = GrabNextUndoSlot(this);
    CreateTrackAction* rAction = new (action)CreateTrackAction(this, anim, target, type);
    rAction->Redo();

    return rAction->result;
}

void Document::RenameAnimation(Animation* anim, const char* newName) {
    PlatformAssert(anim != 0, __LOCATION__);
    PlatformAssert(newName != 0, __LOCATION__);

    bool same = true;
    const char* oldName = anim->name;

    u32 newLen = 0;
    for (; newName[newLen] != '\0'; ++newLen);
    u32 oldLen = 0;

    for (; oldName[oldLen] != '\0'; ++oldLen);

    if (newLen != oldLen) {
        same = false;
    }
    else {
        for (u32 i = 0; i < newLen; ++i) {
            if (newName[i] != oldName[i]) {
                same = false;
                break;
            }
        }
    }

    if (!same) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        RenameAnimationAction* rAction = new (action)RenameAnimationAction(this, anim, newName, newLen);
        rAction->Redo();
    }
}

void Document::RenameNode(Node2D* node, const char* newName) {
    PlatformAssert(node != 0, __LOCATION__);
    PlatformAssert(newName != 0, __LOCATION__);

    const char* oldName = node->name;

    bool same = true;

    u32 newLen = 0;
    for (; newName[newLen] != '\0'; ++newLen);
    u32 oldLen = 0;
    for (; oldName[oldLen] != '\0'; ++oldLen);

    if (newLen != oldLen) {
        same = false;
    }
    else {
        for (u32 i = 0; i < newLen; ++i) {
            if (newName[i] != oldName[i]) {
                same = false;
                break;
            }
        }
    }

    if (!same) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        RenameNodeAction* rAction = new (action)RenameNodeAction(this, node, newName);
        rAction->Redo();
    }
}

bool Document::UpdateSpriteTint(Node2D* node, float r, float g, float b, float a) {
    bool exec = false;

    float rDelta = MathAbsF(r - node->sprite.tintR);
    float gDelta = MathAbsF(g - node->sprite.tintR);
    float bDelta = MathAbsF(b - node->sprite.tintR);
    float aDelta = MathAbsF(a - node->sprite.tintR);

    if (rDelta > NODE_TRANSFORM_DELTA) {
        exec = true;
    }

    if (gDelta > NODE_TRANSFORM_DELTA) {
        exec = true;
    }

    if (bDelta > NODE_TRANSFORM_DELTA) {
        exec = true;
    }

    if (aDelta > NODE_TRANSFORM_DELTA) {
        exec = true;
    }
    if (exec) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        UpdateSpriteTintAction* rAction = new (action)UpdateSpriteTintAction(this, node, r, g, b, a);
        rAction->Redo();
    }

    return exec;
}

bool Document::UpdateSpriteVisibility(Node2D* node, bool visible, i32 sortIndex) {
    if (node->sprite.visible != visible || node->sortIndex != sortIndex) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        ChangeNodeVisibilityAction* rAction = new (action)ChangeNodeVisibilityAction(this, node, visible, sortIndex);
        rAction->Redo();
        return true;
    }
    return false;
}

bool Document::UpdateSprite(Node2D* node, i32 sX, i32 sY, i32 sW, i32 sH, f32 pX, f32 pY) {
    PlatformAssert(node != 0, __LOCATION__);
    
    bool exec = false;

    i32 _sx = node->sprite.sourceX;
    i32 _sy = node->sprite.sourceY;
    i32 _sw = node->sprite.sourceW;
    i32 _sh = node->sprite.sourceH;

    if (_sx != sX) {
        exec = true;
    }

    if (_sy != sY) {
        exec = true;
    }

    if (_sw != sW) {
        exec = true;
    }

    if (_sh != sH) {
        exec = true;
    }

    f32 xDelta = MathAbsF(node->sprite.pivotX - pX);
    f32 yDelta = MathAbsF(node->sprite.pivotY - pY);

    if (xDelta > NODE_TRANSFORM_DELTA) {
        exec = true;
    }

    if (yDelta > NODE_TRANSFORM_DELTA) {
        exec = true;
    }

    if (exec) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        UpdateResourceRectAction* rAction = new (action)UpdateResourceRectAction(this, node, sX, sY, sW, sH, pX, pY);
        rAction->Redo();
    }

    return exec;
}

void Document::UpdateSprite(Node2D* node, struct Resource* resource) {
    PlatformAssert(node != 0, __LOCATION__);

    bool exec = false;
    if (resource != 0) {
        if (resource->uid != node->sprite.resourceUID) {
            exec = true;
        }
    }
    else {
        if (0 != node->sprite.resourceUID) {
            exec = true;
        }
    }

    if (exec) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        SetNodeResourceAction* rAction = new (action)SetNodeResourceAction(this, node, resource);
        rAction->Redo();
    }
}

bool Document::UpdateNodeTransformSingleF(Node2D* node, float val, TrackType type) {
    vec2 pos = node->position;
    vec2 scl = node->scale;
    float rot = node->rotationAngles;
    vec2 pivot(node->sprite.pivotX, node->sprite.pivotY);
    i32 sort = node->sortIndex;

    bool update_transform = false;
    bool update_sprite = false;
    bool update_sort = false;

    if (type == TrackType::TransformPositionX) {
        pos.x = val;
        update_transform = true;
    }
    else if (type == TrackType::TransformPositionY) {
        update_transform = true;
        pos.y = val;
    }
    else if (type == TrackType::TransformRotation) {

        rot = val;
        update_transform = true;
    }
    else if (type == TrackType::TransformScaleX) {
        scl.x = val;
        update_transform = true;
    }
    else if (type == TrackType::TransformScaleY) {
        scl.y = val;
        update_transform = true;
    }
    else if (type == TrackType::SortIndex) {
        sort = val;
        update_sort = true;
    }
    else {
        PlatformAssert(false, __LOCATION__);
    }

    i32 selectedFrame = GetSelectedFrame();
    if (update_transform) {
        return UpdateNodeTransform(node, pos, rot, scl);
    }
    if (update_sprite) {
        return UpdateSprite(node,
            (i32)node->sprite.sourceX,
            (i32)node->sprite.sourceY,
            (i32)node->sprite.sourceW,
            (i32)node->sprite.sourceH,
            pivot.x, pivot.y);
    }
    if (update_sort) {
        return UpdateSpriteVisibility(node, node->sprite.visible, sort);
    }
    return false;
}

bool Document::UpdateNodeTransformSingleI(Node2D* node, i32 val, TrackType type) {
    i32 sX = node->sprite.sourceX;
    i32 sY = node->sprite.sourceY;
    i32 sW = node->sprite.sourceW;
    i32 sH = node->sprite.sourceH;
    i32 sort = node->sortIndex;

    bool updateSprite = false;
    bool updateSort = false;

    if (type == TrackType::SpriteSourceX) {
        sX = val;
        updateSprite = true;
    }
    else if (type == TrackType::SpriteSourceY) {
        sY = val;
        updateSprite = true;
    }
    else if (type == TrackType::SpriteSourceW) {
        sW = val;
        updateSprite = true;
    }
    else if (type == TrackType::SpriteSourceH) {
        sH = val;
        updateSprite = true;
    }
    else if (type == TrackType::SortIndex) {
        sort = val;
        updateSort = true;
    }
    else {
        PlatformAssert(false, __LOCATION__);
    }

    if (updateSprite) {
        return UpdateSprite(node, sX, sY, sW, sH, node->sprite.pivotX, node->sprite.pivotY);
    }
    PlatformAssert(updateSort, __LOCATION__);
    return UpdateSpriteVisibility(node, node->sprite.visible, sort);
}

bool Document::UpdateNodeTransform(Node2D* node, const vec2& pos, float rot, const vec2& scl) {
    PlatformAssert(node != 0, __LOCATION__);

    vec2 posDelta = abs(node->position - pos);
    float rotDelta = MathAbsF(node->rotationAngles - rot);
    vec2 scaleDelta = abs(node->scale - scl);

    bool same = true;
    if (posDelta.x > NODE_TRANSFORM_DELTA) {
        same = false;
    }
    if (posDelta.y > NODE_TRANSFORM_DELTA) {
        same = false;
    }
    if (rotDelta > NODE_TRANSFORM_DELTA) {
        same = false;
    }
    if (scaleDelta.x > NODE_TRANSFORM_DELTA) {
        same = false;
    }
    if (scaleDelta.y > NODE_TRANSFORM_DELTA) {
        same = false;
    }

    if (!same) {
        StaticActionSize* action = GrabNextUndoSlot(this);
        EditTransformAction* eAction = new (action)EditTransformAction(this, node, pos, rot, scl);
        eAction->Redo();
        return true;
    }
    return false;
}

Node2D* Document::SelectNode(Node2D* node) {
    u32 prevSelectionId = 0;
    Node2D* prevSelect = 0;
    if (selectedNode != 0) {
        prevSelectionId = selectedNode->uid;
        prevSelect = selectedNode;
    }

    u32 nodeSelectionID = 0;
    if (selectedNode != 0 && node != 0 && selectedNode->uid == node->uid) {
        return selectedNode;
    }
    else if (node != 0) {
        nodeSelectionID = node->uid;
    }
    
    StaticActionSize* action = GrabNextUndoSlot(this);
    HierarchySelectAction* hAction = new (action) HierarchySelectAction(this, nodeSelectionID, prevSelectionId);
    hAction->Redo();
    return prevSelect;
}

Document::Document(u32 numUndoSteps) :
     numNodes(0), selectedNode(0),
    nodeUidGenerator(0), undoStackCurrent(0),
    timelineSelectedFrame(-1) {

    if (numUndoSteps < 10) {
        numUndoSteps = 10;
    }

    if (numUndoSteps % 2 != 0) {
        numUndoSteps += 1;
    }
    u32 undoStackSize = numUndoSteps * 2 * sizeof(StaticActionSize);

    rootNode = (Node2D*)MemAlloc(sizeof(Node2D));
    MemClear(rootNode, sizeof(Node2D));
    rootNode->scale = vec2(1, 1);
    rootNode->refCount += 1;

    undoNumSteps = numUndoSteps;
    undoStack = (StaticActionSize*)MemAlloc(undoStackSize);
    MemClear(undoStack, undoStackSize);
    undoStackTop = undoStackCurrent;

    // Initialize all undo actions to be null actions
    for (u32 i = 0; i < numUndoSteps * 2; ++i) {
        NullAction* action = new (&undoStack[i])NullAction();
    }
}

void Document::ClearUndoHistory(u32 firstIndex, u32 lastIndex) {
    if (lastIndex > undoStackTop) {
        lastIndex = undoStackTop;
    }
    if (lastIndex == 0 && undoStackTop == 0) {
        return;
    }

    PlatformAssert(firstIndex < lastIndex, __LOCATION__);
    PlatformAssert(lastIndex >= 1, __LOCATION__);

    i32 _lastIndex = lastIndex;
    i32 _firstIndex = firstIndex;

    SelectTrack(0);
    SetSelectedFrame(-1);
    SelectTimeline(0);

    if (lastIndex >= 1) {
        //for (i32 i = _lastIndex - 1; i >= _firstIndex; i--) {
        // I'm sure this will come back to bite me in the ass
        // loop forward, because if looping backward, i can
        // get at a destroy call, without ever having seen it's 
        // create call in the stack, leaking one reference.
        // If it's a problem later, remove the reference assert 1
        // from the destroy action? I'm not sure if that's a good solution.
        for (i32 i = _firstIndex; i < _lastIndex; ++i) {
            DocumentAction* actionToUndo = (DocumentAction*)&undoStack[i];
            actionToUndo->~DocumentAction();
            new (actionToUndo)NullAction();
        }
    }
}

Document::~Document() {
    ClearUndoHistory(0, undoStackTop);
    PlatformAssert(rootNode->refCount == 1, __LOCATION__);

    selectedAnimation = 0;
    timelineSelectedFrame = -1;
    timelineLastSelectedFrame = -1;
    timelineAnimation = 0;

    {
        Animation* anim = allAnimations; 
        while (anim != 0) {
            Animation* to_delete = anim;
            if (to_delete->numTracks > 0) {
                Track* track = to_delete->tracks;
                while (track != 0) {
                    Track* to_delete_track = track;
                    if (to_delete_track->frames != 0) {
                        MemRelease(to_delete_track->frames);
                    }
                    MemRelease(to_delete_track);
                    track = track->next;
                }
            }
            if (to_delete->name != 0) {
                MemRelease(to_delete->name);
            }
            MemRelease(to_delete);
            anim = anim->next;
        }
    }

    // Release the hierarchy
    ForEachNode(rootNode, [](Node2D* node, void* userData) {
        PlatformAssert(node->refCount == 1, __LOCATION__);
        node->refCount -= 1;
    }, 0);
    RecursivleyDestroyNodeIfItHasNoReferences(rootNode);
    rootNode = 0;

    // Release any resources
    if (resources != 0) {
        Resource* iter = resources;
        while (iter != 0) {
            Resource* release = iter;
            iter = iter->next;
            DestroyResource(release);
        }
        resources = 0;
        numResources = 0;
    }

    // Release the undo stack
    MemRelease(undoStack);
}

StaticActionSize* GrabNextUndoSlot(Document* d) {
    // Ran out of steps. Remove the first half of the undo stack
    // and use the second half. I doubled the requested undo
    // stack size to avoid having to use a doubly linked list
    if (d->undoStackCurrent == d->undoNumSteps * 2) {
        void* target = d->undoStack;
        void* source = &d->undoStack[d->undoNumSteps];
        d->ClearUndoHistory(0, d->undoNumSteps);
        
        MemCopy(target, source, d->undoNumSteps * sizeof(StaticActionSize));
        d->undoStackTop -= d->undoNumSteps;
        d->undoStackCurrent -= d->undoNumSteps;

        for (i32 i = d->undoNumSteps * 2 - 1; i >= d->undoNumSteps; i--) {
            DocumentAction* actionToUndo = (DocumentAction*)&d->undoStack[i];
            // Destructor does not need to be called, we're just transferring ownership
            // with that memcpy
            new (actionToUndo)NullAction();
        }
    }
    else {
        // Abandoning old undo stack with new action, need to clean up memory
        if (d->undoStackTop != d->undoStackCurrent) {
            d->ClearUndoHistory(d->undoStackCurrent, d->undoStackTop);
            d->undoStackTop = d->undoStackCurrent;
        }
    }

    StaticActionSize* result = &d->undoStack[d->undoStackCurrent++];
    d->undoStackTop = d->undoStackCurrent;

    return result;
}

void Document::Undo() {
    if (undoStackCurrent == 0) {
        return;
    }

    StaticActionSize* genericAction = &undoStack[--undoStackCurrent];
    DocumentAction* action = (DocumentAction*)genericAction;
    action->Undo();
}

void Document::Redo() {
    if (undoStackCurrent >= undoStackTop) {
        return;
    }

    StaticActionSize* genericAction = &undoStack[undoStackCurrent++];
    DocumentAction* action = (DocumentAction*)genericAction;
    action->Redo();
}

const char* Document::GetUndoStepName(u32 index) {
    PlatformAssert(undoStackTop >= GetNumUndoSteps(), __LOCATION__);
    u32 base = undoStackTop - GetNumUndoSteps();
    StaticActionSize* genericAction = &undoStack[base + index];
    DocumentAction* action = (DocumentAction*)genericAction;
    return action->Name();
}