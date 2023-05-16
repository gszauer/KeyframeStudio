#include "draw2d.h"

#include "../platform/memory.h"
#include "../platform/math.h"
#include "../framework/mat4.h"
#include "../platform/graphics.h"
#include "../debt/stb_image.h"
#include "../platform/loader.h"
#include "../debt/stb_truetype.h"
#include "../debt/stb_rect_pack.h"
#include "../debt/stb_sprintf.h"
#include "vector.h"

#define DRAW2D_MAXVERTS 30000
#define DRAW2D_MAXINDICES 100000
#define DRAW2D_MAX_IMAGES 5
#define DRAW2D_TEXTURE_PACKER_ATLAS_SIZE 1024
#define DRAW2D_FONT_MAX_LINE_HEIGHT 512
#define DRAW2D_NUM_ELLIPSE_SEGMENTS 45

namespace Draw2D {
    namespace Internal {
        struct Image {
            u32 id; // Graphics System Texture ID
            u32 width;
            u32 height;
        };

        struct Vertex { // 36
            vec2 position;
            vec2 uv;
            vec4 color;
            u16 texture;
            u16 padding;
        };

        struct Buffer {
            u32 vboId;
            u32 iboId;
            u32 vaoId;
        };

        struct AsciiFontInstance {
            u32 lineHeight;
            f32 stbScale;
            f32 scaledLineGap;
            int advances[256];
            ivec4 renderRects[256];
            stbrp_rect packedGlyphs[256];
            struct AsciiFont* prototype;
        };

        struct AsciiFont {
            stbtt_fontinfo fontInfo;
            i32 unscaledLineGap;
            bool ownsMemory; // NOTE: Could stuff this into stbtt_fontinfo::userdata
            bool active; // NOTE: Same as above, make the pointer into a bit mask.
            Vector<AsciiFontInstance> instances;
        };

        struct State {
            Vector<vec4> clipStack;
            vec4 activeClip;

            Vector<AsciiFont> loadedFonts;
            Vector<Image> loadedImages;

            u32 fontAtlas;
            stbrp_context fontPacker;
            stbrp_node* fontPackerNodes;
            unsigned char* glyphBuffer;

            u32 designWidth;
            u32 designHeight;
            f32 designDpi;
            u32 displayWidth;
            u32 displayHeight;

            // Flushable
            Vertex* vertices;
            u32   numVertices;

            u16* indices;
            u32   numIndices;

            u32 boundImages[DRAW2D_MAX_IMAGES];
            u32 imgUniformSlots[DRAW2D_MAX_IMAGES];
            u32 numBoundImages;

            Buffer displayBuffer;

            u32 imageShader; // Draw a tinted image.
            //u32 imageShaderScreenSizeUniform;
            u32 imageShaderAttribPosition;
            u32 imageShaderAttribTexCoord;
            u32 imageShaderAttribBlendColor;
            u32 imageShaderAttribTexIndex;

            u32 shaderToDraw; // Originally, i was going to support a shape only shader, etc. Keeping this here, but not using it for anything
        };

        State* gState;

        inline vec2 MakeVertex(const vec2& position, const vec2& rotationPivot, float cosTheta, float sinTheta, const vec2& scale) {
            vec2 vertex = (position - rotationPivot) * scale;

            float x = (vertex.x * cosTheta) - (vertex.y * sinTheta);
            float y = (vertex.x * sinTheta) + (vertex.y * cosTheta);

            return vec2(x, y);
        }

        inline void SetVertexAttribs(Internal::Vertex* verts, u32 count, const vec2& uv, u16 texture, const vec4& color) {
            for (u32 i = 0; i < count; ++i) {
                verts[i].uv = uv;
                verts[i].texture = texture;
                verts[i].color = color;
            }
        }

        inline vec4 UpdateClipStack() {
            u32 globalClipLeft = 0;
            u32 globalClipTop = 0;
            u32 globalClipRight = gState->displayWidth;
            u32 globalClipBottom = gState->displayHeight;

            for (u32 i = 0, size = gState->clipStack.Count(); i < size; ++i) {
                vec4 clipRect = gState->clipStack[i];

                globalClipLeft = MathMaxF(clipRect.x, globalClipLeft);
                globalClipTop = MathMaxF(clipRect.y, globalClipTop);
                globalClipRight = MathMinF(globalClipLeft + clipRect.z, globalClipRight);
                globalClipBottom = MathMinF(globalClipTop + clipRect.w, globalClipBottom);
            }

            vec4 clip(globalClipLeft, globalClipTop, globalClipRight - globalClipLeft, globalClipBottom - globalClipTop);

            gState->activeClip = clip;
            return clip;
        }

        void FlushAllDrawCommands() {
            // Early out if there is nothing to flush
            if (gState->numIndices == 0) {
                return;
            }

            PlatformAssert(gState->numVertices != 0, __LOCATION__);
            if (gState->shaderToDraw == 0) {
                // No shader is bound, we shouldn't really get here....
                // Try to recover gracefully, but break in debug
                gState->numIndices = 0;
                gState->numVertices = 0;
                PlatformAssert(false, __LOCATION__);
                return;
            }

            // Grab an iter from the free list
            Buffer* iter = &gState->displayBuffer;

            // Update Vertex and index data
            GfxFillArrayBuffer(iter->vboId, gState->vertices, sizeof(Vertex) * gState->numVertices, false);
            GfxFillIndexBuffer(iter->iboId, gState->indices, gState->numIndices * sizeof(u16), GfxIndexTypeShort, false);

            float screenSize[3] = { (float)gState->displayWidth, (float)gState->displayHeight, (float)gState->designHeight };
            PlatformAssert(gState->numBoundImages < 10, __LOCATION__); // Can only do single digits

            // Fill out array objects for buffer, depending on what is being drawn
            if (gState->shaderToDraw == gState->imageShader) {
                GfxAddBufferToLayout(iter->vaoId, gState->imageShaderAttribPosition, iter->vboId, 2, sizeof(Vertex), GfxBufferTypeFloat32, 0);
                GfxAddBufferToLayout(iter->vaoId, gState->imageShaderAttribTexCoord, iter->vboId, 2, sizeof(Vertex), GfxBufferTypeFloat32, sizeof(float) * 2);
                GfxAddBufferToLayout(iter->vaoId, gState->imageShaderAttribBlendColor, iter->vboId, 4, sizeof(Vertex), GfxBufferTypeFloat32, sizeof(float) * (2 + 2));
                GfxAddBufferToLayout(iter->vaoId, gState->imageShaderAttribTexIndex, iter->vboId, 1, sizeof(Vertex), GfxBufferTypeInt16, sizeof(float) * (2 + 2 + 4));
                GfxAddIndexBufferToLayout(iter->vaoId, iter->iboId);

                //GfxSetUniform(gState->shaderToDraw, gState->imageShaderScreenSizeUniform, screenSize, GfxUniformTypeFloat3, 1);
                for (u32 i = 0; i < gState->numBoundImages; ++i) {
                    GfxSetUniformTexture(gState->shaderToDraw, gState->imgUniformSlots[i], gState->boundImages[i]);
                }
            }
            else {
                PlatformAssert(false, __LOCATION__);
            }

            // Submit the finalized draw call
            GfxDraw(0, 0, iter->vaoId, GfxDrawModeTriangles, 0, gState->numIndices, 1);

            // Reset the CPU side buffers now that the data lives on the GPU
            gState->numVertices = 0;
            gState->numIndices = 0;
            Internal::gState->numBoundImages = 1; // Bind font atlas always
            Internal::gState->boundImages[0] = Internal::gState->loadedImages[0].id;
        }

        void FlushIfNeeded(u32 numNewVerts, u32 numNewIndices, u32 shaderId) {
            // Flush if at vertex limit
            if (Internal::gState->numVertices + numNewVerts >= DRAW2D_MAXVERTS) {
                Internal::FlushAllDrawCommands(); // Will reset numRenderCommands to 0
            }

            // Flush if at index limit
            else if (Internal::gState->numIndices + numNewIndices >= DRAW2D_MAXINDICES) {
                Internal::FlushAllDrawCommands(); // Will reset numRenderCommands to 0
            }

            // Flush if the image shader isn't bound
            else if (Internal::gState->shaderToDraw != shaderId) {
                if (Internal::gState->shaderToDraw != 0) {
                    Internal::FlushAllDrawCommands();
                }
            }
            // Next flush will happen with the image shader
            Internal::gState->shaderToDraw = shaderId;
        }

        void ClearCodePoints(Internal::AsciiFontInstance* instance) {
            stbtt_fontinfo* info = &instance->prototype->fontInfo;
            for (u32 i = 0; i < 256; ++i) {
                instance->packedGlyphs[i].id = i;
                instance->packedGlyphs[i].was_packed = 0;

                instance->packedGlyphs[i].x = 0;
                instance->packedGlyphs[i].y = 0;
                instance->packedGlyphs[i].w = 0;
                instance->packedGlyphs[i].h = 0;

                instance->advances[i] = 0;
                instance->renderRects[i] = ivec4(0, 0, 0, 0);
            }
        }

        inline Internal::AsciiFontInstance* GetFontInstance(u32 fontIndex, u32 lineHeight) {
            Internal::AsciiFont* font = &Internal::gState->loadedFonts[fontIndex];
            PlatformAssert(font->active, __LOCATION__);

            Internal::AsciiFontInstance* instance = 0;
            for (u32 i = 0, size = font->instances.Count(); i < size; ++i) {
                if (font->instances[i].lineHeight == lineHeight) {
                    instance = &font->instances[i];
                    break;
                }
            }

            if (instance == 0) {
                u32 index = font->instances.Count();
                stbtt_fontinfo* info = &font->fontInfo;

                font->instances.PushBack();
                instance = &font->instances[index];
                instance->prototype = font;
                instance->lineHeight = lineHeight;
                instance->stbScale = stbtt_ScaleForPixelHeight(info, (float)lineHeight);
                instance->scaledLineGap = (float)font->unscaledLineGap * instance->stbScale;
                Internal::ClearCodePoints(instance);
            }

            return instance;
        }

        AsciiFontInstance* MakeSureGlyphIsCached(Internal::AsciiFontInstance* instance, unsigned char codePoint) {
            Internal::AsciiFont* font = instance->prototype;
            f32 stbScale = instance->stbScale;
            u32 lineHeight = instance->lineHeight;
            f32 dpi = gState->designDpi;

            stbrp_rect packedRect = instance->packedGlyphs[codePoint];
            if (!packedRect.was_packed) { // Glyph does not exist
                PlatformAssert(packedRect.id == codePoint, __LOCATION__);

                int ix0, ix1, iy0, iy1;
                stbtt_GetCodepointBitmapBox(&font->fontInfo, codePoint, stbScale, stbScale, &ix0, &iy0, &ix1, &iy1);
                int bitmapWidth = instance->packedGlyphs[codePoint].w = f32(ix1 - ix0) * dpi;
                int bitmapHeight = instance->packedGlyphs[codePoint].h = f32(iy1 - iy0) * dpi;
                instance->renderRects[codePoint] = ivec4(ix0, iy0, ix1, iy1);

                // Pad out by 1 pixel on left and right
                bitmapWidth += 2;
                bitmapHeight += 2;
                // Pad row to 4 bytes for openGL. 
                int alignTo4Bytes = (4 - (bitmapWidth % 4));
                bitmapWidth += alignTo4Bytes;

                int advance = 0;
                int leftBearing = 0;
                stbtt_GetCodepointHMetrics(&font->fontInfo, codePoint, &advance, &leftBearing);
                instance->advances[codePoint] = advance;

                instance->packedGlyphs[codePoint].w = bitmapWidth;
                instance->packedGlyphs[codePoint].h = bitmapHeight;
                packedRect.x = packedRect.y = packedRect.was_packed = 0;
                int result = stbrp_pack_rects(&Internal::gState->fontPacker, &instance->packedGlyphs[codePoint], 1);
                packedRect = instance->packedGlyphs[codePoint];

                if (!result) { // Reset all cache
                    FlushAllDrawCommands();
                    stbrp_init_target(&Internal::gState->fontPacker, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE, Internal::gState->fontPackerNodes, 4096);

                    for (u32 i = 0, size = gState->loadedFonts.Count(); i < size; ++i) {
                        Internal::AsciiFont* clearFont = &gState->loadedFonts[i];
                        clearFont->instances.~Vector(); // Reset instances
                        new (&clearFont->instances) Vector<Internal::AsciiFontInstance>();
                    }

                    PlatformAssert(font->instances.Count() == 0, __LOCATION__);
                    font->instances.PushBack();
                    instance = &font->instances[0];
                    instance->prototype = font;
                    instance->lineHeight = lineHeight;
                    instance->stbScale = stbScale = stbtt_ScaleForPixelHeight(&font->fontInfo, (float)lineHeight);
                    instance->scaledLineGap = (float)font->unscaledLineGap * stbScale;
                    ClearCodePoints(instance);

                    instance->packedGlyphs[codePoint].w = bitmapWidth;
                    instance->packedGlyphs[codePoint].h = bitmapHeight;
                    instance->packedGlyphs[codePoint].x = 0;
                    instance->packedGlyphs[codePoint].y = 0;
                    instance->packedGlyphs[codePoint].was_packed = 0;
                    int result = stbrp_pack_rects(&Internal::gState->fontPacker, &instance->packedGlyphs[codePoint], 1);
                    packedRect = instance->packedGlyphs[codePoint];
                }

                // Remove padding now that it's packed
                packedRect.w -= 2;
                packedRect.h -= 2;
                packedRect.w -= alignTo4Bytes;
                instance->packedGlyphs[codePoint] = packedRect;

                PlatformAssert(result == 1, __LOCATION__);

                stbtt_MakeCodepointBitmap(&font->fontInfo, gState->glyphBuffer,
                    bitmapWidth, bitmapHeight, bitmapWidth,
                    stbScale * dpi, stbScale * dpi, codePoint);

#if 0
                int bytes = 0; // Debug code to make sure stbtt_MakeCodepointBitmap drew each glyph properly
                unsigned char* png = stbi_write_png_to_mem(gState->glyphBuffer, bitmapWidth, bitmapWidth, bitmapHeight, 1, &bytes);
                char path[6] = { codePoint, '.', 'p', 'n', 'g', '\0' };
                WriteDebugFile(path, png, bytes);
                MemRelease(png); // It did
#endif

                GfxWriteToTexture(gState->fontAtlas, gState->glyphBuffer, GfxTextureFormatR8,
                    packedRect.x, packedRect.y, bitmapWidth, bitmapHeight);

            }

            PlatformAssert(packedRect.was_packed, __LOCATION__);

            return instance;
        }
    }

    void Initialize() {
        Internal::gState = (Internal::State*)MemAlloc(sizeof(Internal::State));
        MemSet(Internal::gState, 0, sizeof(Internal::State));

        Internal::gState->vertices = (Internal::Vertex*)MemAlloc(sizeof(Internal::Vertex) * DRAW2D_MAXVERTS);
        Internal::gState->indices = (u16*)MemAlloc(sizeof(u16) * DRAW2D_MAXINDICES);
        Internal::gState->numVertices = 0;
        Internal::gState->numIndices = 0;

        new (&Internal::gState->clipStack) Vector<vec4>(); // Just allocated, do what the constructor does
        new (&Internal::gState->loadedFonts) Vector<Internal::AsciiFont>(); // Just allocated, do what the constructor does
        new (&Internal::gState->loadedImages) Vector<Internal::Image>(); // Just allocated, do what the constructor does

        Internal::gState->fontAtlas = GfxCreateTexture(0, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE, GfxTextureFormatR8, GfxTextureFormatR8, false);
        GfxSetTextureSampler(Internal::gState->fontAtlas, GfxWrapClamp, GfxWrapClamp, GfxFilterLinear, GfxFilterNone, GfxFilterLinear);
        Internal::Image fontImage;
        fontImage.width = DRAW2D_TEXTURE_PACKER_ATLAS_SIZE;
        fontImage.height = DRAW2D_TEXTURE_PACKER_ATLAS_SIZE;
        fontImage.id = Internal::gState->fontAtlas;
        Internal::gState->loadedImages.PushBack(fontImage);
        Internal::gState->numBoundImages = 1; // Bind font atlas always
        Internal::gState->boundImages[0] = Internal::gState->loadedImages[0].id;

        Internal::gState->fontPackerNodes = (stbrp_node*)MemAlloc(sizeof(stbrp_node) * DRAW2D_TEXTURE_PACKER_ATLAS_SIZE * 2);
        stbrp_init_target(&Internal::gState->fontPacker, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE, Internal::gState->fontPackerNodes, DRAW2D_TEXTURE_PACKER_ATLAS_SIZE * 2);
        Internal::gState->glyphBuffer = (unsigned char*)MemAlloc(sizeof(unsigned char) * DRAW2D_TEXTURE_PACKER_ATLAS_SIZE * DRAW2D_TEXTURE_PACKER_ATLAS_SIZE);
        MemSet(Internal::gState->glyphBuffer, 0, sizeof(unsigned char) * DRAW2D_TEXTURE_PACKER_ATLAS_SIZE * DRAW2D_TEXTURE_PACKER_ATLAS_SIZE);

        Internal::UpdateClipStack();

        const char* vShader =
            "#version 300 es  \n"
            "precision highp float;\n"
            "precision highp int;\n"
            "in vec2 position;\n"
            "in vec2 texCoord;\n"
            "in vec4 blendColor;\n"
            "in int texIndex;\n"
            "out vec2 vTexCoord;\n"
            "out vec4 vBlendColor;\n"
            "flat out int vTexIndex;\n"
            "void main() {\n"
            "    vec2 ndc = vec2(position.x, 1.0 - position.y);\n"
            "    ndc = ndc * 2.0 - 1.0;\n"
            "    gl_Position = vec4(ndc, 0.0, 1.0);\n"
            "    vTexCoord = texCoord;\n"
            "    vBlendColor = blendColor;\n"
            "    vTexIndex = texIndex;\n"
            "}\n";

        // Note, consider making variations of this shader that have less in the switch satatments
        const char* fShader =
            "#version 300 es  \n"
            "precision highp float;\n"
            "precision highp int;\n"
            "flat in int vTexIndex;\n"
            "in vec2 vTexCoord;\n"
            "in vec4 vBlendColor;\n"
            "uniform sampler2D tex0;\n"
            "uniform sampler2D tex1;\n"
            "uniform sampler2D tex2;\n"
            "uniform sampler2D tex3;\n"
            "uniform sampler2D tex4;\n"
            "uniform sampler2D tex5;\n"
            "uniform sampler2D tex6;\n"
            "uniform sampler2D tex7;\n"
            "out vec4 outputColor;\n"
            "void main() {\n"
            "   vec4 texColor = vec4(0, 0, 0, 1);\n"
            "   switch (vTexIndex) {\n"
            "      case 0: texColor = vec4(vBlendColor.rgb, vBlendColor.a * texture(tex0, vTexCoord).r); break;\n"
            "      case 1: texColor = texture(tex1, vTexCoord) * vBlendColor; break;\n"
            "      case 2: texColor = texture(tex2, vTexCoord) * vBlendColor; break;\n"
            "      case 3: texColor = texture(tex3, vTexCoord) * vBlendColor; break;\n"
            "      case 4: texColor = texture(tex4, vTexCoord) * vBlendColor; break;\n"
            "      case 5: texColor = texture(tex5, vTexCoord) * vBlendColor; break;\n"
            "      case 6: texColor = texture(tex6, vTexCoord) * vBlendColor; break;\n"
            "      case 7: texColor = texture(tex7, vTexCoord) * vBlendColor; break;\n"
            "      case 8: texColor = vBlendColor; break;\n"
            "   }\n"
            "   outputColor = texColor;\n"
            "}\n";

        Internal::gState->imageShader = GfxCreateShader(vShader, fShader);
        //Internal::gState->imageShaderScreenSizeUniform = GfxGetUniformSlot(Internal::gState->imageShader, "screenSize");
        Internal::gState->imageShaderAttribPosition = GfxGetAttributeSlot(Internal::gState->imageShader, "position");
        Internal::gState->imageShaderAttribTexCoord = GfxGetAttributeSlot(Internal::gState->imageShader, "texCoord");
        Internal::gState->imageShaderAttribBlendColor = GfxGetAttributeSlot(Internal::gState->imageShader, "blendColor");
        Internal::gState->imageShaderAttribTexIndex = GfxGetAttributeSlot(Internal::gState->imageShader, "texIndex");

        PlatformAssert(DRAW2D_MAX_IMAGES < 10, __LOCATION__);
        char uniformName[] = { 't', 'e', 'x', '0', '\0' };
        for (u32 i = 0; i < DRAW2D_MAX_IMAGES; ++i) {
            uniformName[3] = '0' + i;
            Internal::gState->imgUniformSlots[i] = GfxGetUniformSlot(Internal::gState->imageShader, uniformName);
        }

        Internal::gState->displayBuffer.vboId = GfxCreateBuffer();
        Internal::gState->displayBuffer.iboId = GfxCreateBuffer();
        Internal::gState->displayBuffer.vaoId = GfxCreateVertexLayout(Internal::gState->imageShader);
    }

    void Shutdown() {
        GfxDestroyBuffer(Internal::gState->displayBuffer.vboId);
        GfxDestroyBuffer(Internal::gState->displayBuffer.iboId);
        GfxDestroyShaderVertexLayout(Internal::gState->displayBuffer.vaoId);

        MemRelease(Internal::gState->fontPackerNodes);
        MemRelease(Internal::gState->glyphBuffer);

        GfxDestroyShader(Internal::gState->imageShader);

        // Destroy font
        GfxDestroyTexture(Internal::gState->fontAtlas);
        for (u32 i = 0, size = Internal::gState->loadedFonts.Count(); i < size; ++i) {
            DestroyFont(i);
        }
        Internal::gState->loadedFonts.~Vector();

        // Cleanup textures
        // Image 0 is always the texture atlas, which is managed manually. Skip it.
        for (u32 i = 1, size = Internal::gState->loadedImages.Count(); i < size; ++i) {
            PlatformAssert(Internal::gState->loadedImages[i].id == 0, __LOCATION__);
            if (Internal::gState->loadedImages[i].id != 0) {
                GfxDestroyTexture(Internal::gState->loadedImages[i].id);
            }
        }
        Internal::gState->loadedImages.~Vector();
        Internal::gState->clipStack.~Vector();

        MemRelease(Internal::gState->vertices);
        MemRelease(Internal::gState->indices);
        MemRelease(Internal::gState);
    }

    u32 LoadImage(void* memory, u32 bytes, Interpolation interp) {
        i32 width = 0;
        i32 height = 0;
        i32 components = 0;

        unsigned char* img_data = stbi_load_from_memory((unsigned char*)memory, bytes, &width, &height, &components, 0);
        unsigned int format = (components == 4) ? GfxTextureFormatRGBA8 : GfxTextureFormatRGB8;
        u32 imageId = GfxCreateTexture(img_data, width, height, format, format, true);
        MemRelease(img_data);

        Vector<Draw2D::Internal::Image>& loadedImages = Internal::gState->loadedImages;
        // Loaded image 0 is always the texture atlas, skip it
        for (u32 i = 1, size = loadedImages.Count(); i < size; ++i) {
            if (loadedImages[i].id == 0) {
                loadedImages[i].id = imageId;
                loadedImages[i].width = width;
                loadedImages[i].height = height;

                return i;
            }
        }

        if (interp == Interpolation::Step) {
            GfxSetTextureSampler(imageId, GfxWrapClamp, GfxWrapClamp, GfxFilterNearest, GfxFilterNearest, GfxFilterNearest);
        }

        Internal::Image result = { imageId, (u32)width, (u32)height };
        Internal::gState->loadedImages.PushBack(result);
        return Internal::gState->loadedImages.Count() - 1;
    }

    Size GetImageSize(u32 imageId) {
        return Size(Internal::gState->loadedImages[imageId].width, Internal::gState->loadedImages[imageId].height);
    }

    void DestroyImage(u32 imageId) {
        u32 textureId = Internal::gState->loadedImages[imageId].id;
        GfxDestroyTexture(textureId);
        Internal::gState->loadedImages[imageId].id = 0;
    }

    OBB ImageTransform(f32 screenX, f32 screenY, f32 screenW, f32 screenH, f32 scaleX, f32 scaleY, f32 pivotX, f32 pivotY, f32 rotation) {
        OBB result = { 0 };

        float designScale = Internal::gState->designDpi;
        

        vec2 pivot(pivotX, pivotY);
        vec2 scale(MathAbsF(scaleX), MathAbsF(scaleY));
        vec2 screenSize(screenW, screenH);
        vec2 screenPos(screenX, screenY);

        float cosTheta = MathCos(rotation);
        float sinTheta = MathSin(rotation);

        vec2 p0 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p1 = (Internal::MakeVertex(vec2(screenSize.x, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p2 = (Internal::MakeVertex(vec2(screenSize.x, screenSize.y), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p3 = (Internal::MakeVertex(vec2(0, screenSize.y), pivot, cosTheta, sinTheta, scale) + screenPos);

        { // Fill out the sprites oriented bounding box
            vec2 rightAxis = p1 - p0;
            vec2 upAxis = p3 - p0;
            result.extents.x = len(rightAxis) / 2.0f;
            result.extents.y = len(upAxis) / 2.0f;
            result.rotation = angle(rightAxis, vec2(1, 0));

            //result.center = (p0 + p1 + p2 + p3) / 4.0f;
            result.center = p0 + rightAxis * 0.5f + upAxis * 0.5f;
        }

        return result;
    }

    OBB DrawImage(u32 image, f32 screenX, f32 screenY, f32 screenW, f32 screenH, f32 sourceX, f32 sourceY, f32 sourceW, f32 sourceH, f32 scaleX, f32 scaleY, f32 pivotX, f32 pivotY, f32 rotation, f32 blendR, f32 blendG, f32 blendB, f32 blendA) {
        OBB result = { 0 };

        Internal::FlushIfNeeded(4, 6, Internal::gState->imageShader);


        // Flush if no more images can be bound
        u32 textureId = Internal::gState->loadedImages[image].id;
        PlatformAssert(textureId != 0, __LOCATION__);
        float textureWidth = (float)Internal::gState->loadedImages[image].width;
        float textureHeight = (float)Internal::gState->loadedImages[image].height;

        bool found = false;
        u32 imageIndex = 0;
        for (u32 i = 0, size = Internal::gState->numBoundImages; i < size; ++i) {
            if (Internal::gState->boundImages[i] == textureId) {
                imageIndex = i;
                found = true;
                break;
            }
        }
        if (!found) {
            if (Internal::gState->numBoundImages >= DRAW2D_MAX_IMAGES) {
                Internal::FlushAllDrawCommands(); // Will reset numBoundImages to 0
            }
            imageIndex = Internal::gState->numBoundImages++;
        }
        Internal::gState->boundImages[imageIndex] = textureId;

        float designScale = Internal::gState->designDpi;


        u32 firstVertex = Internal::gState->numVertices;
        Internal::Vertex* verts = &Internal::gState->vertices[firstVertex];
        Internal::gState->numVertices += 4;

        vec2 pivot(pivotX, pivotY);
        vec2 scale(MathAbsF(scaleX), MathAbsF(scaleY));
        vec2 screenSize(screenW, screenH);
        vec2 screenPos(screenX, screenY);

        float cosTheta = MathCos(rotation);
        float sinTheta = MathSin(rotation);

        SetVertexAttribs(verts, 4, vec2(), imageIndex, vec4(blendR, blendG, blendB, blendA));
        vec2 p0 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        verts[0].uv = vec2(sourceX / textureWidth, sourceY / textureHeight);
        vec2 p1 = (Internal::MakeVertex(vec2(screenSize.x, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        verts[1].uv = vec2((sourceX + sourceW) / textureWidth, sourceY / textureHeight);
        vec2 p2 = (Internal::MakeVertex(vec2(screenSize.x, screenSize.y), pivot, cosTheta, sinTheta, scale) + screenPos);
        verts[2].uv = vec2((sourceX + sourceW) / textureWidth, (sourceY + sourceH) / textureHeight);
        vec2 p3 = (Internal::MakeVertex(vec2(0, screenSize.y), pivot, cosTheta, sinTheta, scale) + screenPos);
        verts[3].uv = vec2(sourceX / textureWidth, (sourceY + sourceH) / textureHeight);
        
        { // Fill out the sprites oriented bounding box
            vec2 rightAxis = p1 - p0;
            vec2 upAxis = p3 - p0;
            result.extents.x = len(rightAxis) / 2.0f;
            result.extents.y = len(upAxis) / 2.0f;
            result.rotation = angle(rightAxis, vec2(1, 0));

            //result.center = (p0 + p1 + p2 + p3) / 4.0f;
            result.center = p0 + rightAxis * 0.5f + upAxis * 0.5f;

        }

        if (Internal::gState->clipStack.Count() > 0) { // Clip bounding box against screen bounds
            f32 left = MathMinF(MathMinF(MathMinF(p0.x, p1.x), p2.x), p3.x);
            f32 top = MathMinF(MathMinF(MathMinF(p0.y, p1.y), p2.y), p3.y);
            f32 right = MathMaxF(MathMaxF(MathMaxF(p0.x, p1.x), p2.x), p3.x);
            f32 bottom = MathMaxF(MathMaxF(MathMaxF(p0.y, p1.y), p2.y), p3.y);

            vec4 clip = Internal::gState->activeClip;
            f32 clipLeft = clip.x; // This prob needs to be scaled
            f32 clipTop = clip.y;
            f32 clipRight = (clip.x + clip.z);
            f32 clipBotom = (clip.y + clip.w);

            // If any of these are true, the box is invisible
            if (left > clipRight || right < clipLeft || top > clipBotom || bottom < clipTop) {
                Internal::gState->numVertices -= 4;
                return result; // Reset and early out
            }
        }

        { //  After culling, move into design space, and then normalize to prep for shader
            vec2 screenSize(Internal::gState->displayWidth, Internal::gState->displayHeight);

            verts[0].position = p0 * designScale / screenSize;
            verts[1].position = p1 * designScale / screenSize;
            verts[2].position = p2 * designScale / screenSize;
            verts[3].position = p3 * designScale / screenSize;
        }

        if (scaleX < 0.0f) {
            float tmp = verts[0].uv.x;
            verts[0].uv.x = verts[1].uv.x;
            verts[1].uv.x = tmp;

            tmp = verts[2].uv.x;
            verts[2].uv.x = verts[3].uv.x;
            verts[3].uv.x = tmp;
        }

        if (scaleY < 0.0f) {
            float tmp = verts[0].uv.y;
            verts[0].uv.y = verts[2].uv.y;
            verts[2].uv.y = tmp;

            tmp = verts[1].uv.y;
            verts[1].uv.y = verts[3].uv.y;
            verts[3].uv.y = tmp;
        }

        u16* indices = &Internal::gState->indices[Internal::gState->numIndices];
        Internal::gState->numIndices += 6;

        bool ccw = true;
        /* This no longer matters. I made scale be always positive, and flip uv coords instead
        if ((scaleX < 0 && scaleY > 0) || (scaleY < 0 && scaleX > 0)) {
            ccw = false;
        }*/

        if (ccw) { // default
            indices[0] = firstVertex + 1;
            indices[1] = firstVertex + 3;
            indices[2] = firstVertex + 2;

            indices[3] = firstVertex + 1;
            indices[4] = firstVertex + 0;
            indices[5] = firstVertex + 3;
        }
        else {
            indices[0] = firstVertex + 1;
            indices[1] = firstVertex + 2;
            indices[2] = firstVertex + 3;

            indices[3] = firstVertex + 1;
            indices[4] = firstVertex + 3;
            indices[5] = firstVertex + 0;
        }


        return result;
    }

    void Begin(u32 designWidth, u32 designHeight, f32 designScale, u32 screenWidth, u32 screenHeight){
        GfxDisableDepthTest();
        GfxSetViewport(0, 0, screenWidth, screenHeight);

        PlatformAssert(Internal::gState->numVertices == 0, __LOCATION__);
        PlatformAssert(Internal::gState->numIndices == 0, __LOCATION__);
        Internal::gState->numBoundImages = 1; // Bind font atlas always
        Internal::gState->boundImages[0] = Internal::gState->loadedImages[0].id;

        Internal::gState->displayWidth = screenWidth;
        Internal::gState->displayHeight = screenHeight;

        Internal::gState->designWidth = designWidth;
        Internal::gState->designHeight = designHeight;
        Internal::gState->designDpi = designScale;

        Internal::UpdateClipStack();
    }

    void End()                                                                                                                                                {
        PlatformAssert(Internal::gState->clipStack.Count() == 0, __LOCATION__);
        while (Internal::gState->clipStack.Count() != 0) {
            PopClip();
        }

        Internal::FlushAllDrawCommands();
    }

    void EnableAlphaBlending() {
        Internal::FlushAllDrawCommands();
        u32 src = GfxBlendFuncSrcAlpha;
        u32 dst = GfxBlendFuncOneMinusSrcAlpha;
        u32 blend = GfxBlendEquationAdd;
        GfxSetBlendState(true, 0, dst, dst, blend, blend, src, src);
    }

    void DisableAlphaBlending() {
        Internal::FlushAllDrawCommands();
        u32 src = GfxBlendFuncSrcAlpha;
        u32 dst = GfxBlendFuncOneMinusSrcAlpha;
        u32 blend = GfxBlendEquationAdd;
        GfxSetBlendState(false, 0, dst, dst, blend, blend, src, src);
    }

    void PushClip(f32 x, f32 y, f32 w, f32 h) {
        Internal::FlushAllDrawCommands();
        Internal::gState->clipStack.PushBack(vec4(x, y, w, h));
        vec4 clip = Internal::UpdateClipStack();
        
        // Modify clip to be in window space
        float dpi = Internal::gState->designDpi;
        clip.y = Internal::gState->displayHeight - clip.y * dpi;
        clip.y -= clip.w * dpi; // And be bottom left coordinates

        GfxSetClipState(true, clip.x * dpi, clip.y, clip.z * dpi, clip.w * dpi);
    }

    void PopClip() {
        Internal::FlushAllDrawCommands();
        Internal::gState->clipStack.PopBack();
        vec4 clip = Internal::UpdateClipStack();
        
        // Modify clip to be in window space
        float dpi = Internal::gState->designDpi;
        clip.y = Internal::gState->displayHeight - clip.y * dpi;
        clip.y -= clip.w * dpi; // And be bottom left coordinates
        
        GfxSetClipState(Internal::gState->clipStack.Count() != 0, clip.x, clip.y, clip.z, clip.w);
    }

    u32 LoadFont(void* memory, u32 bytes, bool own) {
        Internal::AsciiFont* font = 0;
        u32 size = size = Internal::gState->loadedFonts.Count();
        for (u32 i = 0; i < size; ++i) { // The only place it's actually used as size
            if (!Internal::gState->loadedFonts[i].active) {
                font = &Internal::gState->loadedFonts[i];
                size = i; // Font index, not size.
                break;
            }
        }
        if (font == 0) {
            Internal::gState->loadedFonts.PushBack();
            font = &Internal::gState->loadedFonts[size]; // Used as font index, not size
        }

        PlatformAssert(font != 0, __LOCATION__);

        font->active = stbtt_InitFont(&font->fontInfo, (const unsigned char*)memory, 0) != 0;
        PlatformAssert(font->active, __LOCATION__);
        new (&font->instances) Vector<Internal::AsciiFontInstance>(); // Initialize vector
        font->ownsMemory = own; // If the memory is owned, DestroyFont calls free on stbtt_fontinfo::data, which is the memory argument being passed in.

        i32 ascent = 0;
        i32 descent = 0;
        i32 linegap = 0;
        stbtt_GetFontVMetrics(&font->fontInfo, &ascent, &descent, &linegap);
        font->unscaledLineGap = ascent - descent + linegap;

        return size; // It's the font index, not size. Just re-using the variable.
    }

    void DestroyFont(u32 fontIndex) {
        Internal::AsciiFont* font = &Internal::gState->loadedFonts[fontIndex];
        PlatformAssert(font->active, __LOCATION__);
        font->active = false;

        if (font->ownsMemory) {
            MemRelease(font->fontInfo.data);
            font->fontInfo.data = 0;
            font->ownsMemory = false;
        }
        font->instances.~Vector();
    }

    Rect MeasureSubString(u32 font, u32 pixelHeight, const char* string, u32 startIndex, u32 len) {
        if (string == 0) {
            return Rect();
        }
        Internal::AsciiFontInstance* instance = Internal::GetFontInstance(font, pixelHeight);
        PlatformAssert(instance != 0, __LOCATION__);
        f32 stbScale = instance->stbScale;
        vec2 carrat;
        vec2 reference;

        int kern = 0;
        float maxX = 0.0f;
        u32 index = 0;
        for (const char* iter = string; *iter != '\0'; ++iter, ++index) {
            char codePoint = *iter;

            if (index == startIndex) {
                reference = carrat;
            }

            if (index >= startIndex + len) {
                break;
            }

            if (codePoint == '\n') {
                if (carrat.x > maxX) {
                    maxX = carrat.x;
                }
                carrat.x = 0;
                carrat.y += instance->scaledLineGap;
                continue;
            }

            instance = MakeSureGlyphIsCached(instance, codePoint);
            PlatformAssert(instance->packedGlyphs[codePoint].was_packed, __LOCATION__);

            int advance = instance->advances[codePoint];
            carrat.x += f32(advance + kern) * stbScale;
        }

        if (carrat.x > maxX) {
            maxX = carrat.x;
        }

        return Rect(reference.x, reference.y, MathMaxF(maxX - reference.x, 0.0f),MathMaxF(carrat.y - reference.y + instance->scaledLineGap, 0.0f));
    }

    int FindStringIndex(u32 font, u32 pixelHeight, const char* string, const vec2& relativePoint) {
        Internal::AsciiFontInstance* instance = Internal::GetFontInstance(font, pixelHeight);
        PlatformAssert(instance != 0, __LOCATION__);
        f32 stbScale = instance->stbScale;

        vec2 carrat;
        int kern = 0;
        int index = 0;
        for (const char* iter = string; *iter != '\0'; ++iter, ++index) {
            char codePoint = *iter;

            if (codePoint == '\n') {
                carrat.x = 0;
                carrat.y += instance->scaledLineGap;
                continue;
            }

            instance = MakeSureGlyphIsCached(instance, codePoint);
            PlatformAssert(instance->packedGlyphs[codePoint].was_packed, __LOCATION__);

            int advance = instance->advances[codePoint];
            
            if (relativePoint.x >= carrat.x && relativePoint.x <= carrat.x + f32(advance + kern) * stbScale) {
                if (relativePoint.y >= carrat.y && relativePoint.y <= carrat.y + instance->scaledLineGap) {
                    float midPoint = carrat.x + (f32(advance + kern) * stbScale) * 0.5f;
                    return relativePoint.x < midPoint? index : index + 1;
                }
            }

            carrat.x += f32(advance + kern) * stbScale;
        }

        return -1;
    }

    Size MeasureString(u32 font, u32 pixelHeight, const char* string) {
        if (string == 0) {
            return Size();
        }
        Internal::AsciiFontInstance* instance = Internal::GetFontInstance(font, pixelHeight);
        PlatformAssert(instance != 0, __LOCATION__);
        f32 stbScale = instance->stbScale;
        
        vec2 carrat;
        int kern = 0;
        float maxX = 0.0f;
        for (const char* iter = string; *iter != '\0'; ++iter) {
            char codePoint = *iter;

            if (codePoint == '\n') {
                if (carrat.x > maxX) {
                    maxX = carrat.x;
                }
                carrat.x = 0;
                carrat.y += instance->scaledLineGap;
                continue;
            }

            instance = MakeSureGlyphIsCached(instance, codePoint);
            
            PlatformAssert(instance->packedGlyphs[codePoint].was_packed, __LOCATION__);

            int advance = instance->advances[codePoint];
            carrat.x += f32(advance + kern) * stbScale;
        }

        if (carrat.x > maxX) {
            maxX = carrat.x;
        }

        return Size(maxX, carrat.y + instance->scaledLineGap);
    }

    void DrawCodePoint(u32 fontIndex, u32 lineHeight, f32 x, f32 y, unsigned char codePoint, f32 blendR, f32 blendG, f32 blendB, f32 blendA) {
        PlatformAssert(lineHeight <= DRAW2D_FONT_MAX_LINE_HEIGHT, __LOCATION__);
        if (lineHeight > DRAW2D_FONT_MAX_LINE_HEIGHT) {
            lineHeight = DRAW2D_FONT_MAX_LINE_HEIGHT; // Enforce a max line height...
        }
        Internal::AsciiFont* font = &Internal::gState->loadedFonts[fontIndex];
        PlatformAssert(font->active, __LOCATION__);
        Internal::AsciiFontInstance* instance = Internal::GetFontInstance(fontIndex, lineHeight);
        PlatformAssert(instance != 0, __LOCATION__);
        f32 stbScale = instance->stbScale;
        PlatformAssert(stbScale > 0.0f, __LOCATION__);

        stbtt_fontinfo* info = &font->fontInfo;
        u32 fontAtlas = 0; // The font atlas is always bound to slot 0 of loaded images

        instance = MakeSureGlyphIsCached(instance, codePoint);
        PlatformAssert(instance->packedGlyphs[codePoint].was_packed, __LOCATION__);

        stbrp_rect packedGlyph = instance->packedGlyphs[codePoint];

        f32 displayX = x + instance->renderRects[codePoint].x;
        f32 displayY = y + instance->renderRects[codePoint].y;
        f32 displayW = (instance->renderRects[codePoint].z - instance->renderRects[codePoint].x);
        f32 displayH = (instance->renderRects[codePoint].w - instance->renderRects[codePoint].y);

        f32 scale = 1.0f;

        DrawImage(fontAtlas,
            displayX, displayY, displayW, displayH,
            packedGlyph.x, packedGlyph.y, packedGlyph.w, packedGlyph.h,
            scale, scale, 0, 0, 0, blendR, blendG, blendB, blendA);
    }

    void DrawString(u32 fontIndex, u32 lineHeight, f32 x, f32 y, const char* string, f32 blendR, f32 blendG, f32 blendB, f32 blendA) {
        if (string == 0) {
            return;
        }
        PlatformAssert(lineHeight <= DRAW2D_FONT_MAX_LINE_HEIGHT, __LOCATION__);
        if (lineHeight > DRAW2D_FONT_MAX_LINE_HEIGHT) {
            lineHeight = DRAW2D_FONT_MAX_LINE_HEIGHT; // Enforce a max line height...
        }
        Internal::AsciiFont* font = &Internal::gState->loadedFonts[fontIndex];
        PlatformAssert(font->active, __LOCATION__);
        Internal::AsciiFontInstance* instance = Internal::GetFontInstance(fontIndex, lineHeight);
        PlatformAssert(instance != 0, __LOCATION__);
        f32 stbScale = instance->stbScale;
        PlatformAssert(stbScale > 0.0f, __LOCATION__);
        
        stbtt_fontinfo* info = &font->fontInfo;

        vec2 carrat(x, y);
        u32 fontAtlas = 0; // The font atlas is always bound to slot 0 of loaded images

        //char last = '\0';
        for (const char* iter = string; *iter != '\0'; ++iter) {
            char codePoint = *iter;

            if (codePoint == '\n') {
                carrat.x = x;
                carrat.y += instance->scaledLineGap;
                continue;
            }
            
            instance = MakeSureGlyphIsCached(instance, codePoint);
            PlatformAssert(instance->packedGlyphs[codePoint].was_packed, __LOCATION__);

            int advance = instance->advances[codePoint];
            int kern = 0;// Disabled kerning to make font easier to measure. // stbtt_GetCodepointKernAdvance(info, last, codePoint);

            stbrp_rect packedGlyph = instance->packedGlyphs[codePoint];
            
            f32 displayX = carrat.x + instance->renderRects[codePoint].x;
            f32 displayY = carrat.y + instance->renderRects[codePoint].y;
            f32 displayW = (instance->renderRects[codePoint].z - instance->renderRects[codePoint].x);
            f32 displayH = (instance->renderRects[codePoint].w - instance->renderRects[codePoint].y);

            f32 scale = 1.0f;

            DrawImage(fontAtlas, 
                displayX, displayY, displayW, displayH,
                packedGlyph.x, packedGlyph.y, packedGlyph.w, packedGlyph.h,
                scale, scale, 0, 0, 0, blendR, blendG, blendB, blendA);

            carrat.x += f32(advance + kern) * stbScale;
        }
    }

    void DrawTriangle(f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, f32 r, f32 g, f32 b) {
        float designScale = Internal::gState->designDpi;
        vec2 screenSize(Internal::gState->displayWidth, Internal::gState->displayHeight);

        Internal::FlushIfNeeded(3, 3, Internal::gState->imageShader);

        u32 firstVertex = Internal::gState->numVertices;
        Internal::Vertex* verts = &Internal::gState->vertices[firstVertex];
        Internal::gState->numVertices += 3;

        vec2 pivot(0, 0);
        vec2 scale(1, 1);

        float cosTheta = 1.0f;// MathCos(0);
        float sinTheta = 0.0f;// MathSin(0);

        SetVertexAttribs(verts, 3, vec2(), 8, vec4(r, g, b, 1.0f));

        vec2 p0 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + vec2(x0, y0));
        vec2 p1 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + vec2(x1, y1));
        vec2 p2 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + vec2(x2, y2));

        if (Internal::gState->clipStack.Count() > 0) { // Clip bounding box against screen bounds
            f32 left = MathMinF(MathMinF(p0.x, p1.x), p2.x);
            f32 top = MathMinF(MathMinF(p0.y, p1.y), p2.y);
            f32 right = MathMaxF(MathMaxF(p0.x, p1.x), p2.x);
            f32 bottom = MathMaxF(MathMaxF(p0.y, p1.y), p2.y);

            vec4 clip = Internal::gState->activeClip;
            f32 clipLeft = clip.x; // This prob needs to be scaled
            f32 clipTop = clip.y;
            f32 clipRight = (clip.x + clip.z);
            f32 clipBotom = (clip.y + clip.w);

            // If any of these are true, the box is invisible
            if (left > clipRight || right < clipLeft || top > clipBotom || bottom < clipTop) {
                Internal::gState->numVertices -= 3;
                return; // Reset and early out
            }
        }

        { //  After culling, move into design space, and then normalize to prep for shader
            verts[0].position = p0 * designScale / screenSize;
            verts[1].position = p1 * designScale / screenSize;
            verts[2].position = p2 * designScale / screenSize;
        }

        u16* indices = &Internal::gState->indices[Internal::gState->numIndices];
        Internal::gState->numIndices += 3;

        indices[0] = firstVertex + 0;
        indices[1] = firstVertex + 2;
        indices[2] = firstVertex + 1;
    }

    void DrawRect(f32 _x, f32 _y, f32 _w, f32 _h,
        f32 tlR, f32 tlG, f32 tlB,
        f32 trR, f32 trG, f32 trB,
        f32 blR, f32 blG, f32 blB,
        f32 brR, f32 brG, f32 brB) {

        float designScale = Internal::gState->designDpi;
        vec2 screenSize(Internal::gState->displayWidth, Internal::gState->displayHeight);

        Internal::FlushIfNeeded(4, 6, Internal::gState->imageShader);

        u32 firstVertex = Internal::gState->numVertices;
        Internal::Vertex* verts = &Internal::gState->vertices[firstVertex];
        Internal::gState->numVertices += 4;

        vec2 pivot(0, 0);
        vec2 scale(1, 1);
        vec2 size(_w, _h);
        vec2 screenPos(_x, _y);

        float cosTheta = 1.0f;// MathCos(0);
        float sinTheta = 0.0f;// MathSin(0);

        SetVertexAttribs(verts, 4, vec2(), 8, vec4(1.0f, 1.0f, 1.0f, 1.0f));

        vec2 p0 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p1 = (Internal::MakeVertex(vec2(size.x, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p2 = (Internal::MakeVertex(vec2(size.x, size.y), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p3 = (Internal::MakeVertex(vec2(0, size.y), pivot, cosTheta, sinTheta, scale) + screenPos);

        verts[0].color = vec4(tlR, tlG, tlB, 1.0f);
        verts[1].color = vec4(trR, trG, trB, 1.0f);
        verts[2].color = vec4(blR, blG, blB, 1.0f);
        verts[3].color = vec4(brR, brG, brB, 1.0f);

        if (Internal::gState->clipStack.Count() > 0) { // Clip bounding box against screen bounds
            f32 left = MathMinF(MathMinF(MathMinF(p0.x, p1.x), p2.x), p3.x);
            f32 top = MathMinF(MathMinF(MathMinF(p0.y, p1.y), p2.y), p3.y);
            f32 right = MathMaxF(MathMaxF(MathMaxF(p0.x, p1.x), p2.x), p3.x);
            f32 bottom = MathMaxF(MathMaxF(MathMaxF(p0.y, p1.y), p2.y), p3.y);

            vec4 clip = Internal::gState->activeClip;
            f32 clipLeft = clip.x; // This prob needs to be scaled
            f32 clipTop = clip.y;
            f32 clipRight = (clip.x + clip.z);
            f32 clipBotom = (clip.y + clip.w);

            // If any of these are true, the box is invisible
            if (left > clipRight || right < clipLeft || top > clipBotom || bottom < clipTop) {
                Internal::gState->numVertices -= 4;
                return; // Reset and early out
            }
        }

        { //  After culling, move into design space, and then normalize to prep for shader
            verts[0].position = p0 * designScale / screenSize;
            verts[1].position = p1 * designScale / screenSize;
            verts[2].position = p2 * designScale / screenSize;
            verts[3].position = p3 * designScale / screenSize;
        }

        u16* indices = &Internal::gState->indices[Internal::gState->numIndices];
        Internal::gState->numIndices += 6;

        indices[0] = firstVertex + 1;
        indices[1] = firstVertex + 3;
        indices[2] = firstVertex + 2;

        indices[3] = firstVertex + 1;
        indices[4] = firstVertex + 0;
        indices[5] = firstVertex + 3;
    }

    void DrawCircleSlice(f32 cX, f32 cY, float radius, float startAngle, float endAngle, float red, float green, float blue) {
        startAngle = MathFmod(startAngle, 2.0f * MATH_PI);
        endAngle = MathFmod(endAngle, 2.0f * MATH_PI);
        if (endAngle < startAngle) {
            endAngle += 2.0f * MATH_PI;
        }
        float fillAngle = endAngle - startAngle;

        float CIRCLE_RESOLUTION = 36;
        int numTriangles = MathCeil(fillAngle / (2.0f * MATH_PI) * CIRCLE_RESOLUTION);

        for (int i = 0; i < numTriangles; i++) {
            float angle1 = startAngle + (float)i * fillAngle / numTriangles;
            float angle2 = startAngle + (float)(i + 1) * fillAngle / numTriangles;

            vec2 vertex1 = vec2(cX + radius * MathCos(angle1), cY + radius * MathSin(angle1));
            vec2 vertex2 = vec2(cX + radius * MathCos(angle2), cY + radius * MathSin(angle2));

            DrawTriangle(cX, cY, vertex1.x, vertex1.y, vertex2.x, vertex2.y, red, green, blue);
        }
    }

    void DrawHollowCircle(f32 cx, f32 cy, f32 innerR, f32 outerR, f32 cr, f32 cg, f32 cb) {
        i32 num_segments = 32;
        for (int ii = 0; ii < num_segments; ii++) {
            float theta = 2.0f * MATH_PI * float(ii) / float(num_segments);
            float x = MathCos(theta);
            float y = MathSin(theta);

            vec2 this_vertex_outer(x * outerR + cx, y * outerR + cy);
            vec2 this_vertex_inner(x * innerR + cx, y * innerR + cy);
                                    
            theta = 2.0f * MATH_PI * float((ii + 1) % num_segments) / float(num_segments);
            x = MathCos(theta);
            y = MathSin(theta);

            vec2 next_vertex_outer(x * outerR + cx, y * outerR + cy);
            vec2 next_vertex_inner(x * innerR + cx, y * innerR + cy);

            DrawTriangle(
                this_vertex_outer.x, this_vertex_outer.y,
                this_vertex_inner.x, this_vertex_inner.y,
                next_vertex_outer.x, next_vertex_outer.y,
                cr, cg, cb);
            DrawTriangle(
                next_vertex_outer.x, next_vertex_outer.y,
                next_vertex_inner.x, next_vertex_inner.y,
                this_vertex_inner.x, this_vertex_inner.y,
                cr, cg, cb);
        }
    }

    void DrawRect(f32 _x, f32 _y, f32 _w, f32 _h, f32 blendR, f32 blendG, f32 blendB, f32 blendA, f32 scaleX, f32 scaleY, f32 pivotX, f32 pivotY, f32 rotation) {
        if (scaleX * scaleY == 0.0f) {
            return;
        }

        float designScale = Internal::gState->designDpi;
        vec2 screenSize(Internal::gState->displayWidth, Internal::gState->displayHeight);

        Internal::FlushIfNeeded(4, 6, Internal::gState->imageShader);

        u32 firstVertex = Internal::gState->numVertices;
        Internal::Vertex* verts = &Internal::gState->vertices[firstVertex];
        Internal::gState->numVertices += 4;

        vec2 pivot(pivotX, pivotY);
        vec2 scale(MathAbsF(scaleX), MathAbsF(scaleY));
        vec2 size(_w, _h);
        vec2 screenPos(_x, _y);

        float cosTheta = MathCos(rotation);
        float sinTheta = MathSin(rotation);

        SetVertexAttribs(verts, 4, vec2(), 8, vec4(blendR, blendG, blendB, blendA));
        vec2 p0 = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p1 = (Internal::MakeVertex(vec2(size.x, 0), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p2 = (Internal::MakeVertex(vec2(size.x, size.y), pivot, cosTheta, sinTheta, scale) + screenPos);
        vec2 p3 = (Internal::MakeVertex(vec2(0, size.y), pivot, cosTheta, sinTheta, scale) + screenPos);
        
        if (Internal::gState->clipStack.Count() > 0) { // Clip bounding box against screen bounds
            f32 left =   MathMinF(MathMinF(MathMinF(p0.x, p1.x), p2.x), p3.x);
            f32 top =    MathMinF(MathMinF(MathMinF(p0.y, p1.y), p2.y), p3.y);
            f32 right =  MathMaxF(MathMaxF(MathMaxF(p0.x, p1.x), p2.x), p3.x);
            f32 bottom = MathMaxF(MathMaxF(MathMaxF(p0.y, p1.y), p2.y), p3.y);

            vec4 clip = Internal::gState->activeClip;
            f32 clipLeft = clip.x; // This prob needs to be scaled
            f32 clipTop = clip.y;
            f32 clipRight = (clip.x + clip.z);
            f32 clipBotom = (clip.y + clip.w);

            // If any of these are true, the box is invisible
            if (left > clipRight || right < clipLeft || top > clipBotom || bottom < clipTop) {
                Internal::gState->numVertices -= 4;
                return; // Reset and early out
            }
        }

        { //  After culling, move into design space, and then normalize to prep for shader
            verts[0].position = p0 * designScale / screenSize;
            verts[1].position = p1 * designScale / screenSize;
            verts[2].position = p2 * designScale / screenSize;
            verts[3].position = p3 * designScale / screenSize;
        }

        u16* indices = &Internal::gState->indices[Internal::gState->numIndices];
        Internal::gState->numIndices += 6;
        
        indices[0] = firstVertex + 1;
        indices[1] = firstVertex + 3;
        indices[2] = firstVertex + 2;

        indices[3] = firstVertex + 1;
        indices[4] = firstVertex + 0;
        indices[5] = firstVertex + 3;
    }
    
    void DrawEllipse(f32 x, f32 y, f32 radiusX, f32 radiusY, f32 blendR, f32 blendG, f32 blendB, f32 blendA, f32 scaleX, f32 scaleY, f32 pivotX, f32 pivotY, f32 rotation) {
        if (scaleX * scaleY == 0.0f) {
            return;
        }

        float designScale = Internal::gState->designDpi;
        vec2 screenSize(Internal::gState->displayWidth, Internal::gState->displayHeight);

        Internal::FlushIfNeeded(DRAW2D_NUM_ELLIPSE_SEGMENTS + 1, DRAW2D_NUM_ELLIPSE_SEGMENTS * 3, Internal::gState->imageShader);

        vec2 pivot(pivotX, pivotY);
        vec2 scale(MathAbsF(scaleX), MathAbsF(scaleY));
        vec2 screenPos(x, y);

        float cosTheta = MathCos(rotation);
        float sinTheta = MathSin(rotation);

        u32 firstVertex = Internal::gState->numVertices;
        Internal::Vertex* vertices = &Internal::gState->vertices[firstVertex];
        Internal::gState->numVertices += DRAW2D_NUM_ELLIPSE_SEGMENTS + 1;

        SetVertexAttribs(vertices, DRAW2D_NUM_ELLIPSE_SEGMENTS + 1, vec2(), 8, vec4(blendR, blendG, blendB, blendA));

        vertices[0].position = (Internal::MakeVertex(vec2(0, 0), pivot, cosTheta, sinTheta, scale) + screenPos) * designScale / screenSize;

        for (u32 i = 1; i < DRAW2D_NUM_ELLIPSE_SEGMENTS + 1; ++i) {
            float theta = 2.0f * MathPI * float(i - 1) / float(DRAW2D_NUM_ELLIPSE_SEGMENTS - 1);
            float xPos = radiusX * MathCos(theta);
            float yPos = radiusY * MathSin(theta);

            vertices[i].position = (Internal::MakeVertex(vec2(xPos, yPos), pivot, cosTheta, sinTheta, scale) + screenPos) * designScale / screenSize;
        }

        if (Internal::gState->clipStack.Count() > 0) {
            vec4 clip = Internal::gState->activeClip;
            f32 clipLeft = clip.x;
            f32 clipTop =  clip.y;
            f32 clipRight = clipLeft + clip.z;
            f32 clipBotom =  clipTop + clip.w;

            f32 boundsLeft = (x - radiusX);
            f32 boundsTop = (y - radiusY);
            f32 boundsRight = (x + radiusX);
            f32 boundsBottom = (y + radiusY);

            // If any of these are true, the box is invisible
            if (boundsLeft > clipRight || boundsRight < clipLeft || boundsTop > clipBotom || boundsBottom < clipTop) {
                Internal::gState->numVertices -= DRAW2D_NUM_ELLIPSE_SEGMENTS + 1;
                return; // Reset and early out
            }
        }

        u16* indices = &Internal::gState->indices[Internal::gState->numIndices];
        Internal::gState->numIndices += DRAW2D_NUM_ELLIPSE_SEGMENTS * 3;
        u32 pivotVertex = firstVertex;
        for (u32 i = 0; i < DRAW2D_NUM_ELLIPSE_SEGMENTS; ++i) {
            u32 vertex = (firstVertex + 1) + i;
            u32 nextVertex = (firstVertex + 2) + i;
            if (i + 1 >= DRAW2D_NUM_ELLIPSE_SEGMENTS) {
                nextVertex = firstVertex + 1;
            }
            indices[i * 3 + 0] = pivotVertex;
            indices[i * 3 + 1] = nextVertex;
            indices[i * 3 + 2] = vertex;
        }
    }

    // https://forum.libcinder.org/topic/smooth-thick-lines-using-geometry-shader
    void DrawLine(f32* points, u32 numPoints, f32 thickness, f32 blendR, f32 blendG, f32 blendB, f32 blendA) {
        thickness *= 0.5f;

        PlatformAssert(numPoints >= 2, __LOCATION__);
        const u32 numVertsNeeded = numPoints * 2;
        const u32 numIndicesNeeded = (numPoints - 1) * 6;
        Internal::FlushIfNeeded(numVertsNeeded, numIndicesNeeded, Internal::gState->imageShader);
        
        float designScale = Internal::gState->designDpi;
        vec2 screenSize(Internal::gState->displayWidth, Internal::gState->displayHeight);

        vec4 color(blendR, blendG, blendB, blendA);

        u32 firstVertex = Internal::gState->numVertices;
        Internal::Vertex* vertices = &Internal::gState->vertices[firstVertex];
        Internal::gState->numVertices += numVertsNeeded;

        SetVertexAttribs(vertices, numVertsNeeded, vec2(), 8, vec4(blendR, blendG, blendB, blendA));

        vec2 p0(points[0], points[1]);
        vec2 p1(points[2], points[3]);
        vec2 line01 = p1 - p0;
        vec2 dir01 = normalized(line01);
        vec2 norm01(-dir01.y, dir01.x);

        vec2 debug1 = p0 - norm01 * thickness;
        vec2 debug2 = p0 + norm01 * thickness;
        vertices[0].position = (p0 - norm01 * thickness) * designScale / screenSize;
        vertices[1].position = (p0 + norm01 * thickness) * designScale / screenSize;

        vec2 min = vertices[0].position;
        vec2 max = min;

        if (vertices[1].position.x < min.x) {
            min.x = vertices[1].position.x;
        }
        if (vertices[1].position.y < min.y) {
            min.y = vertices[1].position.y;
        }
        if (vertices[1].position.x > max.x) {
            max.x = vertices[1].position.x;
        }
        if (vertices[1].position.y > max.y) {
            max.y = vertices[1].position.y;
        }

        for (u32 i = 1; i < numPoints - 1; ++i) {
            p0 = vec2(points[(i - 1) * 2 + 0], points[(i - 1) * 2 + 1]);
            p1 = vec2(points[i * 2 + 0], points[i * 2 + 1]);
            vec2 p2(points[(i + 1) * 2 + 0], points[(i + 1) * 2 + 1]);
            
            line01 = p1 - p0;
            dir01 = normalized(line01);
            norm01 = vec2(-dir01.y, dir01.x);

            vec2 line12 = p2 - p1;
            vec2 dir12 = normalized(line12);
            vec2 norm12(-dir12.y, dir12.x);

            vec2 tangent = normalized(dir01 + dir12);
            vec2 miter(-tangent.y, tangent.x);
            f32 length = thickness;
            f32 dt = dot(miter, norm01);
            if (dt != 0.0f) {
                length = thickness / dot(miter, norm01);
            }

            vertices[i * 2 + 0].position = (p1 - miter * length) * designScale / screenSize;
            vertices[i * 2 + 1].position = (p1 + miter * length) * designScale / screenSize;

            if (vertices[i * 2 + 0].position.x < min.x) {
                min.x = vertices[i * 2 + 0].position.x;
            }
            if (vertices[i * 2 + 0].position.y < min.y) {
                min.y = vertices[i * 2 + 0].position.y;
            }
            if (vertices[i * 2 + 0].position.x > max.x) {
                max.x = vertices[i * 2 + 0].position.x;
            }
            if (vertices[i * 2 + 0].position.y > max.y) {
                max.y = vertices[i * 2 + 0].position.y;
            }

            if (vertices[i * 2 + 1].position.x < min.x) {
                min.x = vertices[i * 2 + 1].position.x;
            }
            if (vertices[i * 2 + 1].position.y < min.y) {
                min.y = vertices[i * 2 + 1].position.y;
            }
            if (vertices[i * 2 + 1].position.x > max.x) {
                max.x = vertices[i * 2 + 1].position.x;
            }
            if (vertices[i * 2 + 1].position.y > max.y) {
                max.y = vertices[i * 2 + 1].position.y;
            }
        }

        // This is acutally p1 and p2. I just didn't want to declare new vartiables here for these
        p0 = vec2(points[(numPoints - 2) * 2 + 0], points[(numPoints - 2) * 2 + 1]);
        p1 = vec2(points[(numPoints - 1) * 2 + 0], points[(numPoints - 1) * 2 + 1]);
        line01 = p1 - p0;
        dir01 = normalized(line01);
        norm01 = vec2(-dir01.y, dir01.x);

        vertices[numPoints * 2 - 2].position = (p1 - norm01 * thickness) * designScale / screenSize;
        vertices[numPoints * 2 - 1].position = (p1 + norm01 * thickness) * designScale / screenSize;

        if (vertices[numPoints * 2 - 2].position.x < min.x) {
            min.x = vertices[numPoints * 2 - 2].position.x;
        }
        if (vertices[numPoints * 2 - 2].position.y < min.y) {
            min.y = vertices[numPoints * 2 - 2].position.y;
        }
        if (vertices[numPoints * 2 - 2].position.x > max.x) {
            max.x = vertices[numPoints * 2 - 2].position.x;
        }
        if (vertices[numPoints * 2 - 2].position.y > max.y) {
            max.y = vertices[numPoints * 2 - 2].position.y;
        }

        if (vertices[numPoints * 2 - 1].position.x < min.x) {
            min.x = vertices[numPoints * 2 - 1].position.x;
        }
        if (vertices[numPoints * 2 - 1].position.y < min.y) {
            min.y = vertices[numPoints * 2 - 1].position.y;
        }
        if (vertices[numPoints * 2 - 1].position.x > max.x) {
            max.x = vertices[numPoints * 2 - 1].position.x;
        }
        if (vertices[numPoints * 2 - 1].position.y > max.y) {
            max.y = vertices[numPoints * 2 - 1].position.y;
        }
        
        if (Internal::gState->clipStack.Count() > 0) { 
            vec4 clip = Internal::gState->activeClip;
            f32 clipLeft = clip.x * designScale / screenSize.x;
            f32 clipTop = clip.y * designScale / screenSize.y;
            f32 clipRight = (clip.x + clip.z) * designScale / screenSize.x;
            f32 clipBotom = (clip.y +  clip.w) * designScale / screenSize.y;

            f32 boundsLeft = min.x;
            f32 boundsTop = min.y;
            f32 boundsRight = max.x;
            f32 boundsBottom = (max.y);

            // If any of these are true, the box is invisible
            if (boundsLeft > clipRight || boundsRight < clipLeft || boundsTop > clipBotom || boundsBottom < clipTop) {
                Internal::gState->numVertices -= numPoints * 2;
                return; // Reset and early out
            }
        }

        // Generate indices
        u16* indices = &Internal::gState->indices[Internal::gState->numIndices];
        Internal::gState->numIndices += numIndicesNeeded;

        u32 idx = 0;
        for (u32 i = 0; i < (numPoints - 1); ++i) {
            indices[idx++] = firstVertex + (2 * i) + 3;
            indices[idx++] = firstVertex + (2 * i) + 2;
            indices[idx++] = firstVertex + (2 * i) + 1;

            indices[idx++] = firstVertex + (2 * i) + 1;
            indices[idx++] = firstVertex + (2 * i) + 2;
            indices[idx++] = firstVertex + (2 * i) + 0;
        }
    }
}