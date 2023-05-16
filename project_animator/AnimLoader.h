#define _CRT_SECURE_NO_WARNINGS
#ifndef  _H_ANIM_LOADER_
#define _H_ANIM_LOADER_

#include "../platform/memory.h"

namespace AnimLoader {
	typedef u32 b32;
	typedef u64 str;
	typedef u64 ptr;

	static_assert (sizeof(b32) == 4, "b32 should be defined as a 4 byte type");
	static_assert (sizeof(ptr) == 8, "ptr should be defined as a 8 byte type");
	static_assert (sizeof(str) == 8, "ptr should be defined as a 8 byte type");

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
		/*0*/u32 nodeCount;
		/*1*/u32 resourceCount;
		/*2*/u32 animationCount;
		/*3*/u32 trackCount;
		/*4*/u32 nodeUidGenerator;
		/*5*/u32 resourceUidGenerator;
		/*6*/u32 animationUidGenerator;
		/*7*/u32 padding;
		/*8*/ptr firstNodePtr;
		/*10*/ptr firstResourcePtr;
		/*12*/ptr firstAnimPtr;
		/*14*/ptr firstTrackPtr;
		/*16*/ptr firstFramePtr;
		/*18*/ptr firstFilePtr;

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

	Header* Parse(void* data, u32 size);
};

#endif // ! _H_ANIM_LOADER_