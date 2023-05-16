#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

typedef float f32;
typedef int i32;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned int b32;

typedef u64 str;
typedef u64 ptr;

static_assert (sizeof(i32) == 4, "i32 should be defined as a 4 byte type");
static_assert (sizeof(u32) == 4, "u32 should be defined as a 4 byte type");
static_assert (sizeof(u64) == 8, "u64 should be defined as an 8 byte type");
static_assert (sizeof(f32) == 4, "f32 should be defined as a 4 byte type");
static_assert (sizeof(b32) == 4, "b32 should be defined as a 4 byte type");
static_assert (sizeof(ptr) == 8, "ptr should be defined as a 8 byte type");

struct Header;
struct Node;
struct Resource;
struct Animation;
struct Track;
struct Frame;

#define HEADER_SIZE (8 * 4 + 6 * 8)
#define NODE_SIZE (22 * 4 + 1 * 8)
#define RESOURCE_SIZE (4 * 4 + 2 * 8)
#define ANIMATION_SIZE (6 * 4 + 2 * 8)
#define TRACK_SIZE (4 * 4 + 2 * 8)
#define FRAME_SIZE (4 * 4)

struct Header {
	u32 nodeCount;
	u32 resourceCount;
	u32 animationCount;
	u32 trackCount;
	u32 nodeUidGenerator;
	u32 resourceUidGenerator;
	u32 animationUidGenerator;
	u32 padding;
	ptr firstNodePtr;
	ptr firstResourcePtr;
	ptr firstAnimPtr;
	ptr firstTrackPtr;
	ptr firstFramePtr;
	ptr firstFilePtr;

	inline Node* FirstNode() {
		return (Node*)firstNodePtr;
	}

	inline Resource* FirstResource() {
		return (Resource*)firstResourcePtr;
	}

	inline Animation* FirstAnimation() {
		return (Animation*)firstAnimPtr;
	}

	inline Track* FirstTrack() {
		return (Track*)firstTrackPtr;
	}

	inline Node* FindNodeByUID(u32 uid) {
		Node* iter = FirstNode();
		char* step = (char*)iter;
		for (u32 i = 0; i < nodeCount; ++i) {
			u32* nUid = (u32*)iter;
			if (*nUid == uid) {
				return iter;
			}
			iter = (Node*)(step + NODE_SIZE);
			step = (char*)iter;
		}
		return 0;
	}

	inline Resource* FindResourceByUID(u32 uid) {
		Resource* iter = FirstResource();
		char* step = (char*)iter;

		for (u32 i = 0; i < resourceCount; ++i) {
			u32* rUid = (u32*)iter;
			if (*rUid == uid) {
				return iter;
			}

			iter = (Resource*)(step + RESOURCE_SIZE);
			step = (char*)iter;
		}
		return 0;
	}

	inline Animation* FindAnimationByUID(u32 uid) {
		Animation* iter = FirstAnimation();
		char* step = (char*)iter;

		for (u32 i = 0; i < animationCount; ++i) {
			u32* aUid = (u32*)iter;
			if (*aUid == uid) {
				return iter;
			}

			iter = (Animation*)(step + ANIMATION_SIZE);
			step = (char*)iter;
		}
		return 0;
	}

	// TODO: Find Animation by UID
};
static_assert (sizeof(Header) == HEADER_SIZE, "Header isn't tightly packed");

struct Node {
	u32 nodeUid;
	u32 parentUid;
	u32 firstChildUid;
	u32 nextSiblingUid;
	str name;
	f32 positionX;
	f32 positionY;
	f32 rotationAngles;
	f32 scaleX;
	f32 scaleY;
	u32 resourceUid;
	f32 tintR;
	f32 tintG;
	f32 tintB;
	f32 tintA;
	u32 visible;
	f32 sourceX;
	f32 sourceY;
	f32 sourceW;
	f32 sourceH;
	f32 pivotX;
	f32 pivotY;
	i32 sortIndex;

	inline Node* Next() {
		return this + 1;
	}

	inline bool IsRoot() {
		return nodeUid == 0 && parentUid == 0;
	}
};
static_assert (sizeof(Node) == NODE_SIZE, "Node isn't tightly packed");

struct Resource {
	u32 resourceUid;
	u32 bytes;
	str name;
	u32 width;
	u32 height;
	ptr dataPtr;

	inline void* Data() {
		return (void*)dataPtr;
	}

	inline Resource* Next() {
		return this + 1;
	}
};
static_assert (sizeof(Resource) == RESOURCE_SIZE, "Resource isn't tightly packed");

struct Animation {
	u32 animUid;
	b32 looping;
	u32 frameRate;
	u32 frameCount;
	u32 numTracks;
	u32 padding;
	str name;
	ptr trackPtr;

	inline Track* FirstTrack() {
		return (Track*)trackPtr;
	}

	inline Animation* Next() {
		return this + 1;
	}
};
static_assert (sizeof(Animation) == ANIMATION_SIZE, "Animation isn't tightly packed");

struct Track {
	u32 targetAnimUid;
	u32 targetNodeUid;
	u32 targetProp;
	u32 numFrames;
	ptr framesPtr;
	ptr nextPtr;

	inline Frame* FirstFrame() {
		return (Frame*)framesPtr;
	}

	inline Track* NextInAnimation() {
		return (Track*)nextPtr;
	}

	inline Track* NextInMemory() {
		return this + 1;
	}
};
static_assert (sizeof(Track) == TRACK_SIZE, "Track isn't tightly packed");

struct Frame {
	u32 index;
	union {
		f32 fValue;
		u32 uValue;
		i32 iValue;
		b32 bValue;
	};
	u32 padding;
	u32 interp;

	inline Frame* Next() {
		return this + 1;
	}
};

static_assert (sizeof(Frame) == FRAME_SIZE, "Frame isn't tightly packed");

Header* PatchPointers(Header* header, void* base) {
	header->firstNodePtr = (ptr)((char*)base + (u64)header->firstNodePtr);
	header->firstResourcePtr = (ptr)((char*)base + (u64)header->firstResourcePtr);
	header->firstAnimPtr = (ptr)((char*)base + (u64)header->firstAnimPtr);
	header->firstTrackPtr = (ptr)((char*)base + (u64)header->firstTrackPtr);
	header->firstFramePtr = (ptr)((char*)base + (u64)header->firstFramePtr);
	header->firstFilePtr = (ptr)((char*)base + (u64)header->firstFilePtr);

	{
		Node* iter = header->FirstNode();
		for (u32 i = 0; i < header->nodeCount; ++i) {
			if (iter->name != 0) {
				iter->name = (str)((char*)base + (u64)iter->name);
			}
			iter = iter->Next();
		}
	}

	{
		Animation* iter = header->FirstAnimation();
		for (u32 i = 0; i < header->animationCount; ++i) {
			if (iter->trackPtr != 0) {
				iter->trackPtr = (ptr)((char*)base + (u64)iter->trackPtr);
			}
			if (iter->name != 0) {
				iter->name = (str)((char*)base + (u64)iter->name);
			}
			iter = iter->Next();
		}
	}

	{
		Track* iter = header->FirstTrack();
		for (u32 i = 0; i < header->trackCount; ++i) {
			if (iter->framesPtr != 0) {
				iter->framesPtr = (ptr)((char*)base + (u64)iter->framesPtr);
			}
			if (iter->nextPtr != 0) {
				iter->nextPtr = (ptr)((char*)base + (u64)iter->nextPtr);
			}
			iter = iter->NextInMemory();
		}
	}

	{
		Resource* iter = header->FirstResource();
		for (u32 i = 0; i < header->resourceCount; ++i) {
			if (iter->name != 0) {
				iter->name = (str)((char*)base + (u64)iter->name);
			}
			if (iter->dataPtr != 0) {
				iter->dataPtr = (str)((char*)base + (u64)iter->name);
			}
			iter = iter->Next();
		}
	}


	return header;
}

Header* Parse(void* data) {
	const char* expected = "Keyframe Studio";
	const char* result = (const char*)data;

	bool same = true;
	for (const char* e = expected, *r = result; *e != 0; ++e, ++r) {
		if (*e != *r) {
			same = false;
			break;
		}
	}

	if (!same) {
		std::cout << "Invalid header\n";
	}

	u32 len = 0;
	for (const char* e = expected; *e != 0; ++e, ++len);
	len += 1; // Null terminator
	void* reader = (void*)((char*)data + len);

	Header* header = (Header*)reader;

	header = PatchPointers(header, data);
	return header;
}

int main(int argc, char** argv) {
	FILE* file = fopen("sample.kfs", "rb");
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET); 

	char* fileData = (char*)malloc(fileSize + 1);
	fread(fileData, fileSize, 1, file);
	fclose(file);

	{
		Header* kfs = Parse(fileData);
		{
			std::cout << "---- Headers ----\n";
			std::cout << "Node count: " << kfs->nodeCount << "\n";
			std::cout << "Resource count: " << kfs->resourceCount << "\n";
			std::cout << "Animation count: " << kfs->animationCount << "\n";
			std::cout << "Track count: " << kfs->trackCount << "\n";
			std::cout << "Node UID gen: " << kfs->nodeUidGenerator << "\n";
			std::cout << "Resource UID gen: " << kfs->resourceUidGenerator << "\n";
			std::cout << "Animation UID gen: " << kfs->animationUidGenerator << "\n";
			std::cout << "Padding (1234): " << kfs->padding << "\n";
			std::cout << "First node offset: " << ((u64)kfs->firstNodePtr) << "\n";
			std::cout << "First resource offset: " << ((u64)kfs->firstResourcePtr) << "\n";
			std::cout << "First animation offset:" << ((u64)kfs->firstAnimPtr) << "\n";
			std::cout << "First track offset: " << ((u64)kfs->firstTrackPtr) << "\n";
			std::cout << "First frame offset: " << ((u64)kfs->firstFramePtr) << "\n";
			std::cout << "First file offset: " << ((u64)kfs->firstFilePtr) << "\n";
		}

		Node* root = kfs->FirstNode();
		{
			std::cout << "---- Nodes ----\n";
			Node* iter = kfs->FirstNode();
			char tabs[32] = { 0 };
			int t = 0;
			for (u32 i = 0; i < kfs->nodeCount; ++i) {
				Node* parent = 0;
				if (iter->nodeUid != 0) {
					parent = iter;
					while (true) {
						parent = kfs->FindNodeByUID(parent->parentUid);
						tabs[t++] = '\t';
						if (parent->nodeUid == 0 || t == 32) {
							break;
						}
					}
				}

				std::cout << tabs << "Node UID: " << iter->nodeUid << "\n";
				std::cout << tabs << "Name: " << (iter->name == 0? "NULL" : (char*)iter->name) << "\n";
				std::cout << tabs << "Parent UID: " << iter->parentUid << "\n";
				std::cout << tabs << "First child UID: " << iter->firstChildUid << "\n";
				std::cout << tabs << "Next sibling UID: " << iter->nextSiblingUid << "\n";
				std::cout << tabs << "Sorting index: " << iter->sortIndex << "\n";
				std::cout << tabs << "Name: " << (iter->name != 0 ? (char*)iter->name : "NULL") << "\n";
				std::cout << tabs << "Position.x: " << iter->positionX << "\n";
				std::cout << tabs << "Position.y: " << iter->positionY << "\n";
				std::cout << tabs << "Rotation (degrees): " << iter->rotationAngles << "\n";
				std::cout << tabs << "Scale.x: " << iter->scaleX << "\n";
				std::cout << tabs << "Scale.y: " << iter->scaleY << "\n";
				std::cout << tabs << "Resource UID: " << iter->resourceUid << "\n";
				std::cout << tabs << "Tint.r: " << iter->tintR << "\n";
				std::cout << tabs << "Tint.g: " << iter->tintG << "\n";
				std::cout << tabs << "Tint.b: " << iter->tintB << "\n";
				std::cout << tabs << "Tint.a: " << iter->tintA << "\n";
				std::cout << tabs << "Visible: " << iter->visible << "\n";
				std::cout << tabs << "Source.x: " << iter->sourceX << "\n";
				std::cout << tabs << "Source.y: " << iter->sourceY << "\n";
				std::cout << tabs << "Source.w: " << iter->sourceW << "\n";
				std::cout << tabs << "Source.h: " << iter->sourceH << "\n";
				std::cout << tabs << "Pivot.x: " << iter->pivotX << "\n";
				std::cout << tabs << "Pivot.y: " << iter->pivotY << "\n";

				iter = iter->Next();
				t = 0;
				memset(tabs, 0, 32);
			}
		}

		Resource* firstResource = kfs->FirstResource();
		{
			std::cout << "---- Resources ----\n";
			Resource* iter = kfs->FirstResource();
			for (u32 i = 0; i < kfs->resourceCount; ++i) {
				std::cout << "Name: " << (iter->name == 0 ? "NULL" : (char*)iter->name) << "\n";
				std::cout << "\tWidth: " << iter->width << "\n";
				std::cout << "\tHeight: " << iter->height << "\n";
				std::cout << "\tResource UID: " << iter->resourceUid << "\n";
				std::cout << "\tBytes: " << iter->bytes << "\n";

				iter = iter->Next();
			}
		}

		Animation* firstAnimation = kfs->FirstAnimation();
		{
			std::cout << "---- Animations ----\n";
			Animation* iter = kfs->FirstAnimation();
			for (u32 i = 0; i < kfs->animationCount; ++i) {
				std::cout << "Name: " << (iter->name == 0 ? "NULL" : (char*)iter->name) << "\n";
				std::cout << "\tUID: " << iter->animUid << "\n";
				std::cout << "\tLooping: " << (iter->looping? "TRUE" : "FALSE") << "\n";
				std::cout << "\tFrame rate: " << iter->frameRate << "\n";
				std::cout << "\tFrame count: " << iter->frameCount << "\n";
				std::cout << "\tNum tracks: " << iter->numTracks << "\n";
				std::cout << "\tPadding: " << iter->padding << "\n";

				iter = iter->Next();
			}
		}

		{
			std::cout << "---- Tracks ----\n";
			Animation* outerIter = kfs->FirstAnimation();
			for (u32 i = 0; i < kfs->animationCount; ++i) {
				Track* innerIter = outerIter->FirstTrack();
				for (u32 i = 0; i < outerIter->numTracks; ++i) {

					std::cout << "Anim uid: " << innerIter->targetAnimUid << "\n";
					std::cout << "\tNode uid: " << innerIter->targetNodeUid << "\n";
					std::cout << "\tAnimated prop: " << innerIter->targetProp << "\n";
					std::cout << "\tNum frames: " << innerIter->numFrames << "\n";

					Frame* frame = innerIter->FirstFrame();
					for (u32 j = 0; j < innerIter->numFrames; ++j) {
						std::cout << "\t\tFrame (" << j << ": " << frame->index << ")\n";
						std::cout << "\t\t\tfval: " << frame->fValue << "\n";
						std::cout << "\t\t\tuval: " << frame->uValue << "\n";
						std::cout << "\t\t\tinterp: " << frame->interp << "\n";
						std::cout << "\t\t\tpadding: " << frame->padding << "\n";

						frame = frame->Next();
					}

					innerIter = innerIter->NextInAnimation();
				}
				outerIter = outerIter->Next();
			}
		}

	}

	fileData[fileSize] = 0;
	free(fileData);
}