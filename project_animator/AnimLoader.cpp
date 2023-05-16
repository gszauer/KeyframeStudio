#include "AnimLoader.h"
#include "../platform/loader.h"
#include "../platform/assert.h"

namespace AnimLoader {
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
					iter->dataPtr = (str)((char*)base + (u64)iter->dataPtr);
				}
				//PresentFile(iter->Data(), iter->bytes);
				iter = iter->Next();
			}
		}


		return header;
	}

	Header* Parse(void* data, u32 size) {
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
			PlatformAssert(false, __LOCATION__);
			return 0;
		}

		u32 len = 0;
		for (const char* e = expected; *e != 0; ++e, ++len);
		len += 1; // Null terminator
		void* reader = (void*)((char*)data + len);
		Header* header = (Header*)reader;

		for (const char* e = expected, *r = ((const char*)data) + (size - len + 1); *e != 0; ++e, ++r) {
			if (*e != *r) {
				same = false;
				break;
			}
		}

		if (!same) {
			PlatformAssert(false, __LOCATION__);
			return 0;
		}

		header = PatchPointers(header, data);
		return header;
	}
}