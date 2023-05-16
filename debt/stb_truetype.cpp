#include "../platform/memory.h"
#include "../platform/math.h"
#include "../platform/assert.h"

static int stb_truetype_strlen(char* c) {
    int result = 0;
    char* iter = c;
    int counter = 0;
    while (*iter != 0) {
        result += 1;
        ++iter;

        if (counter > 5000000) {
            return -1;
        }
    }

    return result;
}

#define STBTT_ifloor(x)   ((int) MathFloor(x))
#define STBTT_iceil(x)    ((int) MathCeil(x))
#define STBTT_sqrt(x)      MathSqrt(x)
#define STBTT_pow(x,y)     MathPow(x,y)
#define STBTT_fmod(x,y)    MathFmod(x,y)
#define STBTT_cos(x)       MathCos(x)
#define STBTT_acos(x)      MathACos(x)
#define STBTT_fabs(x)      MathAbsF(x)
#define STBTT_malloc(x,u)       ((void)(u),MemAlloc(x))
#define STBTT_free(x,u)         ((void)(u),MemRelease(x))
#define STBTT_assert(x)         PlatformAssert(x, __LOCATION__)
#define STBTT_strlen(x)         stb_truetype_strlen(x)
#define STBTT_memcpy(d, s, b)   MemCopy(d, s, b)
#define STBTT_memset(d, v, b)   MemSet(d, v, b)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#undef STB_TRUETYPE_IMPLEMENTATION