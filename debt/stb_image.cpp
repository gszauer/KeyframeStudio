

#include "../platform/assert.h"
#include "../platform/memory.h"
#include "../platform/math.h"

#define STBI_ASSERT(cond)         PlatformAssert(cond, __LOCATION__)
#define STBI_MALLOC(sz)           MemAlloc(sz)
#define STBI_REALLOC(p,newsz)     MemRealloc(p,newsz)
#define STBI_FREE(p)              MemRelease(p)

#define STBI_NO_STDIO
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM

#define STBI_UINT_MAX 4294967295
#define STBI_INT_MAX 2147483647


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION