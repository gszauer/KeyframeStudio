#include "../platform/assert.h"
#include "../framework/sort.h"

#define STBRP_SORT QSort
#define STBRP_ASSERT(x) PlatformAssert(x, __LOCATION__);

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#undef STB_RECT_PACK_IMPLEMENTATION