#ifndef _H_GRAPHICS_
#define _H_GRAPHICS_

typedef unsigned int u32;
typedef int i32;
typedef float f32;
static_assert (sizeof(u32) == 4, "u32 should be a 4 byte type");
static_assert (sizeof(f32) == 4, "f32 should be a 4 byte type");
static_assert (sizeof(void*) >= 4, "pointer should be >= 4 byte type");

#define GfxBufferTypeFloat32            0
#define GfxBufferTypeInt16              3
#define GfxBufferTypeInt32              5

#define GfxIndexTypeByte                1
#define GfxIndexTypeShort               2
#define GfxIndexTypeInt                 4

#define GfxTextureFormatRGB8            0
#define GfxTextureFormatRGBA8           1
#define GfxTextureFormatR32F            2
#define GfxTextureFormatRGB32F          3
#define GfxTextureFormatDepth           4
#define GfxTextureFormatR8              5 
// GfxTextureFormatDepth is only a target format, not a source format

#define GfxFilterNearest                0
#define GfxFilterLinear                 1
#define GfxFilterNone                   2

#define GfxWrapRepeat                   0
#define GfxWrapClamp                    1

#define GfxUniformTypeInt1              0
#define GfxUniformTypeInt2              1
#define GfxUniformTypeInt3              2
#define GfxUniformTypeInt4              3
#define GfxUniformTypeFloat1            4
#define GfxUniformTypeFloat2            5
#define GfxUniformTypeFloat3            6
#define GfxUniformTypeFloat4            7
#define GfxUniformTypeFloat9            8
#define GfxUniformTypeFloat16           9
#define GfxUniformTypeTexture           10

#define GfxDepthFuncAlways              0
#define GfxDepthFuncNever               1
#define GfxDepthFuncEqual               2
#define GfxDepthFuncLEqual              3
#define GfxDepthFuncGreater             4
#define GfxDepthFuncGEqual              5
#define GfxDepthFuncNotEqual            6
#define GfxDepthFuncLess                7

#define GfxCullFaceOff                  0
#define GfxCullFaceBack                 1
#define GfxCullFaceFront                2
#define GfxCullFaceFrontAndBack         3

#define GfxFaceWindCounterClockwise     0
#define GfxFaceWindClockwise            1

#define GfxBlendFuncZero                1
#define GfxBlendFuncOne                 2
#define GfxBlendFuncSrcColor            3
#define GfxBlendFuncOneMinusSrcColor    4
#define GfxBlendFuncDstColor            5
#define GfxBlendFuncOneMinusDstColor    6
#define GfxBlendFuncSrcAlpha            7
#define GfxBlendFuncOneMinusSrcAlpha    8
#define GfxBlendFuncDstAlpha            9
#define GfxBlendFuncOneMinusDstAlpha    10
#define GfxBlendFuncConstColor          11
#define GfxBlendFuncOneMinusConstColor  12
#define GfxBlendFuncConstAlpha          13
#define GfxBlendFuncOneMinusconstAlpha  14
#define GfxBlendFuncSrcAlphaSaturate    15

#define GfxBlendEquationAdd             0
#define GfxBlendEquationSubtract        1
#define GfxBlendEquationReverseSubtract 2
#define GfxBlendEquationMin             3
#define GfxBlendEquationMax             4

#define GfxDrawModePoints               0
#define GfxDrawModeLines                1
#define GfxDrawModeLineStrip            2
#define GfxDrawModeTriangles            3
#define GfxDrawModeTriangleStrip        4
#define GfxDrawModeTriangleFan          5

extern "C" u32 GfxCreateBuffer();
extern "C" void GfxDestroyBuffer(u32 bufferId);

extern "C" void GfxFillArrayBuffer(u32 bufferId, void* input, u32 bytes, bool _static);
extern "C" void GfxFillIndexBuffer(u32 bufferId, void* input, u32 bytes, u32 indexType, bool _static);

inline u32 GfxCreateArrayBuffer(void* input, u32 bytes, bool _static) {
    u32 buffer = GfxCreateBuffer();
    GfxFillArrayBuffer(buffer, input, bytes, _static);
    return buffer;
}

inline u32 GfxCreateIndexBuffer(void* input, u32 bytes, u32 indexType, bool _static = true) {
    u32 buffer = GfxCreateBuffer();
    GfxFillIndexBuffer(buffer, input, bytes, indexType, _static);
    return buffer;
}

extern "C" u32 GfxCreateShader(const char* vsource, const char* fsource);
extern "C" void GfxDestroyShader(u32 shaderId);

extern "C" i32 GfxGetUniformSlot(u32 shaderId, const char* name);
extern "C" i32 GfxGetAttributeSlot(u32 shaderId, const char* name);

extern "C" u32 GfxCreateShaderVertexLayout(u32 shaderId);
#define GfxCreateVertexLayout GfxCreateShaderVertexLayout
extern "C" void GfxAddBufferToLayout(u32 layoutId, i32 attribSlot, u32 bufferId, u32 numComponents, u32 strideBytes, u32 bufferType, u32 dataOffsetBytes);
extern "C" void GfxAddBufferToLayoutByName(u32 layoutId, const char* name, u32 bufferId, u32 numComponents, u32 strideBytes, u32 bufferType, u32 dataOffsetBytes);
extern "C" void GfxDestroyShaderVertexLayout(u32 layoutId);

inline void GfxAddIndexBufferToLayout(u32 layoutId, u32 bufferId) {
    GfxAddBufferToLayout(layoutId, -1, bufferId, 1, 0, GfxBufferTypeInt16, 0);
}

extern "C" u32 GfxCreateTexture(void* data, u32 width, u32 height, u32 sourceFormat, u32 targetFormat, bool genMips);
inline u32 GfxCreateDepthTexture(u32 width, u32 height) {
    return GfxCreateTexture(0, width, height, GfxTextureFormatDepth, GfxTextureFormatDepth, false);
}
extern "C" void GfxSetTextureSampler(u32 textureId, u32 wrapS, u32 wrapT, u32 minFilter, u32 mipFilter, u32 magFilter);
extern "C" void GfxDestroyTexture(u32 textureId);

// Data and texture format must match. IE, can't upload RGBA to an RGB texture
extern "C" void GfxWriteToTexture(u32 textureId, void* data, u32 dataAndTextureFormat, u32 x, u32 y, u32 w, u32 h);

extern "C" void GfxSetUniform(u32 shaderId, u32 uniformSlot, void* data, u32 uniformType, u32 count); 
inline void GfxSetUniformTexture(u32 shaderId, u32 uniformSlot, u32 textureId) {
    GfxSetUniform(shaderId, uniformSlot, (void*)textureId, 10, 1);
}

extern "C" void GfxClearAll(u32 colorTargetId, u32 depthTargetId, float r, float g, float b, float d);
extern "C" void GfxClearColor(u32 colorTargetTextureId, u32 depthTargetTextureId, float r, float g, float b);
extern "C" void GfxClearDepth(u32 colorTargetTextureId, u32 depthTargetTextureId, float depth);
extern "C" void GfxDraw(u32 colorTargetTextureId, u32 depthTargetTextureId, u32 vertexLayoutId, u32 drawMode, u32 startIndex, u32 indexCount, u32 instanceCount);

extern "C" void GfxSetBlendState(bool blend, f32* optBlendColor, u32 blendDstRgb, u32 blendDstAlpha, u32 blendEquationRgb, u32 blendEquationAlpha, u32 blendSrcRgb, u32 blendSrcAlpha);
inline void GfxEnableAlphaBlending() {

}
extern "C" void GfxSetCullState(u32 cullFace, u32 faceWind);
extern "C" void GfxSetDepthState(bool enable, u32 depthFunc, f32* depthRange);
inline void GfxEnableDepthTest() {
    GfxSetDepthState(true, GfxDepthFuncLess, 0);
}
inline void GfxDisableDepthTest() {
    GfxSetDepthState(false, GfxDepthFuncLess, 0);
}

// OpenGL style, bottom left / width and height
extern "C" void GfxSetScissorState(bool enable, u32 x, u32 y, u32 w, u32 h);
#define GfxSetClipState GfxSetScissorState
extern "C" void GfxSetWriteMask(bool r, bool g, bool b, bool a, bool depth);
extern "C" void GfxSetViewport(u32 x, u32 y, u32 w, u32 h);

// Initialize and shutdown functions exist, but you don't call them
// the platform does. The void* that's being passed in is platform
// specific, it contains whatever we need for OpenGL rendering 
extern "C" int GfxInitialize(void* in, void* out);
extern "C" void GfxShutdown(void* userData);
extern "C" void GfxFinish(); // This just calls glFinish, used to wait for vsynch on windows

// Need to call at least once for GfxDraw to start tracking stats
extern "C" void GfxEnableStats(bool enable);
// Returns the number of verts since this was last called.
extern "C" int GfxStatsIndexCount();
extern "C" int GfxStatsDrawCount(); 

#endif