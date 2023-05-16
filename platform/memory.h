#ifndef _H_MEMORY_
#define _H_MEMORY_

#define MEM_ENABLE_ALLOCGUARD           0


#define MEM_PAGE_SIZE                   4096 // Each page is 4 KiB
#define MEM_ALLOCATOR_SIZE              48 // (u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32)
#if MEM_ENABLE_ALLOCGUARD
    #define MEM_ALLOCATION_HEADER_SIZE      30 // (u32, u16, u16, u32, u32, u64, u32, u16)
#else
    #define MEM_ALLOCATION_HEADER_SIZE      22 // (u16, u16, u32, u32, u64, u16)
#endif

#if _DEBUG
#ifndef PLATFORM_DEBUG
#define PLATFORM_DEBUG 1
#endif
#else
#ifndef PLATFORM_DEBUG
#define PLATFORM_DEBUG 0
#endif
#endif

#define MEM_MAX_PAGE_ALLOC_SIZE (MEM_PAGE_SIZE - MEM_ALLOCATION_HEADER_SIZE)
#define MEM_MAX_SUB1_ALLOC_SIZE (((MEM_PAGE_SIZE - sizeof(u32)) - (MEM_ALLOCATION_HEADER_SIZE * 8)) / 8)
#define MEM_MAX_SUB2_ALLOC_SIZE (((MEM_PAGE_SIZE - sizeof(u32)) - (MEM_ALLOCATION_HEADER_SIZE * 4)) / 4)
#define MEM_MAX_SUB3_ALLOC_SIZE (((MEM_PAGE_SIZE - sizeof(u32)) - (MEM_ALLOCATION_HEADER_SIZE * 2)) / 2)

typedef char i8;
typedef unsigned char u8;

typedef short i16;
typedef unsigned short u16;

typedef int i32;
typedef unsigned int u32;

typedef long long i64;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

#if defined(__APPLE__) && defined(__MACH__)
#define MEM_PLATFORM_APPLE 1
#define platform_t unsigned long long
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64) || defined(__WIN64__)
#define MEM_PLATFORM_WINDOWS 1
#define platform_t unsigned long long
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
#define MEM_PLATFORM_WINDOWS 1
#define platform_t unsigned long
#elif defined(__linux__)
#define MEM_PLATFORM_LINUX 1
#define platform_t unsigned long long
#else
#define MEM_PLATFORM_WASM 1
#define platform_t unsigned long
#endif

#ifndef __cplusplus
    typedef char mem_size_check_i8[(sizeof(i8) == 1) ? 1 : -1];
    typedef char mem_size_check_u8[(sizeof(i8) == 1) ? 1 : -1];
    typedef char mem_size_check_i16[(sizeof(i16) == 2) ? 1 : -1];
    typedef char mem_size_check_u16[(sizeof(u16) == 2) ? 1 : -1];
    typedef char mem_size_check_i32[(sizeof(i32) == 4) ? 1 : -1];
    typedef char mem_size_check_u32[(sizeof(u32) == 4) ? 1 : -1];
    typedef char mem_size_check_i64[(sizeof(i64) == 8) ? 1 : -1];
    typedef char mem_size_check_u64[(sizeof(u64) == 8) ? 1 : -1];
    typedef char mem_size_check_f32[(sizeof(f32) == 4) ? 1 : -1];
    typedef char mem_size_check_f64[(sizeof(f64) == 8) ? 1 : -1];
    #if MEM_PLATFORM_WASM
        #define mem_cfunc __attribute__ (( visibility( "default" ) )) extern
    #else
        #define mem_cfunc extern
    #endif
    #pragma error "Assuming c++"
#else
    static_assert (sizeof(i8) == 1, "i8 should be defined as a 1 byte type");
    static_assert (sizeof(u8) == 1, "u8 should be defined as a 1 byte type");
    static_assert (sizeof(i16) == 2, "i16 should be defined as a 2 byte type");
    static_assert (sizeof(u16) == 2, "u16 should be defined as a 2 byte type");
    static_assert (sizeof(i32) == 4, "i32 should be defined as a 4 byte type");
    static_assert (sizeof(u32) == 4, "u32 should be defined as a 4 byte type");
    static_assert (sizeof(i64) == 8, "i64 should be defined as an 8 byte type");
    static_assert (sizeof(u64) == 8, "u64 should be defined as an 8 byte type");
    static_assert (sizeof(f32) == 4, "f32 should be defined as a 4 byte type");
    static_assert (sizeof(f64) == 8, "f64 should be defined as an 8 byte type");
    #if MEM_PLATFORM_WASM
        #define mem_cfunc __attribute__ (( visibility( "default" ) )) extern "C"
    #else
        #define mem_cfunc extern "C"
    #endif
#endif

#ifndef __LOCATION__
#define mem_xstr(a) mem_str(a)
#define mem_str(a) #a
#if _DEBUG
#define __LOCATION__ ("On line " mem_xstr(__LINE__) ", in " __FILE__)
#else
#define __LOCATION__ ((const char*)0)
#endif
#endif // !__LOCATION__

// Note:
// I like the idea of having a block allocator. 
// This allocator would dedicate an entire page to a data type, and treat it as a free list

mem_cfunc void PrintDebugString(const char* str);

mem_cfunc void* MemPlatformAllocate(u32 bytes);
mem_cfunc void MemPlatformRelease(void* mem);

mem_cfunc void* MemInitializeHeap(void* heap, u32 heapSize);
mem_cfunc int MemShutdownHeap(void* heap);

mem_cfunc void* MemAllocateOnHeap(void* heap, u32 bytes, u32 alignment, void* tag);
mem_cfunc i32 MemReleaseFromHeap(void* heap, void* target);

mem_cfunc void* MemAllocate(u32 bytes, u32 alignment, void* tag);
mem_cfunc i32 MemRelease(void* target);

#define MemAlloc(bytes) MemAllocate(bytes, 4, (void*)(__LOCATION__))

mem_cfunc void* MemGetTag(void* _mem);
mem_cfunc void MemSetTag(void* _mem, void* tag);

mem_cfunc void* MemCopy(void* dst, const void* src, u32 bytes);
mem_cfunc void* MemReallocateOnHeap(void* heap, void* src, u32 newBytes, void* newTag);
mem_cfunc void* MemSet(void* dst, u8 val, u32 bytes);
mem_cfunc void* MemClear(void* dst, u32 bytes);
mem_cfunc i32   MemCompare(const void* a, const void* b, u32 bytes);
mem_cfunc void* MemMove(void* destination, const void* source, u32 num);

mem_cfunc void* MemReallocate(void* src, u32 newBytes, void* newTag);
#define MemRealloc(src, bytes) MemReallocate(src, bytes, (void*)(__LOCATION__))

// Debugging / Visualization API.
typedef void (*fpAllocationInfo)(u32 index, void* mem, u32 firstPage, u32 numPages, void* tag, void* userData);
mem_cfunc u32 MemForEachAllocationOnHeap(void* heap, fpAllocationInfo callback, void* userData);
mem_cfunc u32 MemForEachAllocation(fpAllocationInfo callback, void* userData);
mem_cfunc u32 MemGetCurrentNumPages(void* heap = 0);
mem_cfunc u32 MemGetMostPagesUsedAtOne(void* heap = 0);
mem_cfunc bool MemIsPageUsed(u32 pageIndex, void* heap = 0);

mem_cfunc u32* MemGetPageMask();
mem_cfunc u32 MemGetPageSize(); // In bytes
mem_cfunc u32 MemGetHeapSize(); // In bytes
mem_cfunc u32 MemGetNumOverheadPages();

void* operator new(platform_t, void* p);
#if MEM_PLATFORM_WASM
void operator delete(void* p) throw();
#endif

#endif