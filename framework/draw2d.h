#ifndef _H_DRAW2D_
#define _H_DRAW2D_

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "../platform/math.h"
#include "../platform/memory.h"

namespace Draw2D {
    ////////////////////////////////////////////////////////////////
    // Core API
    ////////////////////////////////////////////////////////////////

    enum class Interpolation {
        Linear = 0,
        Step = 1
    };

    struct Size {
        float w;
        float h;

        inline Size(float _w = 0.0f, float _h = 0.0f) :
            w(_w), h(_h) { }
    };

    struct Rect {
        float x;
        float y;
        float w;
        float h;

        inline Rect(float _x = 0.0f, float _y = 0.0f, float _w = 0.0f, float _h = 0.0f) :
            x(_x), y(_y), w(_w), h(_h) { }
    };

    struct OBB {
        vec2 center;
        vec2 extents;
        float rotation;

        inline bool Contains(const vec2& point) {
            vec2 rotVector = point - center;
            rotVector = rotate(rotVector, -rotation);

            vec2 localPoint = rotVector;// +extents;
            
            Rect localRect = { 0 };
            localRect.w = extents.x * 2.0f;
            localRect.h = extents.y * 2.0f;
            localRect.x = -extents.x;
            localRect.y = -extents.y;

            return localPoint.x >= localRect.x && localPoint.x <= localRect.x + localRect.w &&
                localPoint.y >= localRect.y && localPoint.y <= localRect.y + localRect.h;
        }
    };

    void Initialize(); 
    void Shutdown();

    void Begin(u32 designWidth, u32 designHeight, f32 designScale, u32 screenWidth, u32 screenHeight);
    void End(); 

    void EnableAlphaBlending();
    void DisableAlphaBlending();

    void PushClip(f32 x, f32 y, f32 w, f32 h);
    void PopClip();

    u32 LoadFont(void* memory, u32 bytes, bool autoCleanup = false); // If own is true, DestroyFont will call MemRelease on the memory pointer passed here
    void DestroyFont(u32 font);

    u32 LoadImage(void* memory, u32 bytes, Interpolation interp); // NOTE: maybe interpolation should be "bool useNearest"
    Size GetImageSize(u32 image);
    void DestroyImage(u32 image); 

    void DrawString(u32 font, u32 pixelHeight, f32 x, f32 y, const char* string, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f);
    void DrawCodePoint(u32 font, u32 pixelHeight, f32 x, f32 y, unsigned char codePoint, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f);
    int FindStringIndex(u32 font, u32 pixelHeight, const char* string, const vec2& relativePoint);
    Size MeasureString(u32 font, u32 pixelHeight, const char* string);
    Rect MeasureSubString(u32 font, u32 pixelHeight, const char* string, u32 startIndex, u32 len);
    OBB DrawImage(u32 image, f32 screenX, f32 screenY, f32 screenW, f32 screenH, f32 sourceX, f32 sourceY, f32 sourceW, f32 shourceH, f32 scaleX = 1.0f, f32 scaleY = 1.0f, f32 pivotX = 0.0f, f32 pivotY = 0.0f, f32 rotation = 0.0f, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f);
    OBB ImageTransform(f32 screenX, f32 screenY, f32 screenW, f32 screenH, f32 scaleX = 1.0f, f32 scaleY = 1.0f, f32 pivotX = 0.0f, f32 pivotY = 0.0f, f32 rotation = 0.0f);
    void DrawLine(f32* points, u32 numPoints, f32 thickness = 1.0f, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f);
    void DrawEllipse(f32 x, f32 y, f32 radiusX, f32 radiusY, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f, f32 scaleX = 1.0f, f32 scaleY = 1.0f, f32 pivotX = 0.0f, f32 pivotY = 0.0f, f32 rotation = 0.0f);
    void DrawRect(f32 x, f32 y, f32 w, f32 h, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f, f32 scaleX = 1.0f, f32 scaleY = 1.0f, f32 pivotX = 0.0f, f32 pivotY = 0.0f, f32 rotation = 0.0f);

    void DrawCircleSlice(f32 cX, f32 cY, float radius, float startAngle, float fillAngle, float red, float green, float blue);
    void DrawHollowCircle(f32 x, f32 y, f32 innerR, f32 outerR, f32 r, f32 g, f32 b);
    //void DrawImageWithTransform(u32 image, f32 screenX, f32 screenY, f32 screenW, f32 screenH, f32 sourceX, f32 sourceY, f32 sourceW, f32 shourceH, f32 scaleX = 1.0f, f32 scaleY = 1.0f, f32 pivotX = 0.0f, f32 pivotY = 0.0f, f32 rotationRadians = 0.0f, f32 blendR = 1.0f, f32 blendG = 1.0f, f32 blendB = 1.0f, f32 blendA = 1.0f);


    void DrawRect(f32 x, f32 y, f32 w, f32 h,
        f32 tlR, f32 tlG, f32 tlB,
        f32 trR, f32 trG, f32 trB,
        f32 blR, f32 blG, f32 blB,
        f32 brR, f32 brG, f32 brB);

    void DrawTriangle(f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, f32 r, f32 g, f32 b);
};

#endif