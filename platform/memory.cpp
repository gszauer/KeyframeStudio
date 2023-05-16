#include "memory.h"
#include "assert.h"

#if defined(MEM_PLATFORM_WINDOWS)
    #include <windows.h>
    mem_cfunc void PrintDebugString(const char* str) {
        OutputDebugStringA(str);
    }
#elif defined(MEM_PLATFORM_WASM)
    extern unsigned char __heap_base;
    extern unsigned char __data_end;

    __attribute__ (( visibility( "default" ) )) extern "C" void MemWasmTriggerCallback(fpAllocationInfo callback, u32 index, void* mem, u32 firstPage, u32 numPages, void* tag, void* userData) {
        if (callback != 0) {
        callback(index, mem, firstPage, numPages, tag, userData);
        }
    }
#endif

// Don't disable these
#define MEM_ENABLE_SUBALLOCATORS        1

void* gHeap = 0;


#if MEM_ENABLE_ALLOCGUARD
#define MEM_ALLOCATION_magicHead        0
#define MEM_ALLOCATION_startPage        4
#define MEM_ALLOCATION_pageCount        6
#define MEM_ALLOCATION_allocPrev        8
#define MEM_ALLOCATION_allocNext        12
#define MEM_ALLOCATION_tag              16
#define MEM_ALLOCATION_magicTail        24
#define MEM_ALLOCATION_padding          28
#else
#define MEM_ALLOCATION_startPage        0
#define MEM_ALLOCATION_pageCount        2
#define MEM_ALLOCATION_allocPrev        4
#define MEM_ALLOCATION_allocNext        8
#define MEM_ALLOCATION_tag              12
#define MEM_ALLOCATION_padding          20
#endif

#define MEM_ALLOCATOR_heapSizeBytes     0
#define MEM_ALLOCATOR_overheadPages     4
#define MEM_ALLOCATOR_firstPage         8
#define MEM_ALLOCATOR_activeAllocs      12
#define MEM_ALLOCATOR_subAllocFree1     16
#define MEM_ALLOCATOR_subAllocSize1     20
#define MEM_ALLOCATOR_subAllocFree2     24
#define MEM_ALLOCATOR_subAllocSize2     28
#define MEM_ALLOCATOR_subAllocFree3     32
#define MEM_ALLOCATOR_subAllocSize3     36
#define MEM_ALLOCATOR_activePages       40
#define MEM_ALLOCATOR_mostActivePages   44

#define MEM_ALLOCACTION_MAGIC_MAIN       (((u32)'m' << 0) | ((u32)'e' << 8) | ((u32)'m' << 16) | ((u32)'_' << 24))    // 1601004909
#define MEM_ALLOCACTION_MAGIC_SUB_ACTIVE (((u32)'s' << 0) | ((u32)'u' << 8) | ((u32)'b' << 16) | ((u32)'a' << 24))    // 
#define MEM_ALLOCACTION_MAGIC_SUB_FREE   (((u32)'s' << 0) | ((u32)'u' << 8) | ((u32)'b' << 16) | ((u32)'f' << 24))    // 
#define MEM_ALLOCACTION_MAGIC_RELEASED   (((u32)'f' << 0) | ((u32)'r' << 8) | ((u32)'e' << 16) | ((u32)'e' << 24))    // 1701147238


#if 1
#define MEM_READU16(ptr, offset)        (*(u16*)((u8*)(ptr) + (offset)))
#define MEM_WRITEU16(ptr, offset, val)  (*(u16*)((u8*)(ptr) + (offset)) = (u16)(val))

#define MEM_READU32(ptr, offset)        (*(u32*)((u8*)(ptr) + (offset)))
#define MEM_WRITEU32(ptr, offset, val)  (*(u32*)((u8*)(ptr) + (offset)) = (u32)(val))

#define MEM_READU64(ptr, offset)        (*(u64*)((u8*)(ptr) + (offset)))
#define MEM_WRITEU64(ptr, offset, val)  (*(u64*)((u8*)(ptr) + (offset)) = (u64)(val))

#define MEM_SUBALLOCSIZE(ptr, index)    (*(u32*)((u8*)(ptr) + 20 + ((index) * 8)))
#define MEM_GETSUBALLOCOFFSET(ptr, idx) (*(u32*)((u8*)(ptr) + 16 + ((idx) * 8)))
#define MEM_SETSUBALLOCOFFSET(ptr,i,v)  (*(u32*)((u8*)(ptr) + 16 + ((i) * 8)) = (u32)(v))
#define MEM_PAGEMASKPTR(ptr)            ( (u32*)((u8*)(ptr) + MEM_ALLOCATOR_SIZE))
#define MEM_HEAPSIZE(ptr)               (*(u32*)((u8*)(ptr) + 0 ))
#define MEM_ACTIVEALLOCSOFFSET(ptr)     (*(u32*)((u8*)(ptr) + 12))
#define MEM_FIRSTPAGEOFFSET(ptr)        (*(u32*)((u8*)(ptr) + 8 ))
#define MEM_FIRSTPAGEPTR(ptr)           (       ((u8*)(ptr) + MEM_FIRSTPAGEOFFSET(ptr)))
#define MEM_GETPAGEPTR(ptr, page)       ((u8*)MEM_FIRSTPAGEPTR(ptr) + (MEM_PAGE_SIZE * (page)))

#else

inline u16 MEM_READU16(void* ptr, u32 offset) {
    unsigned char* u8_ptr = (unsigned char*)ptr;
    u16 low =  u8_ptr[offset];
    u16 high = u8_ptr[offset + 1];

    return low | (high << 8);
}

inline void MEM_WRITEU16(void* ptr, u32 offset, u16 val) {
    unsigned char* u8_ptr = (unsigned char*)ptr;

    u8 b1 = (val & (255));
    u8 b2 = (val & (255 << 8)) >> 8;

    u8_ptr[offset] = b1;
    u8_ptr[offset + 1] = b2;
}

inline u32 MEM_READU32(void* ptr, u32 offset) {
    unsigned char* u8_ptr = (unsigned char*)ptr;
    u32 b1 = u8_ptr[offset + 0];
    u32 b2 = u8_ptr[offset + 1];
    u32 b3 = u8_ptr[offset + 2];
    u32 b4 = u8_ptr[offset + 3];

    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

inline void MEM_WRITEU32(void* ptr, u32 offset, u32 val) {
    unsigned char* u8_ptr = (unsigned char*)ptr;

    u8 b1 = (val & (255));
    u8 b2 = (val & (255 << 8)) >> 8;
    u8 b3 = (val & (255 << 16)) >> 16;
    u8 b4 = (val & (255 << 24)) >> 24;

    u8_ptr[offset + 0] = b1;
    u8_ptr[offset + 1] = b2;
    u8_ptr[offset + 2] = b3;
    u8_ptr[offset + 3] = b4;
}

inline u64 MEM_READU64(void* ptr, u32 offset) {
    unsigned char* u8_ptr = (unsigned char*)ptr;
    u64 b1 = u8_ptr[offset + 0];
    u64 b2 = u8_ptr[offset + 1];
    u64 b3 = u8_ptr[offset + 2];
    u64 b4 = u8_ptr[offset + 3];
    u64 b5 = u8_ptr[offset + 4];
    u64 b6 = u8_ptr[offset + 5];
    u64 b7 = u8_ptr[offset + 6];
    u64 b8 = u8_ptr[offset + 7];

    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24) | (b5 << 32) | (b6 << 40) | (b7 << 48) || (b8 << 56);
}

inline void MEM_WRITEU64(void* ptr, u32 offset, u64 val) {
    unsigned char* u8_ptr = (unsigned char*)ptr;

    u32 a = (val << 32) >> 32;
    u32 b = val >> 32;

    MEM_WRITEU32(ptr, offset, a);
    MEM_WRITEU32(ptr, offset + sizeof(u32), b);
}

inline u32 MEM_SUBALLOCSIZE(void* ptr, u32 index) {
    return MEM_READU32(ptr, MEM_ALLOCATOR_subAllocSize1 + index * 8);
}

inline u32 MEM_GETSUBALLOCOFFSET(void* ptr, u32 index) {
    return MEM_READU32(ptr, MEM_ALLOCATOR_subAllocFree1 + index * 8);
}

inline void MEM_SETSUBALLOCOFFSET(void* ptr, u32 index, u32 value) {
    MEM_WRITEU32(ptr, MEM_ALLOCATOR_subAllocFree1 + index * 8, value);
}

inline u32* MEM_PAGEMASKPTR(void* ptr) {
    u8* target = (u8*)ptr + MEM_ALLOCATOR_SIZE;
    return (u32*)target;
}

inline u32 MEM_HEAPSIZE(void* ptr) {
    return MEM_READU32(ptr, MEM_ALLOCATOR_heapSizeBytes);
}

inline u32 MEM_ACTIVEALLOCSOFFSET(void* ptr) {
    return MEM_READU32(ptr, MEM_ALLOCATOR_activeAllocs);
}

inline u32 MEM_FIRSTPAGEOFFSET(void* ptr) {
    return MEM_READU32(ptr, MEM_ALLOCATOR_firstPage);
}

inline u8* MEM_FIRSTPAGEPTR(void* ptr) {
    return (u8*)ptr + MEM_FIRSTPAGEOFFSET(ptr);
}

inline u8* MEM_GETPAGEPTR(void* ptr, u32 page) {
    return MEM_FIRSTPAGEPTR(ptr) + (MEM_PAGE_SIZE * page);
}
#endif

void* operator new(platform_t, void* p) {
    return p;
}

#if MEM_PLATFORM_WASM
void operator delete(void* p)  throw() {
    PlatformAssert(false, __LOCATION__);
}
#endif

static void MemInternal_ActivePagesAdded(u8* heap, u32 delta) {
    u32 activePages = MEM_READU32(heap, MEM_ALLOCATOR_activePages);
    u32 maxPages = MEM_READU32(heap, MEM_ALLOCATOR_mostActivePages);
    activePages += delta;
    if (activePages > maxPages) {
        maxPages = activePages;
        MEM_WRITEU32(heap, MEM_ALLOCATOR_mostActivePages, maxPages);
    }
    MEM_WRITEU32(heap, MEM_ALLOCATOR_activePages, activePages);
}

static void MemInternal_ActivePagesRemoved(u8* heap, u32 delta) {
    u32 activePages = MEM_READU32(heap, MEM_ALLOCATOR_activePages);
    PlatformAssert(activePages >= delta, __LOCATION__);
    activePages -= delta;
    MEM_WRITEU32(heap, MEM_ALLOCATOR_activePages, activePages);
}

mem_cfunc u32 MemGetCurrentNumPages(void* heap) {
    if (heap == 0) {
        heap = gHeap;
    }
    u32 activePages = MEM_READU32(heap, MEM_ALLOCATOR_activePages);
    return activePages;
}

mem_cfunc u32 MemGetMostPagesUsedAtOne(void* heap) {
    if (heap == 0) {
        heap = gHeap;
    }
    u32 maxPages = MEM_READU32(heap, MEM_ALLOCATOR_mostActivePages);
    return maxPages;
}


mem_cfunc i32 MemRelease(void* target) {
    return MemReleaseFromHeap(gHeap, target);
}

mem_cfunc void* MemAllocate(u32 bytes, u32 alignment, void* tag) {
    return MemAllocateOnHeap(gHeap, bytes, alignment, tag);
}

mem_cfunc void* MemReallocate(void* src, u32 newBytes, void* newTag) {
    return MemReallocateOnHeap(gHeap, src, newBytes, newTag);
}

mem_cfunc u32 MemForEachAllocation(fpAllocationInfo callback, void* userData) {
    return MemForEachAllocationOnHeap(gHeap, callback, userData);
}

mem_cfunc void* MemPlatformAllocate(u32 bytes) {
#if defined(MEM_PLATFORM_WINDOWS)
    return VirtualAlloc(0, bytes, MEM_COMMIT, PAGE_READWRITE);
#elif defined(MEM_PLATFORM_WASM)
    return &__heap_base;
#endif
}

mem_cfunc void MemPlatformRelease(void* mem) {
#if defined(MEM_PLATFORM_WINDOWS)
    VirtualFree(mem, 0, MEM_RELEASE);
#endif
}

// Can only manage up to 4 GiB of RAM. This is because internally pointers are keps as offsets
// and each offset is stored as a u32
mem_cfunc void* MemInitializeHeap(void* heap, u32 heapSize) {
    PlatformAssert(heapSize > MEM_PAGE_SIZE * 3, __LOCATION__); // Arbitrary

    // Align to be on a 4 byte boundary. This will cause all masks to be on a 4 byte boundary as well
    // and will make sure that the page mask is 4 byte aligned (since the header is 40 bytes, and the mask starts right after)
    u64 alignment = sizeof(u32); // 4
    u8* align_heap = (u8*)heap;
    if ((u64)align_heap % alignment != 0) {
        u64 remainder = (u64)align_heap % alignment;
        align_heap += (u32)alignment - (u32)remainder;
        heapSize -= (u32)alignment - (u32)remainder;
    }
    gHeap = align_heap;

    u32 numPages = heapSize / MEM_PAGE_SIZE;
    u32 pageMaskCount = (numPages / 32) + (numPages % 32 != 0 ? 1 : 0);

    PlatformAssert(numPages >= 0, __LOCATION__);

    u32 overheadBytes = (MEM_ALLOCATOR_SIZE + pageMaskCount * sizeof(u32));

    u32 overheadPageCount = (overheadBytes / MEM_PAGE_SIZE) + (overheadBytes % MEM_PAGE_SIZE != 0 ? 1 : 0);
    u32 firstPageOffset = overheadPageCount * MEM_PAGE_SIZE; // 0 = 0, 1 = 4096, etc...

    // As arguments, you pass in how big each sub-alloctors chunks should be. IE, i want this sub-allocator
    // to allocate 72 byte chunks. Then there would be "pageSize / (72 + sizeof(header))" blocks in each page

    // If you request an allocation that's MemAlloc(72, 4), it wouldn't fit into the allocator, because that
    // allocation needs 72 + 4 bytes. So it would bucket over to the next allocator.

    // In the above examples, 72 would be written to the allocator struct as a sub allocator size.
    u32 subAlloc1Size = ((MEM_PAGE_SIZE - sizeof(u32)) - (MEM_ALLOCATION_HEADER_SIZE * 8)) / 8;
    u32 subAlloc2Size = ((MEM_PAGE_SIZE - sizeof(u32)) - (MEM_ALLOCATION_HEADER_SIZE * 4)) / 4;
    u32 subAlloc3Size = ((MEM_PAGE_SIZE - sizeof(u32)) - (MEM_ALLOCATION_HEADER_SIZE * 2)) / 2;

    // struct Allocator (40 bytes)
    /* u32 heapSizeBytes */ MEM_WRITEU32(align_heap, sizeof(u32) * 0, heapSize);             // 0
    /* u32 overheadPages */ MEM_WRITEU32(align_heap, sizeof(u32) * 1, overheadPageCount);    // 4
    /* u32 firstPage     */ MEM_WRITEU32(align_heap, sizeof(u32) * 2, firstPageOffset);      // 8
    /* u32 activeAllocs  */ MEM_WRITEU32(align_heap, sizeof(u32) * 3, 0);                    // 12
    /* u32 subAllocFree1 */ MEM_WRITEU32(align_heap, sizeof(u32) * 4, 0);                    // 16
    /* u32 subAllocSize1 */ MEM_WRITEU32(align_heap, sizeof(u32) * 5, subAlloc1Size);        // 20
    /* u32 subAllocFree2 */ MEM_WRITEU32(align_heap, sizeof(u32) * 6, 0);                    // 24
    /* u32 subAllocSize2 */ MEM_WRITEU32(align_heap, sizeof(u32) * 7, subAlloc2Size);        // 28
    /* u32 subAllocFree3 */ MEM_WRITEU32(align_heap, sizeof(u32) * 8, 0);                    // 32
    /* u32 subAllocSize3 */ MEM_WRITEU32(align_heap, sizeof(u32) * 9, subAlloc3Size);        // 36
    /* u32 usedPages     */ MEM_WRITEU32(align_heap, sizeof(u32) * 10, overheadPageCount);   // 40
    /* u32 maxUedAtOnce  */ MEM_WRITEU32(align_heap, sizeof(u32) * 11, overheadPageCount);   // 44

    // Clear page mask (and set overhead pages)
    for (u32 page = 0; page < pageMaskCount; ++page) {
        MEM_WRITEU32(align_heap, MEM_ALLOCATOR_SIZE + page * sizeof(u32), (page < overheadPageCount));
    }

    return align_heap;
}

// Returns true if all memory has been released. 
mem_cfunc int MemShutdownHeap(void* heap) {
#if _DEBUG
    u32 allocationOffset = MEM_ACTIVEALLOCSOFFSET(heap);
    u32 numUnreleasedAllocations = 0;
    while (allocationOffset != 0) {
        u8* header = (u8*)heap + allocationOffset;

        u32 firstPage = MEM_READU16(header, MEM_ALLOCATION_startPage);
        u32 numPages = MEM_READU16(header, MEM_ALLOCATION_pageCount);
        u64 _label = MEM_READU64(header, MEM_ALLOCATION_tag);
        const char* label = (const char*)_label;

#if MEM_ENABLE_ALLOCGUARD
        PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicHead) != MEM_ALLOCACTION_MAGIC_RELEASED, __LOCATION__);
        PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicTail) != MEM_ALLOCACTION_MAGIC_RELEASED, __LOCATION__);
#endif

        allocationOffset = MEM_READU32(header, MEM_ALLOCATION_allocNext); // Iterate to next element
        numUnreleasedAllocations += 1;
    }
#endif

    bool allAllocationsReleased = MEM_ACTIVEALLOCSOFFSET(heap) == 0;
    bool subAllocPool0Empty = MEM_GETSUBALLOCOFFSET(heap, 0) == 0;
    bool subAllocPool1Empty = MEM_GETSUBALLOCOFFSET(heap, 1) == 0;
    bool subAllocPool2Empty = MEM_GETSUBALLOCOFFSET(heap, 2) == 0;

    PlatformAssert(allAllocationsReleased && subAllocPool0Empty && subAllocPool1Empty && subAllocPool2Empty, __LOCATION__);
    return allAllocationsReleased && subAllocPool0Empty && subAllocPool1Empty && subAllocPool2Empty;
}

mem_cfunc u32 MemGetPageSize() {
    return MEM_PAGE_SIZE;
}

mem_cfunc u32* MemGetPageMask() {
    return (u32*)((u8*)gHeap + MEM_ALLOCATOR_SIZE);
}

mem_cfunc u32 MemGetHeapSize() {
    return MEM_READU32(gHeap, MEM_ALLOCATOR_heapSizeBytes);
}

mem_cfunc u32 MemGetNumOverheadPages() {
    return MEM_READU32(gHeap, MEM_ALLOCATOR_overheadPages);
}

mem_cfunc u64 MemGetTag64(void* _mem) {
    u8* mem = (u8*)_mem;
    u16 padding = MEM_READU16((u8*)_mem - 2, 0);
    u8* header = mem - padding - MEM_ALLOCATION_HEADER_SIZE;
#if MEM_ENABLE_ALLOCGUARD
    u32 magicHead = MEM_READU32(header, MEM_ALLOCATION_magicHead);
    u32 magicTail = MEM_READU32(header, MEM_ALLOCATION_magicTail);

    PlatformAssert(magicHead != MEM_ALLOCACTION_MAGIC_RELEASED);
    PlatformAssert(magicTail != MEM_ALLOCACTION_MAGIC_RELEASED);
#endif

    return MEM_READU64(header, MEM_ALLOCATION_tag);
}

mem_cfunc void MemSetTag64(void* _mem, u64 tag) {
    u8* mem = (u8*)_mem;
    u16 padding = MEM_READU16((u8*)_mem - 2, 0);
    u8* header = mem - padding - MEM_ALLOCATION_HEADER_SIZE;
#if MEM_ENABLE_ALLOCGUARD
    u32 magicHead = MEM_READU32(header, MEM_ALLOCATION_magicHead);
    u32 magicTail = MEM_READU32(header, MEM_ALLOCATION_magicTail);

    PlatformAssert(magicHead != MEM_ALLOCACTION_MAGIC_RELEASED);
    PlatformAssert(magicTail != MEM_ALLOCACTION_MAGIC_RELEASED);
#endif

    MEM_WRITEU64(header, MEM_ALLOCATION_tag, tag);
}

mem_cfunc u32 MemGetTag32(void* _mem) {
    return (u32)MemGetTag64(_mem);
}

mem_cfunc void MemSetTag32(void* _mem, u32 tag) {
    MemSetTag64(_mem, tag);
}

// Returns 0 on error, or the index of the page where a range of numPages starts
static u32 MemInternal_FindFreePages(void* heap, u32 numPages) {
    PlatformAssert(heap != 0, __LOCATION__);
    PlatformAssert(numPages >= 1, __LOCATION__);

    // Find the required number of pages
    u32* pageMask = MEM_PAGEMASKPTR(heap);
    u32 heapNumPages = MEM_HEAPSIZE(heap) / MEM_PAGE_SIZE;

    // Find available pages
    u32 startPage = 0; // Page 0 is invalid. First page must be reserved for overhead
    u32 pageCount = 0;
    for (u32 i = 1; i < heapNumPages; ++i) {
        u32 index = i / 32;
        u32 bit = i % 32;

        if (pageMask[index] & (1 << bit)) { // Page is in use
            startPage = 0;
            pageCount = 0;
        }
        else { // Free page
            if (startPage == 0) { // New range
                startPage = i;
                pageCount = 1;
                if (numPages == 1) {
                    break;
                }
            }
            else { // Expand range
                pageCount += 1;
                if (pageCount == numPages) {
                    break;
                }
            }
        }
    }

    PlatformAssert(startPage != 0, __LOCATION__);
    PlatformAssert(pageCount == numPages, __LOCATION__);

    return startPage;
}

static bool MemInternal_ClaimPages(void* heap, u32 startPage, u32 pageCount) {
    if (startPage == 0 || pageCount == 0) {
        return false;
    }

    u32* pageMask = MEM_PAGEMASKPTR(heap);
    for (u32 i = startPage, count = startPage + pageCount; i < count; ++i) {
        u32 index = i / 32;
        u32 bit = i % 32;
        if ((pageMask[index] & (1 << bit))) {
            PlatformAssert(false, __LOCATION__);
            return false; // Error
        }
        pageMask[index] |= (1 << bit);
    }

    return true;
}

mem_cfunc bool MemIsPageUsed(u32 pageIndex, void* heap) {
    if (heap == 0) {
        heap = gHeap;
    }

    u32* pageMask = MEM_PAGEMASKPTR(heap);
    u32 index = pageIndex / 32;
    u32 bit = pageIndex % 32;

    return (bool)(pageMask[index] & (1 << bit));
}

static bool MemInternal_ReleasePages(void* heap, u32 startPage, u32 pageCount) {
    if (startPage == 0 || pageCount == 0) {
        return false;
    }

    u32* pageMask = MEM_PAGEMASKPTR(heap);
    for (u32 i = startPage, count = startPage + pageCount; i < count; ++i) {
        u32 index = i / 32;
        u32 bit = i % 32;
        if (!(pageMask[index] & (1 << bit))) {
            PlatformAssert(false, __LOCATION__);
            return false; // Error
        }
        pageMask[index] &= ~(1 << bit);
    }

    return true;
}

static void MemInternal_WriteAllocationHeader(void* header, u32 startPage, u32 pageCount, u32 allocPrev, u32 allocNext, u64 tag) {
    PlatformAssert(header != 0, __LOCATION__);

    // struct Allocation
#if MEM_ENABLE_ALLOCGUARD
    /* u32 magic        */ MEM_WRITEU32(header, MEM_ALLOCATION_magicHead, pageCount == 0 ? (MEM_ALLOCACTION_MAGIC_SUB_ACTIVE) : (MEM_ALLOCACTION_MAGIC_MAIN));
#endif
    /* u16 startPage    */ MEM_WRITEU16(header, MEM_ALLOCATION_startPage, startPage);   // 0
    /* u16 pageCount    */ MEM_WRITEU16(header, MEM_ALLOCATION_pageCount, pageCount);   // 2
    /* u32 allocPrev    */ MEM_WRITEU32(header, MEM_ALLOCATION_allocPrev, allocPrev);   // 4
    /* u32 allocNext    */ MEM_WRITEU32(header, MEM_ALLOCATION_allocNext, allocNext);   // 8
    /* u64 tag          */ MEM_WRITEU64(header, MEM_ALLOCATION_tag, tag);               // 12
#if MEM_ENABLE_ALLOCGUARD
    /* u32 magic        */ MEM_WRITEU32(header, MEM_ALLOCATION_magicTail, pageCount == 0 ? (MEM_ALLOCACTION_MAGIC_SUB_ACTIVE) : (MEM_ALLOCACTION_MAGIC_MAIN));
#endif
    /* u16 padding      */ MEM_WRITEU16(header, MEM_ALLOCATION_padding, 0);// Only sub-allocators use padding
}

static void MemInternal_AddAllocationToActiveListList(void* heap, void* header) {
#if MEM_ENABLE_ALLOCGUARD
    PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicHead) != MEM_ALLOCACTION_MAGIC_RELEASED);
    PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicTail) != MEM_ALLOCACTION_MAGIC_RELEASED);
#endif

    PlatformAssert(heap != 0, __LOCATION__);
    PlatformAssert(header != 0, __LOCATION__);
    PlatformAssert(header > heap, __LOCATION__);

    u32 offset = (u8*)header - (u8*)heap;

    u32 headOffset = MEM_READU32(heap, MEM_ALLOCATOR_activeAllocs);
    if (headOffset != 0) {
        u8* headHeader = (u8*)heap + headOffset;
        // Replace the prev "pointer" (really offset), which should be 0 (assert?) with our own
        MEM_WRITEU32(headHeader, MEM_ALLOCATION_allocPrev, offset); // Set the prev pointer of the old head to the current allocation
    }
    MEM_WRITEU32(header, MEM_ALLOCATION_allocNext, headOffset); // Set the next ptr of current allocation to old head
    MEM_WRITEU32(header, MEM_ALLOCATION_allocPrev, 0);
    MEM_WRITEU32(heap, MEM_ALLOCATOR_activeAllocs, offset); // Set the current allocation as the new head
}

#if MEM_ENABLE_ALLOCGUARD
static void MemInternal_RemoveAllocationFromActiveList(void* heap, void* header, u32 magic) {
    u32 magicHead = MEM_READU32(header, MEM_ALLOCATION_magicHead);
    u32 magicTail = MEM_READU32(header, MEM_ALLOCATION_magicTail);

    PlatformAssert(magicHead == magic);
    PlatformAssert(magicTail == magic);
#else
static void MemInternal_RemoveAllocationFromActiveList(void* heap, void* header) {
#endif

    u32 offset = (u8*)header - (u8*)heap;
    u32 prev = MEM_READU32(header, MEM_ALLOCATION_allocPrev);
    u32 next = MEM_READU32(header, MEM_ALLOCATION_allocNext);

    if (next != 0) { // Reling next
        u8* nextPtr = (u8*)heap + next;
        MEM_WRITEU32(nextPtr, MEM_ALLOCATION_allocPrev, prev);
    }

    if (prev != 0) { // Relink prev
        u8* prevPtr = (u8*)heap + prev;
        MEM_WRITEU32(prevPtr, MEM_ALLOCATION_allocNext, next);
    }

    u32 headOffset = MEM_READU32(heap, MEM_ALLOCATOR_activeAllocs);
    if (headOffset == offset) { // Update list head
        MEM_WRITEU32(heap, MEM_ALLOCATOR_activeAllocs, next); // Head was detached, next is new head
    }

    MEM_WRITEU32(header, MEM_ALLOCATION_allocPrev, 0);
    MEM_WRITEU32(header, MEM_ALLOCATION_allocNext, 0);

    // Header magic is expected to be set to free by caller
}

static void MemInternal_RemoveSubAllocationsFromFreeList(void* heap, u8 * header) {
#if MEM_ENABLE_ALLOCGUARD
    u32 magicHead = MEM_READU32(header, MEM_ALLOCATION_magicHead);
    u32 magicTail = MEM_READU32(header, MEM_ALLOCATION_magicTail);

    PlatformAssert(magicHead == MEM_ALLOCACTION_MAGIC_SUB_FREE);
    PlatformAssert(magicTail == MEM_ALLOCACTION_MAGIC_SUB_FREE);
#endif

    u32 page = MEM_READU16(header, MEM_ALLOCATION_startPage);
    u16* mask = (u16*)(MEM_GETPAGEPTR(heap, page));
    u16 stride = *(mask + 1);
    u8* allocPtr = (u8*)(mask + 2);

    u32 numAllocationsInBlock = (MEM_PAGE_SIZE - sizeof(u32)) / (u32)stride;
    for (u32 i = 0; i < numAllocationsInBlock; ++i) {
        u32 offset = (u8*)allocPtr - (u8*)heap;
        u32 prev = MEM_READU32(allocPtr, MEM_ALLOCATION_allocPrev);
        u32 next = MEM_READU32(allocPtr, MEM_ALLOCATION_allocNext);

        if (next != 0) { // Relink next
            u8* nextPtr = (u8*)heap + next;
            MEM_WRITEU32(nextPtr, MEM_ALLOCATION_allocPrev, prev);
        }

        if (prev != 0) { // Relink prev
            u8* prevPtr = (u8*)heap + prev;
            MEM_WRITEU32(prevPtr, MEM_ALLOCATION_allocNext, next);
        }

        // Remove any list headers
        if (offset == MEM_GETSUBALLOCOFFSET(heap, 0)) {
            MEM_SETSUBALLOCOFFSET(heap, 0, next);
        }
        else if (offset == MEM_GETSUBALLOCOFFSET(heap, 1)) {
            MEM_SETSUBALLOCOFFSET(heap, 1, next);
        }
        else if (offset == MEM_GETSUBALLOCOFFSET(heap, 2)) {
            MEM_SETSUBALLOCOFFSET(heap, 2, next);
        }

        MEM_WRITEU32(allocPtr, MEM_ALLOCATION_allocPrev, 0);
        MEM_WRITEU32(allocPtr, MEM_ALLOCATION_allocNext, 0);

#if MEM_ENABLE_ALLOCGUARD
        MEM_WRITEU32(allocPtr, MEM_ALLOCATION_magicHead, MEM_ALLOCACTION_MAGIC_RELEASED);
        MEM_WRITEU32(allocPtr, MEM_ALLOCATION_magicTail, MEM_ALLOCACTION_MAGIC_RELEASED);

        u8* write = allocPtr + MEM_ALLOCATION_magicTail + sizeof(u32);
        *write = '\0'; // Null out bit after magic
        write = allocPtr + MEM_ALLOCATION_magicHead + sizeof(u32);
        *write = '\0'; // Null out bit after magic
#endif

        allocPtr = allocPtr + stride;
    }
}

static void MemInternal_AddSubAllocationToFreeList(void* heap, u8 * header) {
#if MEM_ENABLE_ALLOCGUARD
    PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicHead) == MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
    PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicTail) == MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
#endif

    u32 page = MEM_READU16(header, MEM_ALLOCATION_startPage);
    u16* mask = (u16*)(MEM_GETPAGEPTR(heap, page));
    u16 stride = *(mask + 1);

    u32 subAllocatorIndex = 0;
    u32 subAlloc1Stride = MEM_SUBALLOCSIZE(heap, 0) + MEM_ALLOCATION_HEADER_SIZE;
    u32 subAlloc2Stride = MEM_SUBALLOCSIZE(heap, 1) + MEM_ALLOCATION_HEADER_SIZE;
    u32 subAlloc3Stride = MEM_SUBALLOCSIZE(heap, 2) + MEM_ALLOCATION_HEADER_SIZE;
    if (stride == (u16)subAlloc1Stride) {
        subAllocatorIndex = 0;
    }
    else if (stride == (u16)subAlloc2Stride) {
        subAllocatorIndex = 1;
    }
    else if (stride == (u16)subAlloc3Stride) {
        subAllocatorIndex = 2;
    }
    else {
        PlatformAssert(false, __LOCATION__);//  Stride should be one of the known sub-allocator strides
    }

    u32 prev = 0; // Adding as head, always 0
    u32 next = MEM_GETSUBALLOCOFFSET(heap, subAllocatorIndex); // Current head

    MEM_WRITEU32(header, MEM_ALLOCATION_allocPrev, prev);
    MEM_WRITEU32(header, MEM_ALLOCATION_allocNext, next);

    u32 headerOffset = header - (u8*)heap;
    if (next != 0) { // Set next prev to this
        u8* nextHeader = (u8*)heap + next;
        MEM_WRITEU32(nextHeader, MEM_ALLOCATION_allocPrev, headerOffset);
    }

    MEM_SETSUBALLOCOFFSET(heap, subAllocatorIndex, headerOffset); // Set list head to this

#if MEM_ENABLE_ALLOCGUARD
    MEM_WRITEU32(header, MEM_ALLOCATION_magicHead, MEM_ALLOCACTION_MAGIC_SUB_FREE);
    MEM_WRITEU32(header, MEM_ALLOCATION_magicTail, MEM_ALLOCACTION_MAGIC_SUB_FREE);
#endif
}

static void* MemInternal_SubAllocate(void* heap, u32 subAlloctorIndex, u32 bytes, u32 alignment, u64 tag) {
    PlatformAssert(subAlloctorIndex <= 2, __LOCATION__);
    PlatformAssert(heap != 0, __LOCATION__);
    PlatformAssert(bytes != 0, __LOCATION__);

    u32 subAllocHeaderStride = MEM_SUBALLOCSIZE(heap, subAlloctorIndex) + MEM_ALLOCATION_HEADER_SIZE;
    u32 numAllocationsInBlock = (MEM_PAGE_SIZE - sizeof(u32)) / subAllocHeaderStride; // First 32 bits in sub-alloc page is a flags mask.
    PlatformAssert(numAllocationsInBlock >= 2, __LOCATION__);
    u32 freeListOffset = MEM_GETSUBALLOCOFFSET(heap, subAlloctorIndex);

    // If free blocks for this sub-allocator don't exist, create some.
    if (freeListOffset == 0) {
        u32 page = MemInternal_FindFreePages(heap, 1);
        PlatformAssert(page > 0, __LOCATION__);
        if (!MemInternal_ClaimPages(heap, page, 1)) {
            PlatformAssert(false, __LOCATION__);
            return 0;
        }
        MemInternal_ActivePagesAdded((u8*)heap, 1);

        u8* header = MEM_GETPAGEPTR(heap, page);
        u16* subAllocationMask = (u16*)header;
        *subAllocationMask = 0;
        u16* subAllocationStride = subAllocationMask + 1;
        *subAllocationStride = subAllocHeaderStride;

        header += sizeof(u32);

        for (u32 i = 0; i < numAllocationsInBlock; ++i) { // struct Allocation
            u32 prev = 0; // Adding as head, always 0
            u32 next = MEM_GETSUBALLOCOFFSET(heap, subAlloctorIndex); // Current head

            MemInternal_WriteAllocationHeader(header, page, 0, prev, next, (u64)"uninitialized"); // Sub allocation fence is active
            //
            MemInternal_AddSubAllocationToFreeList(heap, header); // Sub allocation fence is free

            header += subAllocHeaderStride;
        }
    }

    // Free block is guaranteed to exist. Grab one.
    u32 head = MEM_GETSUBALLOCOFFSET(heap, subAlloctorIndex); // Current head
    u8* allocHeader = (u8*)heap + head;

    // Align the allocation (if needed)
    u8* allocation = allocHeader + MEM_ALLOCATION_HEADER_SIZE;
    u64 address = (u64)allocation; // Align the allocation
    u16 padding = 0;
    if (alignment != 0 && address % alignment != 0) {
        u64 remainder = address % alignment;
        padding = (u16)(alignment - (u32)remainder);
        allocation += padding;
    }

    // Update header data
    MEM_WRITEU64(allocHeader, MEM_ALLOCATION_tag, tag); // Update tag
    MEM_WRITEU16(allocHeader, MEM_ALLOCATION_padding, padding);
    MEM_WRITEU16(allocation - 2, 0, padding);
    
#if MEM_ENABLE_ALLOCGUARD
    PlatformAssert(MEM_READU32(allocHeader, MEM_ALLOCATION_magicHead) == MEM_ALLOCACTION_MAGIC_SUB_FREE);
    PlatformAssert(MEM_READU32(allocHeader, MEM_ALLOCATION_magicTail) == MEM_ALLOCACTION_MAGIC_SUB_FREE);
#endif

    // Get the pointer to the current pages used bitmask, and find the right bit
    u32 page = MEM_READU16(allocHeader, MEM_ALLOCATION_startPage);//head / MEM_PAGE_SIZE; // Also stored in allocHeader
    u16* mask = (u16*)MEM_GETPAGEPTR(heap, page);
    u8* firstAllocPtr = (u8*)mask + sizeof(u16) + sizeof(u16);

    // Claim the allocation as used
    u32 index = (allocHeader - firstAllocPtr) / subAllocHeaderStride;
    u16 _mask = *mask;
    PlatformAssert(!(_mask & (1 << index)), __LOCATION__); // Assume it was off before
    _mask |= (1 << index);
    *mask = _mask;

    // If there is a next pointer, unhook this from it's prev, and set it as the new sub-alloc header
    u32 next = MEM_READU32(allocHeader, MEM_ALLOCATION_allocNext);
    if (next != 0) {
        u8* nextHeader = (u8*)heap + next;
        MEM_WRITEU32(nextHeader, MEM_ALLOCATION_allocPrev, 0); // Set next prev to 0 (unlink head)
    }
    MEM_SETSUBALLOCOFFSET(heap, subAlloctorIndex, next); // Set next to be the new head

    MemInternal_AddAllocationToActiveListList(heap, allocHeader); // Will reset allocHeader prev and next pointers

#if MEM_ENABLE_ALLOCGUARD
    MEM_WRITEU32(allocHeader, MEM_ALLOCATION_magicHead, MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
    MEM_WRITEU32(allocHeader, MEM_ALLOCATION_magicTail, MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
#endif

    return allocation;
}

static i32 MemInternal_SubRelease(void* heap, void* target) {
    PlatformAssert(target != 0, __LOCATION__);

    u8* allocation = (u8*)target;
    u16 padding = MEM_READU16(allocation - 2, 0);
    u8* header = allocation - padding - MEM_ALLOCATION_HEADER_SIZE;

#if MEM_ENABLE_ALLOCGUARD
    u32 magicHead = MEM_READU32(header, MEM_ALLOCATION_magicHead);
    u32 magicTail = MEM_READU32(header, MEM_ALLOCATION_magicTail);

    PlatformAssert(magicHead == MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
    PlatformAssert(magicTail == MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
#endif

    u32 page = MEM_READU16(header, MEM_ALLOCATION_startPage);
    u16* mask = (u16*)(MEM_GETPAGEPTR(heap, page));
    u16 stride = *(mask + 1);
    u8* firstAllocPtr = (u8*)(mask + 2);
    u32 index = (header - firstAllocPtr) / (u32)stride;

    u64 _tag = MEM_READU64(header, MEM_ALLOCATION_tag);
    const char* tag = (const char*)_tag;
    PlatformAssert(MEM_READU16(header, MEM_ALLOCATION_pageCount) == 0, __LOCATION__);

    // Release (sub allocation) mask
    u16 _mask = *mask;
    if (!(_mask & (1 << index))) {  // Assume it was on before
        PlatformAssert(false, __LOCATION__);
        return 0;
    }
    _mask &= ~(1 << index);
    *mask = _mask;


#if MEM_ENABLE_ALLOCGUARD
    MemInternal_RemoveAllocationFromActiveList(heap, header, MEM_ALLOCACTION_MAGIC_SUB_ACTIVE);
#else
    MemInternal_RemoveAllocationFromActiveList(heap, header);
#endif
    MemInternal_AddSubAllocationToFreeList(heap, header);

    if (_mask == 0) { // Page is empty, release it
        MemInternal_RemoveSubAllocationsFromFreeList(heap, header);  // Sets header magic
        MemInternal_ReleasePages(heap, page, 1);
        MemInternal_ActivePagesRemoved((u8*)heap, 1);
    }

    return 1;
}

mem_cfunc void* MemAllocateOnHeap(void* heap, u32 bytes, u32 alignment, void* _tag) {
    u64 tag = (u64)_tag;

    if (heap == 0) {
        heap = gHeap;
    }

    if (bytes == 0) {
        return 0;
    }

    // Early out for any sub-allocators
#if MEM_ENABLE_SUBALLOCATORS
    if (bytes + alignment < MEM_SUBALLOCSIZE(heap, 0)) {
        return MemInternal_SubAllocate(heap, 0, bytes, alignment, tag);
    }
    else if (bytes + alignment < MEM_SUBALLOCSIZE(heap, 1)) {
        return MemInternal_SubAllocate(heap, 1, bytes, alignment, tag);
    }
    else if (bytes + alignment < MEM_SUBALLOCSIZE(heap, 2)) {
        return MemInternal_SubAllocate(heap, 2, bytes, alignment, tag);
    }
#endif

    u32 totalAllocationSize = bytes + alignment + MEM_ALLOCATION_HEADER_SIZE;
    u32 pageCount = totalAllocationSize / MEM_PAGE_SIZE + (totalAllocationSize % MEM_PAGE_SIZE == 0 ? 0 : 1);

    // Find and claim available pages
    u32 startPage = MemInternal_FindFreePages(heap, pageCount);
    PlatformAssert(startPage != 0, __LOCATION__);
    if (!MemInternal_ClaimPages(heap, startPage, pageCount)) {
        PlatformAssert(false, __LOCATION__);
        return 0; // Error
    }

    // Create allocation header
    u8* memory = MEM_GETPAGEPTR(heap, startPage);
    u8* allocation = memory + MEM_ALLOCATION_HEADER_SIZE;
    u64 address = (u64)allocation; // Align the allocation
    if (alignment != 0 && address % alignment != 0) {
        u64 remainder = address % alignment;
        allocation += alignment - (u32)remainder;
    }
    u8* header = allocation - MEM_ALLOCATION_HEADER_SIZE;

    MemInternal_WriteAllocationHeader(header, startPage, pageCount, 0, 0, tag);
    MemInternal_AddAllocationToActiveListList(heap, header); // Will set header prev and next pointers
    MemInternal_ActivePagesAdded((u8*)heap, pageCount);

    return allocation;
}

// Returns true if the memory was successfully released
// Returns true if the memory being passed in is NULL
mem_cfunc i32 MemReleaseFromHeap(void* heap, void* target) {
    if (heap == 0) {
        heap = gHeap;
    }

    if (target == 0) { // Sage release
        return true;
    }

    u16 padding = MEM_READU16((u8*)target - 2, 0);
    u8* header = (u8*)target - padding - MEM_ALLOCATION_HEADER_SIZE;
    u32 offset = header - (u8*)heap;

    u16 startPage = MEM_READU16(header, MEM_ALLOCATION_startPage);
    u16 pageCount = MEM_READU16(header, MEM_ALLOCATION_pageCount);

    // Delegate to sub-allocator
#if MEM_ENABLE_SUBALLOCATORS
    if (pageCount == 0) { // No page count means it's a sub-allocator
        return MemInternal_SubRelease(heap, target);
    }
#endif


#if MEM_ENABLE_ALLOCGUARD
    PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicHead) == MEM_ALLOCACTION_MAGIC_MAIN);
    PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicTail) == MEM_ALLOCACTION_MAGIC_MAIN);
#endif

    // Clear the pages that where in use
    MemInternal_ReleasePages(heap, startPage, pageCount);
    MemInternal_ActivePagesRemoved((u8*)heap, pageCount);

#if MEM_ENABLE_ALLOCGUARD
    MemInternal_RemoveAllocationFromActiveList(heap, header, MEM_ALLOCACTION_MAGIC_MAIN);
#else
    MemInternal_RemoveAllocationFromActiveList(heap, header);
#endif

#if MEM_ENABLE_ALLOCGUARD
    MEM_WRITEU32(header, MEM_ALLOCATION_magicHead, MEM_ALLOCACTION_MAGIC_RELEASED);
    MEM_WRITEU32(header, MEM_ALLOCATION_magicTail, MEM_ALLOCACTION_MAGIC_RELEASED);

    u8* write = header + MEM_ALLOCATION_magicHead + sizeof(u32);
    *write = '\0';
    write = header + MEM_ALLOCATION_magicTail + sizeof(u32);
    *write = '\0';
#endif

    return 1;
}

mem_cfunc u32 MemForEachAllocationOnHeap(void* heap, fpAllocationInfo callback, void* userData) {
    u32 numAllocations = 0;

    u32 allocationOffset = MEM_ACTIVEALLOCSOFFSET(heap);
    while (allocationOffset != 0) {
        u8* header = (u8*)heap + allocationOffset;
        u8* mem = header + MEM_ALLOCATION_HEADER_SIZE;

#if MEM_ENABLE_ALLOCGUARD
        PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicHead) != MEM_ALLOCACTION_MAGIC_RELEASED);
        PlatformAssert(MEM_READU32(header, MEM_ALLOCATION_magicTail) != MEM_ALLOCACTION_MAGIC_RELEASED);
#endif

        u32 firstPage = MEM_READU16(header, MEM_ALLOCATION_startPage);
        u32 numPages = MEM_READU16(header, MEM_ALLOCATION_pageCount);
        allocationOffset = MEM_READU32(header, MEM_ALLOCATION_allocNext); // Iterate to next element
        u64 _label = MEM_READU64(header, MEM_ALLOCATION_tag);
        const char* loc = (const char*)_label;

        if (callback != 0) {
            callback(numAllocations, mem, firstPage, numPages, (void*)_label, userData);
        }
        numAllocations += 1;
    }

    return numAllocations;
}

mem_cfunc void* MemClear(void* dst, u32 bytes) {
    return MemSet(dst, 0, bytes);
}

mem_cfunc void* MemReallocateOnHeap(void* heap, void* src, u32 newBytes, void* newTag) {
    if (src == 0) {
        return MemAllocateOnHeap(heap, newBytes, 4, newTag);
    }
    u16 padding = MEM_READU16((u8*)src - 2, 0);
    u8* header = (u8*)src - padding - MEM_ALLOCATION_HEADER_SIZE;

#if MEM_ENABLE_ALLOCGUARD
    u32 magicHead = MEM_READU32(header, MEM_ALLOCATION_magicHead);
    u32 magicTail = MEM_READU32(header, MEM_ALLOCATION_magicTail);

    PlatformAssert(magicHead != MEM_ALLOCACTION_MAGIC_RELEASED);
    PlatformAssert(magicTail != MEM_ALLOCACTION_MAGIC_RELEASED);
#endif

    u32 firstPage = MEM_READU16(header, MEM_ALLOCATION_startPage);
    u32 numPages = MEM_READU16(header, MEM_ALLOCATION_pageCount);
    if (numPages == 0) {
        numPages = 1;
    }

    u8* lastByte = MEM_GETPAGEPTR(heap, firstPage);
    lastByte += numPages * MEM_PAGE_SIZE;
    u32 maxAllocBytes = lastByte - (u8*)src;

    u32 bytesToCopy = newBytes;
    if (maxAllocBytes < bytesToCopy) {
        bytesToCopy = maxAllocBytes;
    }

    void* newMem = MemAllocateOnHeap(heap, newBytes, 4, newTag); // Realloc is always 4 byte aligned. Consider storing alignment in each allocation as well?
    if (bytesToCopy < newBytes) {
        MemSet((u8*)newMem + bytesToCopy, 0, newBytes - bytesToCopy);
    }
    MemCopy(newMem, src, bytesToCopy);
    MemRelease(src);

    return newMem;
}

mem_cfunc void* MemCopy(void* _dst, const void* _src, u32 bytes) {
    u8* dst = (u8*)_dst;
    const u8* src = (const u8*)_src;

    u32 delta = 0; // Check for overlap
    if ((u32)dst < (u32)src) {
        delta = (u32)src - (u32)dst;
    }
    else {
        delta = (u32)dst - (u32)src;
    }
    PlatformAssert(delta >= bytes, __LOCATION__);

    for (unsigned int b = 0; b < bytes; ++b) {
        dst[b] = src[b];
    }

    return dst;
}

mem_cfunc void* MemSet(void* _dst, u8 val, u32 bytes) {
    u8* dst = (u8*)_dst;

    for (unsigned int b = 0; b < bytes; ++b) {
        dst[b] = val;
    }

    return dst;
}

mem_cfunc i32 MemCompare(const void* _a, const void* _b, u32 bytes) {
    const u8* a = (const u8*)_a;
    const u8* b = (const u8*)_b;
    for (u32 i = 0; i < bytes; ++i) {
        if (a[i] < b[i]) {
            return -1;
        }
        else if (b[i] < a[i]) {
            return 1;
        }
    }

    return 0;
}

mem_cfunc void* MemMove(void* destination, const void* source, u32 num) {
    void* tmp = MemAlloc(num);
    MemCopy(tmp, source, num);
    MemCopy(destination, tmp, num);
    MemRelease(tmp);
    return destination;
}