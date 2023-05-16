#ifndef _H_PLATFORMASSERT_
#define _H_PLATFORMASSERT_

// Mirrored from memory.h
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

#if _DEBUG
#ifndef PLATFORM_DEBUG
#define PLATFORM_DEBUG 1
#endif
#else
#ifndef PLATFORM_DEBUG
#define PLATFORM_DEBUG 0
#endif
#endif

#if _DEBUG
inline int PlatformAssert(bool cond, const char* msg) { // Always returns 0
#if MEM_PLATFORM_WASM
	if (!cond) {
			PrintDebugString(msg);
            __builtin_trap();
		}
#else
	if (!cond) {
		unsigned char* ptr = (unsigned char*)0;
		*ptr = 0;
	}
#endif
	return 0;
}

#else
#define PlatformAssert(x, m)
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

#endif // !_H_ASSERT_
