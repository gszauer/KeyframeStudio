#include "../platform/memory.h"
#include "../platform/assert.h"
#if MEM_PLATFORM_WASM
// https://stackoverflow.com/questions/72568387/why-is-an-objects-constructor-being-called-in-every-exported-wasm-function
extern "C" void __wasm_call_ctors(void);
__attribute__((export_name("_initialize")))
extern "C" void _initialize(void) {
    __wasm_call_ctors();
}

extern "C" void _cxa_pure_virtual() {
    PlatformAssert(false, __LOCATION__);
}

extern "C" void __cxa_pure_virtual() {
    PlatformAssert(false, __LOCATION__);
}

#define export __attribute__ (( visibility( "default" ) )) extern "C" 
#else
#define export extern "C"
#endif

#include "../platform/platform.h"
#include "../platform/memory.h"
#include "../debt/stb_vorbis.h"
#include "../debt/stb_image.h"
#include "../debt/stb_truetype.h"
#include "../framework/vec2.h"
#include "../framework/vec3.h"
#include "../framework/vec2.h"
#include "../debt/fast_obj.h"
#include "../framework/vector.h"
#include "../framework/draw2d.h"
#include "../debt/stb_sprintf.h"
#include "imgui.h"
#include "Node2D.h"
#include "Document.h"
#include "../framework/mat4.h"
#include "inter_ttf.inc"
#include "pixel_ttf.inc"
#include "material_ttf.inc"
#include "AnimLoader.h"

#if MEM_PLATFORM_WASM
#include "../platform/memory.cpp"
#include "../platform/window.cpp"
#include "../framework/sort.cpp"
#include "../framework/draw2d.cpp"
#include "../framework/vec2.cpp"
#include "../framework/vec3.cpp"
#include "../framework/Transform.cpp"
#include "../framework/quat.cpp"
#include "../framework/mat4.cpp"
#include "../debt/stb_vorbis.cpp"
#include "../debt/stb_image.cpp"
#include "../debt/stb_truetype.cpp"
#include "../debt/stb_rect_pack.cpp"
#include "../debt/stb_sprintf.cpp"
#include "AnimLoader.cpp"
#include "imgui.cpp"
#include "Node2D.cpp"
#include "Document.cpp"

WASM_LOADER_ENABLE_CALLBACKS
#endif

#define FONT_LOAD_ARENA_BYTES (1024 * 1024)
#define IMAGE_PREVIEW_SIZE 88.0f

#if _WIN32 & _DEBUG
#define DOCUMENT_NUM_UNDO_STEPS 100
#else
#define DOCUMENT_NUM_UNDO_STEPS 1000
#endif

enum class ActiveTool {
    None = 0,
    Move,
    Rotate,
    Scale,
    Pivot,
    Pan,
    Zoom,
    Grid
};

struct Application {
    Document* document;
    StyleSheet style;

    vec3 view; // x & y are viewport offsets, z is scale
    bool zoomIn;

    bool showSide;
    bool showBottom;

    u32 gridShader;
    u32 gridPositionAttrib;
    u32 gridOffsetUniform;
    u32 gridColor1Uniform;
    u32 gridColor2Uniform;
    u32 gridVbo;
    u32 gridVao;
    u32 gridIbo;

    u32 kbdHot;
    u32 kbdActive;
    u32 kbdGen;
    bool kbdHandled;
    bool ctrlLock;
    bool altLock;

    Imgui::HoldAreaDetails zoomDetails;
    i32 selectedMenu;

    bool moveWorld;
    bool moveSnap;
    f32 moveSnapVal;
    bool moveSelect;

    bool rotateSnap;
    f32 rotateSnapVal;
    bool rotateSelect;

    bool scaleSnap;
    f32 scaleSnapVal;
    bool scaleSelect;

    bool pivotWorld;
    bool pivotSnap;
    f32 pivotSnapVal;
    bool pivotSelect;

    bool gridShow;
    bool gridHighlight;
    f32 gridSize;

    u32 iconFont;
    u32 labelFont;
    u32 interfaceFont;

    u32 numFilesLoading;
    f32 msPerFrame[60];
    u32 msIter;

    float inspectorWidth;
    float inspectorSplit;
    float timelineSplit;
    float animationSplit;
    float drawerHeight;

    float hierarchyScroll;
    u32 hierarchyTab;
    u32 transformTab;
    u32 sceneTab;

    bool synchAnimView;
    float frameSideScroll;
    float frameUpScroll;

    float memoryScroll;
    bool memVisible;
    u32 fpsDisplay;

    bool playSelectedAnimation;
    i32  playingAnimationFrame;
    f32 playingAnimationTimer;

    i32 timelineTab;
    //vec3 camera;

    bool autoKey;

    ActiveTool activeTool;
    ActiveTool effector;
    vec2 effectorMousePos;
    Transform effectorTransform;
    Node2D* effectorNode;
    vec2 effectorAxis;

    inline void NewDocument() {

        effectorNode = 0;
        playSelectedAnimation = false;
        playingAnimationFrame = 0;
        playingAnimationTimer = 0;

        document->NewDocument();
    }

    inline void SelectTool(ActiveTool tool) {
        if (activeTool == tool) {
            activeTool = ActiveTool::None;
        }
        else {
            activeTool = tool;
        }
    }
};

struct FontUserData {
    Application* app;
    u32* target;
    const char* path;
    void* data;
};

void LoadFont(Application* app, u32* target, const char* path) {
    app->numFilesLoading += 1;

    void* loadArena = MemAlloc(FONT_LOAD_ARENA_BYTES + sizeof(FontUserData));
    FontUserData* fontData = (FontUserData*)((u8*)loadArena + FONT_LOAD_ARENA_BYTES);
    fontData->app = app;
    fontData->target = target;
    fontData->path = path;
    fontData->data = fontData + 1;

    LoadFileAsynch(path, loadArena, FONT_LOAD_ARENA_BYTES, [](const char* path, void* data, unsigned int bytes, void* userData) {
        FontUserData* fontData = (FontUserData*)((u8*)data + FONT_LOAD_ARENA_BYTES);
        Application* app = (Application*)userData;
        u32* target = fontData->target;

        PlatformAssert(data != 0, __LOCATION__);
        PlatformAssert(bytes != 0, __LOCATION__);

        *target = Draw2D::LoadFont(data, bytes, true);
        app->numFilesLoading -= 1;
        }, app);
}

void OpenFile(Application* app, void* data, u32 bytes);

export void* Initialize() {
    WindowUpdateTitle("Keyframe Studio");
    Draw2D::Initialize();
    GfxEnableStats(true);

    Application* app = (Application*)MemAlloc(sizeof(Application));
    MemClear(app, sizeof(Application));
    app->document = (Document*)MemAlloc(sizeof(Document));
    MemClear(app->document, sizeof(Document));
    new (app->document) Document(DOCUMENT_NUM_UNDO_STEPS);

    {
        app->gridShader = GfxCreateShader(
            "#version 300 es\n\
            precision highp float;                                   \n\
            in vec2 position;                                        \n\
            out vec2 fragCoord;                                      \n\
            void main() {                                            \n\
                vec2 ndc = vec2(position.x, 1.0 - position.y);       \n\
                ndc = ndc * 2.0 - 1.0;                               \n\
                gl_Position = vec4(ndc, 0.0, 1.0);                   \n\
                fragCoord = (sign(ndc) + vec2(1.0)) / vec2(2.0);     \n\
                fragCoord = vec2(fragCoord.x, 1.0 - fragCoord.y);    \n\
            }",
            "#version 300 es\n\
            precision highp float;                                                   \n\
            uniform vec4 offset; // xy(position), z(zoom), w(size)                   \n\
            uniform vec4 color1; // w is screen width (ie 1920)                      \n\
            uniform vec4 color2; // w is screen height (ie 1080)                     \n\
            out vec4 fragColor;                                                      \n\
            in vec2 fragCoord;                                                       \n\
            void main() {                                                            \n\
                vec2 uv = floor((vec2(fragCoord.x, fragCoord.y) * vec2(color1.w, color2.w) + offset.xy) / offset.w * (1.0f - offset.z + 0.1f));                 \n\
                float mask = mod(uv.x + mod(uv.y, 2.0), 2.0);                        \n\
                fragColor = vec4(mix(color1.rgb, color2.rgb, mask), 1.0);            \n\
            }"
        );

        app->gridPositionAttrib = GfxGetAttributeSlot(app->gridShader, "position");
        app->gridOffsetUniform = GfxGetUniformSlot(app->gridShader, "offset");
        app->gridColor1Uniform = GfxGetUniformSlot(app->gridShader, "color1");
        app->gridColor2Uniform = GfxGetUniformSlot(app->gridShader, "color2");;
    
        app->gridVbo = GfxCreateBuffer();
        app->gridIbo =  GfxCreateBuffer();
        app->gridVao = GfxCreateVertexLayout(app->gridShader);

        f32 vertices[8] = {
            /*0*/0.0f, 0.0f,         /*1*/1.0f, 0.0f,
            /*2*/0.0f, 1.0f,         /*3*/1.0f, 1.0f
        };

        u16 indices[6] = {
            0, 2, 1,
            3, 1, 2
        };

        GfxFillArrayBuffer(app->gridVbo, vertices, sizeof(f32) * 8, true);
        GfxFillIndexBuffer(app->gridIbo, indices, sizeof(u16) * 6, GfxIndexTypeShort, true);
        
        GfxAddBufferToLayout(app->gridVao, app->gridPositionAttrib, app->gridVbo, 2, 0, GfxBufferTypeFloat32, 0);
        GfxAddIndexBufferToLayout(app->gridVao, app->gridIbo);
    }

    app->memVisible = true;
    app->gridSize = 30;
    app->moveSnapVal = 10;
    app->scaleSnapVal = 10;
    app->rotateSnapVal = 10;

    app->showSide = true;
    app->showBottom = true;

    app->synchAnimView = false;
    app->zoomIn = true;

    for (i32 i = 0; i < 60; ++i) {
        app->msPerFrame[i] = 16.6f / 1000.0f;
    }
    app->fpsDisplay = 60;

    app->style.animationFrameWidth = 10;
    app->style.animationFrameHeight = 16;
    app->style.frameEven = ColorRGB8(80);
    app->style.frameOdd = ColorRGB8(90);

    app->style.gizmoR = ColorRGB8(150, 20, 20);
    app->style.gizmoG = ColorRGB8(20, 150, 20);
    app->style.gizmoB = ColorRGB8(20, 20, 150);
    app->style.gizmoY = ColorRGB8(150, 150, 20);

    app->style.gridA = ColorRGB8(60);
    app->style.gridB = ColorRGB8(70);

    app->style.gizmoR_Hover = ColorRGB8(230, 20, 20);
    app->style.gizmoG_Hover = ColorRGB8(20, 230, 20);
    app->style.gizmoB_Hover = ColorRGB8(20, 20, 230);
    app->style.gizmoY_Hover = ColorRGB8(220, 230, 20);

    app->style.menuBarHeight = 30.0f;
    app->style.footerHeight = 25.0f;
    app->style.toolbarWidth = 36.0f;
    app->style.inspectorMinWidth = 300;
    app->style.animatorMinHeight = 300;
    app->style.timelineMinWidth = 200.0f;
    app->style.menuTextLineHeight = 16;
    app->style.headerHeight = 27;
    app->style.headerFontSize = 14;
    app->style.listBoxItemHeight = 20;
    app->style.hierarchyLabelFontSize = 14;

    app->style.menuBarBg = ColorRGB8(60);
    app->style.footerBg = ColorRGB8(60);

    app->style.toolBarBg = ColorRGB8(66);
    app->style.tooltipIconBGColor = ColorRGB8(10, 10, 10);
    app->style.tooltipIconFGColor = ColorRGB8(240, 240, 240);

    app->style.tooltipTextColor = ColorRGB8(40);
    app->style.tooltipTextBGColor = ColorRGB8(180);

    app->style.keyframeDiamond = ColorRGB8(30);
    app->style.documentBGColor = ColorRGB8(40, 40, 40); // Checker board can be 50, or 45
    app->style.panelBgColor = ColorRGB8(70);
    app->style.dividerAColor = ColorRGB8(55);
    app->style.dividerBColor = ColorRGB8(85);
    app->style.dividerHotTint = 1.1f;
    app->style.dividerActiveTint = 0.8f;

    app->style.headerBgColor = ColorRGB8(50);
    app->style.hierarchyFooterBg = ColorRGB8(60);

    app->style.HeaderBGHotColor = ColorRGB8(55);
    app->style.HeaderBGActiveColor = ColorRGB8(60);

    app->style.hierarchyToggleHot = ColorRGB8(210);
    app->style.hierarchyToggleNormal = ColorRGB8(150);

    app->style.headerFontColor = ColorRGB8(190);
    app->style.hierarchyFooterButtonBG_Hot = ColorRGB8(60, 60, 60);
    app->style.hierarchyFooterButtonBorder_Hot = ColorRGB8(50, 50, 50);
    app->style.hierarchyFooterButtonBG_Active = ColorRGB8(70, 70, 70);
    app->style.hierarchyFooterButtonBorder_Active = ColorRGB8(40, 40, 40);
    app->style.hierarchyFooterButtonIcon = ColorRGB8(200, 200, 200);
    app->style.hierarchyFooterRemoveIcon = ColorRGB8(240, 180, 180);
    app->style.hierarchyFooterDisabledIconColor = ColorRGB8(40, 40, 40);
    app->style.scrollBarTrackBG = ColorRGB8(55);
    app->style.scrollBarHotButtonBg = ColorRGB8(150, 150, 150);
    app->style.scrollBarIconNormal = ColorRGB8(200, 200, 200);
    app->style.scrollBarIconHot = ColorRGB8(50, 50, 50);
    app->style.scrollGrabberNormal = ColorRGB8(120);
    app->style.scrollGrabberHot = ColorRGB8(180);
    app->style.karratColor = ColorRGB8(200);

    app->style.textAreaBg_AnimatedTint.r =
        app->style.textAreaOutline_AnimatedTint.r =
        app->style.textAreaFont_AnimatedTint.r = 0.05f;
    app->style.textAreaBg_AnimatedTint.g =
        app->style.textAreaBg_AnimatedTint.b =
        app->style.textAreaOutline_AnimatedTint.g =
        app->style.textAreaOutline_AnimatedTint.b =
        app->style.textAreaFont_AnimatedTint.g =
        app->style.textAreaFont_AnimatedTint.b = -0.05f;

    app->style.textAreaBg_InterpolatedTint.g =
        app->style.textAreaOutline_InterpolatedTint.g =
        app->style.textAreaFont_InterpolatedTint.g = 0.05f;
    app->style.textAreaBg_InterpolatedTint.r =
        app->style.textAreaBg_InterpolatedTint.b =
        app->style.textAreaOutline_InterpolatedTint.r =
        app->style.textAreaOutline_InterpolatedTint.b =
        app->style.textAreaFont_InterpolatedTint.r =
        app->style.textAreaFont_InterpolatedTint.b = -0.05f;

    app->style.textAreaBg_DirtyTint.b =
        app->style.textAreaOutline_DirtyTint.b =
        app->style.textAreaFont_DirtyTint.b = 0.05f;
    app->style.textAreaBg_DirtyTint.r =
        app->style.textAreaBg_DirtyTint.g =
        app->style.textAreaOutline_DirtyTint.r =
        app->style.textAreaOutline_DirtyTint.g =
        app->style.textAreaFont_DirtyTint.r =
        app->style.textAreaFont_DirtyTint.g = -0.05f;

    app->style.hierarchyLabel = ColorRGB8(200, 200, 200);
    app->style.hierarchyItemBG_A = ColorRGB8(70);
    app->style.hierarchyItemBG_B = ColorRGB8(65);
    app->style.hierarchyItemBG_Selected = ColorRGB8(10, 90, 130);
    app->style.hierarchyItemBG_Movable = ColorRGB8(130, 90, 10);

    app->style.fps1 = ColorRGB8(30, 180, 30);
    app->style.fps2 = ColorRGB8(180, 80, 30);
    app->style.fps3 = ColorRGB8(180, 30, 30);

    app->style.hierarchyLabelDisabled = ColorRGB8(70);
    app->style.hierarchyItemBGDisabled = ColorRGB8(100);

    app->style.textAreaBg_Normal = ColorRGB8(65);
    app->style.textAreaBg_Hot = ColorRGB8(75);
    app->style.textAreaBg_Active = ColorRGB8(60);
    app->style.textAreaBg_Disabled = app->style.panelBgColor;

    app->style.textAreaFont_Normal = ColorRGB8(190);
    app->style.textAreaFont_Hot = ColorRGB8(210);
    app->style.textAreaFont_Active = ColorRGB8(170);
    app->style.textAreaFont_Disabled = ColorRGB8(120);

    app->style.textAreaOutline_Normal = ColorRGB8(40);
    app->style.textAreaOutline_Hot = ColorRGB8(40);
    app->style.textAreaOutline_Active = ColorRGB8(20);
    app->style.textAreaOutline_Disabled = ColorRGB8(110);

    app->style.textAreaLabel_Normal = ColorRGB8(170);
    app->style.textAreaLabel_Hot = ColorRGB8(190);
    app->style.textAreaLabel_Active = ColorRGB8(30);
    app->style.textAreaLabel_Disabled = ColorRGB8(120);

    app->style.memoryFreePage = ColorRGB8(30, 150, 30);
    app->style.memoryPageInUse = ColorRGB8(150, 30, 30);

    app->style.toggleButtonBorder = app->style.textAreaOutline_Normal;
    app->style.toggleButtonDisabledBorder = app->style.textAreaOutline_Disabled;

    app->style.toggleButtonNormal = app->style.textAreaBg_Normal;
    app->style.toggleButtonHot = app->style.textAreaBg_Hot;
    app->style.toggleButtonActive = app->style.textAreaBg_Active;
    app->style.toggleButtonDisabled = app->style.textAreaBg_Disabled;

    app->style.textAreaSelectionColor = ColorRGB8(10, 90, 130);
    app->style.labelFontColorNormal = app->style.textAreaLabel_Normal;
    app->style.labelFontColorDisabled = app->style.textAreaLabel_Disabled;

    app->style.imageBlockOutlineColor = app->style.textAreaOutline_Normal;
    app->style.imageBlockBackgroundColor = app->style.textAreaBg_Normal;
    app->style.imageBlockLabelColor = app->style.textAreaLabel_Normal;

    app->inspectorWidth = app->style.inspectorMinWidth;
    app->inspectorSplit = 0.9f; // Making these big is an easy way to make sure they have enough space.
    app->animationSplit = 0.9f;
    app->timelineSplit = 0.1f; // Make these small
    app->drawerHeight = app->style.animatorMinHeight;
    app->style.hierarchyFooterHeight = app->style.headerHeight;
    app->style.hierarchyFooterButtonSize = 20;
    app->style.scrollBarSize = 20;
    app->style.scrollIconSize = 18;
    app->style.scrollBarHandleSize = 30;

    app->style.txtAreaHeight = 20;
    app->style.textAreaFontSize = 14;
    app->style.textAreaLabelSize = 9;

    app->numFilesLoading += 1; // Increase so none of the load functions end
    app->interfaceFont = Draw2D::LoadFont(inter_ttf, inter_ttf_len, false);
    app->iconFont = Draw2D::LoadFont(material_ttf, material_ttf_len, false);
    app->labelFont = Draw2D::LoadFont(pixel_ttf, pixel_ttf_len, false);

#if MEM_PLATFORM_WASM || _DEBUG
    app->numFilesLoading += 1; // Increase so none of the load functions end
    void* loadArena = MemAlloc(1024 * 1024);
    LoadFileAsynch("assets/sample.kfs", loadArena, 1024 * 1024, [](const char* path, void* data, unsigned int bytes, void* userData) {
        Application* app = (Application*)userData;
        OpenFile(app, data, bytes); // Calls MEmRelease(data)
        app->numFilesLoading -= 1; // Increase so none of the load functions end
    }, app);
#endif
    
    return app;
}

export void Update(float dt, void* userData) {
    Application* app = (Application*)userData;

    app->msPerFrame[app->msIter] = dt;
    if (++app->msIter >= 60) {
        app->msIter = 0;
    }

    if (app->numFilesLoading == 1) {
        Imgui::Initialize(app->interfaceFont, app->iconFont, app->labelFont);
        app->numFilesLoading -= 1; // Artificial file i added in init.
    }
    if (app->numFilesLoading != 0) {
        return;
    }

    Imgui::TickFrame(dt);

    if (app->playSelectedAnimation) {
        Animation* timelineAnim = app->document->GetTimelineAnimation();
        f32 duration = timelineAnim->GetDuration();
        if (timelineAnim != 0 && duration > 0.0f) {
            app->playingAnimationTimer += dt;
            if (app->playingAnimationTimer >= duration) {
                if (timelineAnim->loop == AnimationLoopMode::Looping) {
                    while (app->playingAnimationTimer >= duration) {
                        app->playingAnimationTimer -= duration;
                    }
                    PlatformAssert(app->playingAnimationTimer > 0, __LOCATION__);
                }
                else {
                    app->playingAnimationTimer = duration;
                }
            }

            float t = app->playingAnimationTimer / duration;
            float frame = t * (float)timelineAnim->frameCount;
            if (frame != app->playingAnimationFrame) {
                app->document->SetSelectedFrame(frame);
            }
            app->playingAnimationFrame = frame;
        }
    }
}

float FigureOutScrollBarHeight(float contentHeight, float scrollAreaHeight) {
    float scrollerHeight = 0.0f;
    if (contentHeight > scrollAreaHeight) {
        float ratio = MathFloor(contentHeight / scrollAreaHeight);

        if (ratio >= 5.0f) {
            scrollerHeight = scrollAreaHeight * 0.1f;
        }
        else if (ratio >= 4.0f) {
            scrollerHeight = scrollAreaHeight * 0.2f;
        }
        else if (ratio >= 3.0f) {
            scrollerHeight = scrollAreaHeight * 0.35f;
        }
        else if (ratio >= 2.0f) {
            scrollerHeight = scrollAreaHeight * 0.5f;
        }
        else if (ratio >= 1.0f) {
            scrollerHeight = scrollAreaHeight * 0.75f;
        }
        else if (ratio < 0.0f) {
            scrollerHeight = 0.0f;
        }
        else {
            PlatformAssert(false, __LOCATION__);
        }
        if (scrollerHeight < 30.0f) {
            scrollerHeight = 30.0f;
        }
    }
    return scrollerHeight;
}

static void ImguiResources(const Imgui::Rect& scrollArea, Application* a) {
    // Draw footer
    StyleSheet* s = &a->style;

    Imgui::Rect listItemKarrat = scrollArea;
    listItemKarrat.h = s->listBoxItemHeight;
    {
        listItemKarrat.y -= (a->document->GetNumResources() * s->listBoxItemHeight - scrollArea.h) * a->hierarchyScroll;

        int i = 0;
        for (Resource* iter = a->document->ResourceIterator(0); iter != 0; iter = a->document->ResourceIterator(iter)) {
            const char* name = "[NULL]";
            if (iter->name != 0) {
                name = iter->name;
            }

            if (Imgui::UndoListItem(listItemKarrat, scrollArea, name, (i++) % 2, false, a->document->GetSelectedResource() == iter)) {
                a->document->SelectResource(iter);
            }

            listItemKarrat.y += s->listBoxItemHeight;
        }
    }

    // Fill any missing space
    float bottom = scrollArea.y + scrollArea.h;
    float delta = bottom - (listItemKarrat.y);
    if (delta > 0.0f) {
        Imgui::Rect fillMissing = listItemKarrat;
        fillMissing.h = delta;

        Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
        if (Imgui::ClickArea(fillMissing)) {
            a->document->SelectResource(0);
        }
    }

    // Draw scroll bar
    Imgui::Rect hierarchyScrollBar = scrollArea;
    hierarchyScrollBar.x = scrollArea.x + scrollArea.w;
    hierarchyScrollBar.w = s->scrollBarSize;
    float contentHeight = a->document->GetNumResources() * s->listBoxItemHeight;
    float scrollerHeight = FigureOutScrollBarHeight(contentHeight, scrollArea.h);
    a->hierarchyScroll = Imgui::VScroll(hierarchyScrollBar, a->hierarchyScroll, scrollerHeight, Imgui::HandleScroll(scrollArea));

    // Draw footer
    Imgui::Rect hierarchyFooter = scrollArea;
    hierarchyFooter.y = scrollArea.y + scrollArea.h;
    hierarchyFooter.h = s->hierarchyFooterHeight;
    hierarchyFooter.w = scrollArea.w + s->scrollBarSize;

    {
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, hierarchyFooter.h,
            s->hierarchyFooterBg.r, s->hierarchyFooterBg.g, s->hierarchyFooterBg.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y + 1, hierarchyFooter.w, 1,
            s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, 1,
            s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

        Imgui::Rect footerKarrat = hierarchyFooter;
        footerKarrat.x = hierarchyFooter.x + hierarchyFooter.w - s->hierarchyFooterButtonSize - 3;
        footerKarrat.y = hierarchyFooter.y + 5;
        footerKarrat.w = s->hierarchyFooterButtonSize;
        footerKarrat.h = s->hierarchyFooterButtonSize;

        {
            if (a->document->GetSelectedResource() == 0) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific offset
                iconRect.y += 3; // Footer specific offset
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_TRASHCAN, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_TRASHCAN, "Delete Resource", s->hierarchyFooterButtonIcon)) {
                    if (a->document->GetSelectedResource() != 0) {
                        a->document->DestroyResource(a->document->GetSelectedResource());
                    }
                }
            }

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_ADD_FILE, "Import Resource", s->hierarchyFooterButtonIcon)) {
                a->document->RequestResource([](const char* path, const Resource& resource, void* userData) {
                    // Nothing to really do here. If the function suceeds, then the resource
                    // is already registered. If the function fails, then nothing.
                    }, a);
            }

            { // Label
                char printString[36] = { 0 };
                stbsp_snprintf(printString, 36, "Num Resources: %d", a->document->GetNumResources());
                Draw2D::DrawString(a->interfaceFont, 12, hierarchyFooter.x + 5, hierarchyFooter.y + 20, printString,
                    s->hierarchyLabel.r, s->hierarchyLabel.g, s->hierarchyLabel.b);
            }
        }
    }
}

static void ImguiSynchAnimViewIcon(Imgui::Rect footerKarrat, Application* a, StyleSheet* s, bool selectTimeline) {
    bool activated = false;
    if (a->synchAnimView) {
        StyleColor c = s->hierarchyFooterButtonIcon;
        c.r *= 0.25f;
        c.g *= 0.55f;
        c.b *= 0.25f;
        if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_SYNC_ENABLED, "Sync Enabled (click to disbale)", c)) {
            a->synchAnimView = false;
            activated = true;
        }
    }
    else {
        StyleColor c = s->hierarchyFooterRemoveIcon;
        if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_SYNC_DISABLED, "Sync Disabled (click to enable)", c)) {
            a->synchAnimView = true;
            activated = true;
        }
    }

    if (a->synchAnimView) {
        if (selectTimeline) {
            // This happens all the time, has no bearing on selected frame
            if (a->document->GetTimelineAnimation() != a->document->GetSelectedAnimation()) {
                a->playSelectedAnimation = false;
                a->autoKey = false;
            }
            a->document->SelectTimeline(a->document->GetSelectedAnimation());
        }
        else {
            // This changes the selected frame.
            if (activated) {
                a->document->SelectAnimation(a->document->GetTimelineAnimation());
                a->playSelectedAnimation = false;
                a->autoKey = false;
            }
        }
    }
}

static void ImguiAnimations(const Imgui::Rect& scrollArea, Application* a) {
    // Draw footer
    StyleSheet* s = &a->style;

    Imgui::Rect listItemKarrat = scrollArea;
    listItemKarrat.h = s->listBoxItemHeight;
    {
        listItemKarrat.y -= (a->document->GetNumAnimations() * s->listBoxItemHeight - scrollArea.h) * a->frameUpScroll;

        int i = 0;
        for (Animation* iter = a->document->AnimationIterator(0); iter != 0; iter = a->document->AnimationIterator(iter)) {
            const char* name = "[NULL]";
            if (iter->name != 0) {
                name = iter->name;
            }

            if (Imgui::UndoListItem(listItemKarrat, scrollArea, name, (i++) % 2, false, a->document->GetSelectedAnimation() == iter)) {
                a->document->SelectAnimation(iter);
            }

            listItemKarrat.y += s->listBoxItemHeight;
        }
    }

    // Fill any missing space
    float bottom = scrollArea.y + scrollArea.h;
    float delta = bottom - (listItemKarrat.y);
    if (delta > 0.0f) {
        Imgui::Rect fillMissing = listItemKarrat;
        fillMissing.h = delta;

        Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
        if (Imgui::ClickArea(fillMissing)) {
            a->document->SelectAnimation(0);
        }
    }

    // Draw scroll bar
    Imgui::Rect hierarchyScrollBar = scrollArea;
    hierarchyScrollBar.x = scrollArea.x + scrollArea.w;
    hierarchyScrollBar.w = s->scrollBarSize;
    float contentHeight = a->document->GetNumAnimations() * s->listBoxItemHeight;
    float scrollerHeight = FigureOutScrollBarHeight(contentHeight, scrollArea.h);
    a->hierarchyScroll = Imgui::VScroll(hierarchyScrollBar, a->hierarchyScroll, scrollerHeight, Imgui::HandleScroll(scrollArea));

    // Draw footer
    Imgui::Rect hierarchyFooter = scrollArea;
    hierarchyFooter.y = scrollArea.y + scrollArea.h;
    hierarchyFooter.h = s->hierarchyFooterHeight;
    hierarchyFooter.w = scrollArea.w + s->scrollBarSize;

    {
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, hierarchyFooter.h,
            s->hierarchyFooterBg.r, s->hierarchyFooterBg.g, s->hierarchyFooterBg.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y + 1, hierarchyFooter.w, 1,
            s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, 1,
            s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

        Imgui::Rect footerKarrat = hierarchyFooter;
        footerKarrat.x = hierarchyFooter.x + hierarchyFooter.w - s->hierarchyFooterButtonSize - 3;
        footerKarrat.y = hierarchyFooter.y + 5;
        footerKarrat.w = s->hierarchyFooterButtonSize;
        footerKarrat.h = s->hierarchyFooterButtonSize;

        {
            if (a->document->GetSelectedAnimation() == 0) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific offset
                iconRect.y += 3; // Footer specific offset
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_TRASHCAN, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_TRASHCAN, "Delete animation", s->hierarchyFooterButtonIcon)) {
                    if (a->document->GetSelectedAnimation() != 0) {
                        Animation* oldSelected = a->document->GetSelectedAnimation();
                        if (a->document->GetTimelineAnimation() == oldSelected) {
                            a->document->SelectTimeline(0);
                        }
                        a->document->SelectAnimation(0);
                        a->autoKey = false;
                        a->document->DeleteAnimation(oldSelected);
                    }
                }
            }

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_ADDTRACK, "Create animation", s->hierarchyFooterButtonIcon)) {
                a->document->CreateAnimation();
            }

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (a->document->GetSelectedAnimation() == 0) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific
                iconRect.y += 3;
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_DESELECT, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_DESELECT, "Deselect animation", s->hierarchyFooterButtonIcon)) {
                    a->document->SelectAnimation(0);
                    a->autoKey = false;

                }
            }

            // Seperator
            footerKarrat.x -= 5;
            Draw2D::DrawRect((footerKarrat.x), hierarchyFooter.y + 2, 1, hierarchyFooter.h - 2,
                s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
            Draw2D::DrawRect((footerKarrat.x) + 1, hierarchyFooter.y + 2, 1, hierarchyFooter.h - 2,
                s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            ImguiSynchAnimViewIcon(footerKarrat, a, s, true);
        }
    }
}

static void ImguiKeyButton(Imgui::Rect& footerKarrat, Application* a, StyleSheet* s) {
    if (a->document->GetSelectedNode() == 0 || a->document->GetTimelineAnimation() == 0) {
        Imgui::Rect iconRect = footerKarrat;
        iconRect.x += 2; // Footer specific
        iconRect.y += 3;
        Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_KEY, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
    }
    else {
        u32 numPopupItems = 14;

        f32 popupMenuWidth = 200.0f;
        Imgui::Rect popupArea(footerKarrat.x + footerKarrat.w - popupMenuWidth / 2.0f,
            footerKarrat.y - (numPopupItems * s->listBoxItemHeight) * 2,
            popupMenuWidth, numPopupItems * s->listBoxItemHeight);

        i32 modal = Imgui::BeginModalPopup(popupArea, footerKarrat, numPopupItems);
        Imgui::PushModalPopupItem("Transform.Position.X");
        Imgui::PushModalPopupItem("Transform.Position.Y");
        Imgui::PushModalPopupItem("Transform.Rotation");
        Imgui::PushModalPopupItem("Transform.Scale.X");
        Imgui::PushModalPopupItem("Transform.Scale.Y");
        Imgui::PushModalPopupItem("Sprite.Tint.R");
        Imgui::PushModalPopupItem("Sprite.Tint.G");
        Imgui::PushModalPopupItem("Sprite.Tint.B");
        Imgui::PushModalPopupItem("Sprite.Visible");
        Imgui::PushModalPopupItem("Sprite.Source.X");
        Imgui::PushModalPopupItem("Sprite.Source.Y");
        Imgui::PushModalPopupItem("Sprite.Source.W");
        Imgui::PushModalPopupItem("Sprite.Source.H");
        Imgui::PushModalPopupItem("SortIndex");
        Imgui::EndModalPopup();

        if (modal != -1) {
            TrackType trackType = TrackType::TransformPositionX;
            if (modal == 0) {
                trackType = TrackType::TransformPositionX;
            }
            else if (modal == 1) {
                trackType = TrackType::TransformPositionY;
            }
            else if (modal == 2) {
                trackType = TrackType::TransformRotation;
            }
            else if (modal == 3) {
                trackType = TrackType::TransformScaleX;
            }
            else if (modal == 4) {
                trackType = TrackType::TransformScaleY;
            }
            else if (modal == 5) {
                trackType = TrackType::SpriteTintR;
            }
            else if (modal == 6) {
                trackType = TrackType::SpriteTintG;
            }
            else if (modal == 7) {
                trackType = TrackType::SpriteTintB;
            }
            else if (modal == 8) {
                trackType = TrackType::SpriteVisibility;
            }
            else if (modal == 9) {
                trackType = TrackType::SpriteSourceX;
            }
            else if (modal == 10) {
                trackType = TrackType::SpriteSourceY;
            }
            else if (modal == 11) {
                trackType = TrackType::SpriteSourceW;
            }
            else if (modal == 12) {
                trackType = TrackType::SpriteSourceH;
            }
            else if (modal == 13) {
                trackType = TrackType::SortIndex;
            }

            a->document->AddTrack(a->document->GetTimelineAnimation(), a->document->GetSelectedNode(), trackType);
        }

        if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_KEY, "Animate node property", s->hierarchyFooterButtonIcon)) {
            // Nothing to do, modal self-activates
        }
    }
}

static void ImguiHierarchy(const Imgui::Rect& scrollArea, Application* a) {
    // Draw scroll list
    StyleSheet* s = &a->style;
    Imgui::Rect listItemKarrat = scrollArea;
    listItemKarrat.h = s->listBoxItemHeight;

    Node2D* hierarchyRearrangeDragging = 0;
    Imgui::Rect hierarchyInsertIndicator;

    u32 visibleNodeCount = a->document->GetVisibleNodeCount();
    Imgui::Point mousePos = Imgui::GetPointer();

    listItemKarrat.y -= (visibleNodeCount * s->listBoxItemHeight - scrollArea.h) * a->hierarchyScroll;

    Node2D* itr = a->document->DepthFirstExpandedOnly(0); // Root is guaranteed to exist
    itr = a->document->DepthFirstExpandedOnly(itr); // First one to draw

    bool even = true;
    while (itr != 0) {
        Imgui::HierarchyListItemResult listItem = Imgui::HierarchyListItem(listItemKarrat, scrollArea, itr->name,
            (itr->depth - 1) * 18, even, a->document->GetSelectedNode() == itr, itr->uiExpanded, itr->firstChild == 0);

        itr->uiExpanded = listItem.expanded;

        if (listItem.dragging) {
            hierarchyRearrangeDragging = itr;
        }

        if (listItem.activated) {
            a->document->SelectNode(itr);
        }

        even = !even;
        listItemKarrat.y += s->listBoxItemHeight;
        itr = a->document->DepthFirstExpandedOnly(itr);
    }

    if (hierarchyRearrangeDragging != 0) {
        float mouseInScrollSpace = (mousePos.y - scrollArea.y);
        if (mouseInScrollSpace < 0.0f) {
            mouseInScrollSpace = 0.0f;
        }
        else if (mouseInScrollSpace > scrollArea.h) {
            mouseInScrollSpace = scrollArea.h;
        }
        // Content goes up, so mouse goes down
        mouseInScrollSpace += (visibleNodeCount * s->listBoxItemHeight - scrollArea.h) * a->hierarchyScroll;

        i32 index = mouseInScrollSpace / s->listBoxItemHeight;
        PlatformAssert(index >= 0, __LOCATION__);

        if (index >= visibleNodeCount && visibleNodeCount >= 1) {
            index = visibleNodeCount - 1;
        }

        if (visibleNodeCount > 0) {
            PlatformAssert(index < visibleNodeCount, __LOCATION__);
        }

        Node2D* dropTarget = a->document->DepthFirstExpandedOnly(0);
        dropTarget = a->document->DepthFirstExpandedOnly(dropTarget);
        for (i32 i = 0; i < index; ++i) {
            dropTarget = a->document->DepthFirstExpandedOnly(dropTarget);
        }
        PlatformAssert(dropTarget != 0, __LOCATION__);

        bool above = false;
        bool below = false;

        float firstThird = ((float)index * s->listBoxItemHeight + s->listBoxItemHeight / 3.0f);
        float secondThird = firstThird + s->listBoxItemHeight / 3.0f;
        if (mouseInScrollSpace < firstThird) {
            above = true;
        }

        if (mouseInScrollSpace > secondThird) {
            below = true;
        }

        float midPoint = ((float)index * s->listBoxItemHeight + s->listBoxItemHeight / 2.0f);
        hierarchyInsertIndicator = Imgui::Rect(
            scrollArea.x, scrollArea.y + midPoint,
            scrollArea.w, 2
        );

        if (above) {
            hierarchyInsertIndicator.y -= s->listBoxItemHeight / 2.0f;

            if (dropTarget->depth >= 1) {
                hierarchyInsertIndicator.x += (dropTarget->depth - 1) * 18 + s->hierarchyLabelFontSize + 3;
                hierarchyInsertIndicator.w -= (dropTarget->depth - 1) * 18 + s->hierarchyLabelFontSize + 3;
            }
        }
        else if (below) {
            hierarchyInsertIndicator.y += s->listBoxItemHeight / 2.0f;

            if (dropTarget->depth >= 1) {
                hierarchyInsertIndicator.x += (dropTarget->depth - 1) * 18 + s->hierarchyLabelFontSize + 3;
                hierarchyInsertIndicator.w -= (dropTarget->depth - 1) * 18 + s->hierarchyLabelFontSize + 3;
            }
        }
        else {
            hierarchyInsertIndicator.y += s->listBoxItemHeight / 2.0f;
            hierarchyInsertIndicator.x += (dropTarget->depth) * 18 + s->hierarchyLabelFontSize + 3;
            hierarchyInsertIndicator.w -= (dropTarget->depth) * 18 + s->hierarchyLabelFontSize + 3;
        }
        hierarchyInsertIndicator.y -= (visibleNodeCount * s->listBoxItemHeight - scrollArea.h) * a->hierarchyScroll;

        if (Imgui::PointerReleased() && Imgui::Contains(scrollArea, mousePos)) {
            if (dropTarget != hierarchyRearrangeDragging) {
                if (above) {
                    Node2D* parent = dropTarget->parent;
                    Node2D* prev = 0;
                    Node2D* iter = parent->firstChild;
                    while (iter != 0) {
                        if (iter == dropTarget) {
                            break;
                        }
                        prev = iter;
                        iter = iter->next;
                    }
                    a->document->RearrangeNode(*hierarchyRearrangeDragging, *parent, prev);
                }
                else if (below) {
                    Node2D* parent = dropTarget->parent;
                    Node2D* prev = dropTarget;
                    a->document->RearrangeNode(*hierarchyRearrangeDragging, *parent, prev);
                }
                else {
                    a->document->RearrangeNode(*hierarchyRearrangeDragging, *dropTarget, 0);
                }
            }
        }
    }

    // Fill any missing space
    float bottom = scrollArea.y + scrollArea.h;
    float delta = bottom - (listItemKarrat.y);
    if (delta > 0.0f) {
        Imgui::Rect fillMissing = listItemKarrat;
        fillMissing.h = delta;

        Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
        if (Imgui::ClickArea(fillMissing)) {
            a->document->SelectNode(0);
        }
    }

    // Draw scroll bar
    Imgui::Rect hierarchyScrollBar = scrollArea;
    hierarchyScrollBar.x = scrollArea.x + scrollArea.w;
    hierarchyScrollBar.w = s->scrollBarSize;
    float contentHeight = visibleNodeCount * s->listBoxItemHeight;

    // Move scroll bar if re-arranging hierarchy
    if (hierarchyRearrangeDragging != 0) {
        Imgui::Rect scrollUp = scrollArea;
        scrollUp.h = s->scrollBarSize;
        Imgui::Rect scrollDown = scrollUp;
        scrollDown.y += scrollArea.h - scrollUp.h;

        if (Imgui::GetPulse()) {
            if (Imgui::Contains(scrollUp, mousePos)) {
                a->hierarchyScroll -= 0.05f;
            }
            if (Imgui::Contains(scrollDown, mousePos)) {
                a->hierarchyScroll += 0.05f;
            }

            if (a->hierarchyScroll < 0.0f) {
                a->hierarchyScroll = 0.0f;
            }
            if (a->hierarchyScroll > 1.0f) {
                a->hierarchyScroll = 1.0f;
            }
        }
    }

    float scrollerHeight = FigureOutScrollBarHeight(contentHeight, scrollArea.h);
    a->hierarchyScroll = Imgui::VScroll(hierarchyScrollBar, a->hierarchyScroll, scrollerHeight, Imgui::HandleScroll(scrollArea));

    // Draw footer
    Imgui::Rect hierarchyFooter = scrollArea;
    hierarchyFooter.y = scrollArea.y + scrollArea.h;
    hierarchyFooter.h = s->hierarchyFooterHeight;
    hierarchyFooter.w = scrollArea.w + s->scrollBarSize;

    {
        // Footer BG
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, hierarchyFooter.h,
            s->hierarchyFooterBg.r, s->hierarchyFooterBg.g, s->hierarchyFooterBg.b);
        // Footer divider
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y + 1, hierarchyFooter.w, 1,
            s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, 1,
            s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

        Imgui::Rect footerKarrat = hierarchyFooter;
        footerKarrat.x = hierarchyFooter.x + hierarchyFooter.w - s->hierarchyFooterButtonSize - 3;
        footerKarrat.y = hierarchyFooter.y + 5;
        footerKarrat.w = s->hierarchyFooterButtonSize;
        footerKarrat.h = s->hierarchyFooterButtonSize;

        { // New / delete node
            if (a->document->GetSelectedNode() == 0) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific offset
                iconRect.y += 3; // Footer specific offset
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_TRASHCAN, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_TRASHCAN, "Delete selected node", s->hierarchyFooterButtonIcon)) {
                    a->document->DeleteNode(a->document->GetSelectedNode());
                }
            }

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_NEWLAYER, "Create new node", s->hierarchyFooterButtonIcon)) {
                a->document->CreateNode(a->document->GetSelectedNode());
            }

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (a->document->GetSelectedNode() == 0) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific
                iconRect.y += 3;
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_DESELECT, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_DESELECT, "Deselect node", s->hierarchyFooterButtonIcon)) {
                    a->document->SelectNode(0);
                }
            }
        }

        footerKarrat.x -= 5;
        Draw2D::DrawRect((footerKarrat.x), hierarchyFooter.y + 2, 1, hierarchyFooter.h - 2,
            s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
        Draw2D::DrawRect((footerKarrat.x) + 1, hierarchyFooter.y + 2, 1, hierarchyFooter.h - 2,
            s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

        { // Key / unkey
            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            ImguiKeyButton(footerKarrat, a, s);
        }

        { // Node Count Label
            char printString[12] = { 0 };
            stbsp_snprintf(printString, 12, "Count: %d", a->document->GetNodeCount());
            Draw2D::DrawString(a->interfaceFont, 12, hierarchyFooter.x + 5, hierarchyFooter.y + 20, printString,
                s->hierarchyLabel.r, s->hierarchyLabel.g, s->hierarchyLabel.b);
        }
    }

    // Draw re-arrange indicator
    if (a->hierarchyTab == 0) { // Hierarchy re-arrange indicator (needs to draw above footer)
        if (hierarchyRearrangeDragging != 0) {
            Draw2D::DrawRect(hierarchyInsertIndicator.x, hierarchyInsertIndicator.y, hierarchyInsertIndicator.w, hierarchyInsertIndicator.h, s->hierarchyItemBG_Movable.r, s->hierarchyItemBG_Movable.g, s->hierarchyItemBG_Movable.b);
        }
    }
}

static void ImguiUndoStack(const Imgui::Rect& scrollArea, Application* a) {
    // Draw scroll list
    StyleSheet* s = &a->style;
    Imgui::Rect listItemKarrat = scrollArea;
    listItemKarrat.h = s->listBoxItemHeight;

    listItemKarrat.y -= (a->document->GetNumUndoSteps() * s->listBoxItemHeight - scrollArea.h) * a->hierarchyScroll;

    bool do_undo = false;
    u32 undoIndex = 0;

    u32 currentUndoStep = a->document->GetUndoStackCurrent();
    u32 currentUndoIndex = currentUndoStep;
    if (currentUndoIndex != 0) {
        currentUndoIndex -= 1;
    }

    for (u32 i = 0, numUndoSteps = a->document->GetNumUndoSteps(); i < numUndoSteps; ++i) {
        const char* name = a->document->GetUndoStepName(i);

        if (Imgui::UndoListItem(listItemKarrat, scrollArea, name, i % 2, currentUndoStep < i + 1, i + 1 == currentUndoStep)) {
            if (!do_undo) {
                do_undo = true;
                undoIndex = i;
            }
        }

        listItemKarrat.y += s->listBoxItemHeight;
    }

    if (do_undo) {
        if (undoIndex < currentUndoIndex) { // Undo x times
            u32 numUndoSteps = currentUndoIndex - undoIndex;
            for (u32 i = 0; i < numUndoSteps; ++i) {
                if (a->document->CanUndo()) {
                    a->document->Undo();
                }
            }
        }
        else if (undoIndex > currentUndoIndex) { // Redo x times
            u32 numRedoSteps = undoIndex - currentUndoIndex;
            for (u32 i = 0; i < numRedoSteps; ++i) {
                if (a->document->CanRedo()) {
                    a->document->Redo();
                }
            }
        }
    }

    // Fill any missing space
    float bottom = scrollArea.y + scrollArea.h;
    float delta = bottom - (listItemKarrat.y);
    if (delta > 0.0f) {
        Imgui::Rect fillMissing = listItemKarrat;
        fillMissing.h = delta;

        Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
    }

    // Draw scroll bar
    Imgui::Rect hierarchyScrollBar = scrollArea;
    hierarchyScrollBar.x = scrollArea.x + scrollArea.w;
    hierarchyScrollBar.w = s->scrollBarSize;
    float contentHeight = a->document->GetNumUndoSteps() * s->listBoxItemHeight;
    float scrollerHeight = FigureOutScrollBarHeight(contentHeight, scrollArea.h);

    a->hierarchyScroll = Imgui::VScroll(hierarchyScrollBar, a->hierarchyScroll, scrollerHeight, Imgui::HandleScroll(scrollArea));

    // Draw footer
    Imgui::Rect hierarchyFooter = scrollArea;
    hierarchyFooter.y = scrollArea.y + scrollArea.h;
    hierarchyFooter.h = s->hierarchyFooterHeight;
    hierarchyFooter.w = scrollArea.w + s->scrollBarSize;

    // Draw footer
    {
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, hierarchyFooter.h,
            s->hierarchyFooterBg.r, s->hierarchyFooterBg.g, s->hierarchyFooterBg.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y + 1, hierarchyFooter.w, 1,
            s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
        Draw2D::DrawRect(hierarchyFooter.x, hierarchyFooter.y, hierarchyFooter.w, 1,
            s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

        Imgui::Rect footerKarrat = hierarchyFooter;
        footerKarrat.x = hierarchyFooter.x + hierarchyFooter.w - s->hierarchyFooterButtonSize - 3;
        footerKarrat.y = hierarchyFooter.y + 5;
        footerKarrat.w = s->hierarchyFooterButtonSize;
        footerKarrat.h = s->hierarchyFooterButtonSize;

        { // Clear Stack / Debug View
            if (a->document->GetNumUndoSteps() == 0) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific offset
                iconRect.y += 3; // Footer specific offset
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_TRASHCAN, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_TRASHCAN, "Clear undo history", s->hierarchyFooterButtonIcon)) {
                    a->document->ClearUndoHistory();
                }
            }

            footerKarrat.x -= 5;
            Draw2D::DrawRect((footerKarrat.x), hierarchyFooter.y + 2, 1, hierarchyFooter.h - 2,
                s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
            Draw2D::DrawRect((footerKarrat.x) + 1, hierarchyFooter.y + 2, 1, hierarchyFooter.h - 2,
                s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (!a->document->CanRedo()) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific
                iconRect.y += 3;
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_REDO, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_REDO, "Redo", s->hierarchyFooterButtonIcon)) {
                    a->document->Redo();
                }
            }
            footerKarrat.x = footerKarrat.x - s->hierarchyFooterButtonSize - 3;
            if (!a->document->CanUndo()) {
                Imgui::Rect iconRect = footerKarrat;
                iconRect.x += 2; // Footer specific
                iconRect.y += 3;
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_UNDO, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (Imgui::FooterButton(footerKarrat, IMGUI_ICON_CODEPOINT_UNDO, "Undo", s->hierarchyFooterButtonIcon)) {
                    a->document->Undo();
                }
            }

            { // History Label
                char printString[36] = { 0 };
                stbsp_snprintf(printString, 36, "History: %d, Max: %d", a->document->GetUndoStackCurrent(), a->document->GetMaxUndoSteps());
                Draw2D::DrawString(a->interfaceFont, 12, hierarchyFooter.x + 5, hierarchyFooter.y + 20, printString,
                    s->hierarchyLabel.r, s->hierarchyLabel.g, s->hierarchyLabel.b);
            }
        }
    }
}

static void ImguiAnimationEditor(const Imgui::Rect& area, Application* a) {
    StyleSheet* s = &a->style;
    Draw2D::DrawRect(area.x, area.y, area.w, area.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);

    // Draw text boxes
    Imgui::Rect karrat(area.x + 10, area.y + 13, area.w - 20, 0);
    Imgui::Rect textArea = karrat;
    textArea.h = s->txtAreaHeight;

    const char* oldName = "";
    Animation* selectedAnim = a->document->GetSelectedAnimation();
    if (selectedAnim != 0) {
        oldName = selectedAnim->name;
    }

    const char* newName = Imgui::TextArea(textArea, oldName, "Name", selectedAnim == 0, false, false, false, false, false);
    if (selectedAnim != 0 && newName != oldName) {
        a->document->RenameAnimation(selectedAnim, newName);
    }

    textArea.y += textArea.h + 5 + s->textAreaLabelSize;
    float trippleWidth = (textArea.w - 20) / 3.0f;
    textArea.w = trippleWidth;

    char num_buffer[64] = { 0 };
    {
        if (selectedAnim != 0) {
            stbsp_snprintf(num_buffer, 64, "%.d", selectedAnim->frameRate);
        }
        else {
            num_buffer[0] = '\0';
        }
        const char* newNumber = Imgui::TextArea(textArea, num_buffer, "Frame Rate", selectedAnim == 0, false, true, false, false, false);
        if (selectedAnim != 0 && newNumber != num_buffer) {
            i32 frameRate = MathAToI(newNumber);
            if (frameRate < 0) {
                frameRate = 0;
            }
            a->document->UpdateAnimation(selectedAnim, selectedAnim->frameCount, frameRate, selectedAnim->loop);
        }
    }

    textArea.x += trippleWidth + 10;

    {
        if (selectedAnim != 0) {
            stbsp_snprintf(num_buffer, 64, "%.d", selectedAnim->frameCount);
        }
        else {
            num_buffer[0] = '\0';
        }
        const char* newNumber = Imgui::TextArea(textArea, num_buffer, "Frame Count", selectedAnim == 0, false, true, false, false, false);
        if (selectedAnim != 0 && newNumber != num_buffer) {
            i32 frameCount = MathAToI(newNumber);
            if (frameCount < 0) {
                frameCount = 0;
            }
            a->document->UpdateAnimation(selectedAnim, frameCount, selectedAnim->frameRate, selectedAnim->loop);
        }
    }

    textArea.x += trippleWidth + 10;

    {
        const char* name = "None";
        i32 selected = -1;
        if (selectedAnim != 0) {
            selected = 0;
            if (selectedAnim->loop == AnimationLoopMode::Looping) {
                selected = 1;
                name = "Looping";
            }
        }

        i32 selection = Imgui::BeginComboBox(textArea, name, selectedAnim == 0 ? 0 : 2, selected, "Repeat", false, selectedAnim == 0);
        if (selectedAnim != 0) {
            Imgui::PushComboBoxItem("None");
            Imgui::PushComboBoxItem("Looping");
        }
        Imgui::EndComboBox();

        if (selected != selection && selectedAnim != 0) {
            a->document->UpdateAnimation(selectedAnim, selectedAnim->frameCount, selectedAnim->frameRate, selection == 1 ? AnimationLoopMode::Looping : AnimationLoopMode::None);
        }
    }

}

inline Track* ContainsTrack(Animation* anim, Node2D* node, TrackType type) {
    if (anim == 0 || node == 0) {
        return 0;
    }

    return anim->Contains(node, type);
}

static void ImguiAnimatedFloatTextField(Application* a, const Imgui::Rect& area, const char* label, TrackType trackType, char* displayBuffer, u32 displayBufferLen) {
    Document* doc = a->document;

    Animation* timelineAnimation = doc->GetTimelineAnimation();
    Node2D* selectedNode = doc->GetSelectedNode();
    i32 selectedFrame = doc->GetSelectedFrame();
    Track* animatedTrack = ContainsTrack(timelineAnimation, selectedNode, trackType);

    bool contained = animatedTrack != 0 && animatedTrack->numKeyFrames > 0;
    bool interpolated = false;
    bool tempChanged = false;

    if (selectedNode != 0) {
        if (contained) {
            PlatformAssert(timelineAnimation != 0, __LOCATION__); // Safe if contained

            if (selectedNode->IsPropertyDirty(trackType)) {
                interpolated = false;
                tempChanged = true;
                stbsp_snprintf(displayBuffer, displayBufferLen, "%.2f", selectedNode->GetPropertyBuffer<f32>(trackType));
            }
            else {
                if (selectedFrame == -1) { // No frame selected, show frame 0
                    if (animatedTrack->frames[0].key) { // Frame 0 was a key, show it's value
                        interpolated = false;
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%.2f", animatedTrack->frames[0].fValue);
                    }
                    else { // Nothing is keyed for frame 0, show hierarchy value
                        interpolated = true;
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%.2f", selectedNode->GetProperty<f32>(trackType));
                    }
                }
                else {
                    PlatformAssert(selectedFrame < animatedTrack->frameCount, __LOCATION__);

                    if (animatedTrack->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                        interpolated = false;
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%.2f", animatedTrack->frames[selectedFrame].fValue);
                    }
                    else {
                        interpolated = true;

                        float interpolated_value = animatedTrack->InterpolateF(selectedFrame, selectedNode->GetProperty<f32>(trackType), timelineAnimation->loop);
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%.2f", interpolated_value);
                    }
                }
            }
        }
        else {
            stbsp_snprintf(displayBuffer, displayBufferLen, "%.2f", selectedNode->GetProperty<f32>(trackType));
        }
    }
    else {
        displayBuffer[0] = '\0';
    }

    bool disabled = selectedNode == 0;


    const char* newNumber = Imgui::TextArea(area, displayBuffer, label, disabled, true, false, contained, interpolated, tempChanged);
    if (selectedNode != 0 && newNumber != displayBuffer) {
        float new_val = MathAToF(newNumber);
        if (contained) {
            if (a->document->GetSelectedFrame() == a->document->GetLastSelectedFrame() && a->autoKey && animatedTrack != 0 && selectedFrame >= 0) {
                Frame dummy = { 0 };
                dummy.fValue = new_val;
                doc->AutoKeyFrameValue(animatedTrack, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
            }
            else {
                selectedNode->SetBuffered<f32>(trackType, new_val);
            }
        }
        else {
            if (doc->UpdateNodeTransformSingleF(selectedNode, new_val, trackType)) {
                if (a->autoKey && animatedTrack != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = new_val;
                    doc->AutoKeyFrameValue(animatedTrack, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
            }
        }
    }
}

static void ImguiAnimatedToggle(Application* a, const Imgui::Rect& toggleArea, TrackType trackType) {
    Document* doc = a->document;
    StyleSheet* s = &a->style;

    Animation* timelineAnimation = doc->GetTimelineAnimation();
    Node2D* selectedNode = doc->GetSelectedNode();
    i32 selectedFrame = doc->GetSelectedFrame();
    Track* animatedTrack = ContainsTrack(timelineAnimation, selectedNode, trackType);

    bool contained = animatedTrack != 0 && animatedTrack->numKeyFrames > 0;
    bool interpolated = false;
    bool tempChanged = false;

    bool displayBool = false;

    if (selectedNode != 0) {
        if (contained) {
            if (selectedNode->IsPropertyDirty(trackType)) {
                interpolated = false;
                tempChanged = true;
                displayBool = selectedNode->GetPropertyBuffer<bool>(trackType);
            }
            else {
                if (selectedFrame == -1) { // No frame selected, show frame 0
                    if (animatedTrack->frames[0].key) { // Frame 0 was a key, show it's value
                        interpolated = false;
                        displayBool = animatedTrack->frames[0].bValue;
                    }
                    else { // Nothing is keyed for frame 0, show hierarchy value
                        interpolated = true;
                        displayBool = selectedNode->GetProperty<bool>(trackType);
                    }
                }
                else {
                    if (animatedTrack->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                        interpolated = false;
                        displayBool = animatedTrack->frames[selectedFrame].bValue;
                    }
                    else {
                        interpolated = true;
                        bool interpolated_value = animatedTrack->InterpolateB(selectedFrame, selectedNode->GetProperty<bool>(trackType), timelineAnimation->loop);
                        displayBool = interpolated_value;
                    }
                }
            }
        }
        else {
            displayBool = selectedNode->GetProperty<bool>(trackType);
        }
    }

    StyleColor tint = { 0 };
    if (tempChanged) {
        tint.r = s->textAreaBg_DirtyTint.r;
        tint.g = s->textAreaBg_DirtyTint.g;
        tint.b = s->textAreaBg_DirtyTint.b;
    }
    else if (interpolated) {
        tint.r = s->textAreaBg_InterpolatedTint.r;
        tint.g = s->textAreaBg_InterpolatedTint.g;
        tint.b = s->textAreaBg_InterpolatedTint.b;
    }
    else if (contained) {
        tint.r = s->textAreaBg_AnimatedTint.r;
        tint.g = s->textAreaBg_AnimatedTint.g;
        tint.b = s->textAreaBg_AnimatedTint.b;
    }


    bool visible = Imgui::ToggleButton(toggleArea, IMGUI_ICON_CODEPOINT_VISIBLE, IMGUI_ICON_CODEPOINT_INVISIBLE,
        displayBool, selectedNode == 0, 0, tint);

    if (selectedNode != 0 && visible != displayBool) {
        if (contained) {
            if (a->document->GetSelectedFrame() == a->document->GetLastSelectedFrame() && a->autoKey && animatedTrack != 0 && selectedFrame >= 0) {
                Frame dummy = { 0 };
                dummy.bValue = visible;
                doc->AutoKeyFrameValue(animatedTrack, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
            }
            else {
                selectedNode->SetBuffered<bool>(trackType, visible);
            }
        }
        else {
            if (doc->UpdateSpriteVisibility(selectedNode, visible, selectedNode->sortIndex)) {
                if (a->autoKey && animatedTrack != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.bValue = visible;
                    doc->AutoKeyFrameValue(animatedTrack, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
            }
        }

    }
}

static void ImguiAnimatedColorPicker(Application* a, StyleSheet* s, const Imgui::Rect& toggleArea) {
    Document* doc = a->document;

    Animation* timelineAnimation = doc->GetTimelineAnimation();
    Node2D* selectedNode = doc->GetSelectedNode();
    i32 selectedFrame = doc->GetSelectedFrame();
    Track* animatedTrackR = ContainsTrack(timelineAnimation, selectedNode, TrackType::SpriteTintR);
    Track* animatedTrackG = ContainsTrack(timelineAnimation, selectedNode, TrackType::SpriteTintG);
    Track* animatedTrackB = ContainsTrack(timelineAnimation, selectedNode, TrackType::SpriteTintB);


    rgb color = { 0 };

    bool containedR = animatedTrackR != 0 && animatedTrackR->numKeyFrames > 0;
    bool containedG = animatedTrackG != 0 && animatedTrackG->numKeyFrames > 0;
    bool containedB = animatedTrackB != 0 && animatedTrackB->numKeyFrames > 0;
    bool interpolatedR = false;
    bool interpolatedG = false;
    bool interpolatedB = false;
    bool tempChangedR = false;
    bool tempChangedG = false;
    bool tempChangedB = false;

    if (selectedNode != 0) {
        TrackType trackType = TrackType::SpriteTintR;
        if (containedR) {
            if (selectedNode->IsPropertyDirty(trackType)) {
                interpolatedR = false;
                tempChangedR = true;
                color.r = selectedNode->GetPropertyBuffer<f32>(trackType);
            }
            else {
                if (selectedFrame == -1) { // No frame selected, show frame 0
                    if (animatedTrackR != 0 && animatedTrackR->frames[0].key) { // Frame 0 was a key, show it's value
                        interpolatedR = false;
                        color.r = animatedTrackR->frames[0].fValue;
                    }
                    else { // Nothing is keyed for frame 0, show hierarchy value
                        interpolatedR = true;
                        color.r = selectedNode->GetProperty<f32>(trackType);
                    }
                }
                else {
                    if (animatedTrackR->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                        interpolatedR = false;
                        color.r = animatedTrackR->frames[selectedFrame].fValue;
                    }
                    else {
                        interpolatedR = true;
                        f32 interpolated_value = animatedTrackR->InterpolateF(selectedFrame, selectedNode->GetProperty<f32>(trackType), timelineAnimation->loop);
                        color.r = interpolated_value;
                    }
                }
            }
        }
        else {
            color.r = selectedNode->GetProperty<f32>(trackType);
        }

        trackType = TrackType::SpriteTintG;
        if (containedG) {
            if (selectedNode->IsPropertyDirty(trackType)) {
                interpolatedG = false;
                tempChangedG = true;
                color.g = selectedNode->GetPropertyBuffer<f32>(trackType);
            }
            else {
                if (selectedFrame == -1) { // No frame selected, show frame 0
                    if (animatedTrackG->frames[0].key) { // Frame 0 was a key, show it's value
                        interpolatedG = false;
                        color.g = animatedTrackG->frames[0].fValue;
                    }
                    else { // Nothing is keyed for frame 0, show hierarchy value
                        interpolatedG = true;
                        color.g = selectedNode->GetProperty<f32>(trackType);
                    }
                }
                else {
                    if (animatedTrackG->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                        interpolatedG = false;
                        color.g = animatedTrackG->frames[selectedFrame].fValue;
                    }
                    else {
                        interpolatedG = true;
                        f32 interpolated_value = animatedTrackG->InterpolateF(selectedFrame, selectedNode->GetProperty<f32>(trackType), timelineAnimation->loop);
                        color.g = interpolated_value;
                    }
                }
            }
        }
        else {
            color.g = selectedNode->GetProperty<f32>(trackType);
        }

        trackType = TrackType::SpriteTintB;
        if (containedB) {
            if (selectedNode->IsPropertyDirty(trackType)) {
                interpolatedB = false;
                tempChangedB = true;
                color.b = selectedNode->GetPropertyBuffer<f32>(trackType);
            }
            else {
                if (selectedFrame == -1) { // No frame selected, show frame 0
                    if (animatedTrackB->frames[0].key) { // Frame 0 was a key, show it's value
                        interpolatedB = false;
                        color.b = animatedTrackB->frames[0].fValue;
                    }
                    else { // Nothing is keyed for frame 0, show hierarchy value
                        interpolatedB = true;
                        color.b = selectedNode->GetProperty<f32>(trackType);
                    }
                }
                else {
                    if (animatedTrackB->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                        interpolatedB = false;
                        color.b = animatedTrackB->frames[selectedFrame].fValue;
                    }
                    else {
                        interpolatedB = true;
                        f32 interpolated_value = animatedTrackB->InterpolateF(selectedFrame, selectedNode->GetProperty<f32>(trackType), timelineAnimation->loop);
                        color.b = interpolated_value;
                    }
                }
            }
        }
        else {
            color.b = selectedNode->GetProperty<f32>(trackType);
        }
    }

    StyleColor tint = { 0 };
    if (tempChangedR || tempChangedG || tempChangedB) {
        tint.r = s->textAreaBg_DirtyTint.r;
        tint.g = s->textAreaBg_DirtyTint.g;
        tint.b = s->textAreaBg_DirtyTint.b;
    }
    else if (containedR || containedG || containedB) {
        tint.r = s->textAreaBg_AnimatedTint.r;
        tint.g = s->textAreaBg_AnimatedTint.g;
        tint.b = s->textAreaBg_AnimatedTint.b;
    }
    else if (interpolatedR || interpolatedG || interpolatedB) {
        tint.r = s->textAreaBg_InterpolatedTint.r;
        tint.g = s->textAreaBg_InterpolatedTint.g;
        tint.b = s->textAreaBg_InterpolatedTint.b;
    }


    rgb newColor = hsv2rgb(Imgui::ColorPickerButton(toggleArea, rgb2hsv(color), selectedNode == 0, tint));

    if (selectedNode != 0 && !compare(color, newColor)) {
        if (containedR || containedG || containedB) {
            bool validTrack = animatedTrackR != 0 || animatedTrackG != 0 || animatedTrackB != 0;
            if (a->document->GetSelectedFrame() == a->document->GetLastSelectedFrame() && a->autoKey && validTrack && selectedFrame >= 0) {
                if (a->autoKey && animatedTrackR != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = newColor.r;
                    doc->AutoKeyFrameValue(animatedTrackR, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
                if (a->autoKey && animatedTrackG != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = newColor.g;
                    doc->AutoKeyFrameValue(animatedTrackG, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
                if (a->autoKey && animatedTrackB != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = newColor.b;
                    doc->AutoKeyFrameValue(animatedTrackB, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
            }
            else {
                selectedNode->SetBuffered<float>(TrackType::SpriteTintR, newColor.r);
                selectedNode->SetBuffered<float>(TrackType::SpriteTintG, newColor.g);
                selectedNode->SetBuffered<float>(TrackType::SpriteTintB, newColor.b);
            }
        }
        else {
            if (doc->UpdateSpriteTint(selectedNode, newColor.r, newColor.g, newColor.b, selectedNode->sprite.tintA)) {
                if (a->autoKey && animatedTrackR != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = newColor.r;
                    doc->AutoKeyFrameValue(animatedTrackR, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
                if (a->autoKey && animatedTrackG != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = newColor.g;
                    doc->AutoKeyFrameValue(animatedTrackG, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
                if (a->autoKey && animatedTrackB != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = newColor.b;
                    doc->AutoKeyFrameValue(animatedTrackB, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
            }
        }
    }
}

static void ImguiAnimatedIntTextField(Application* a, const Imgui::Rect& area, const char* label, TrackType trackType, char* displayBuffer, u32 displayBufferLen, bool allowNegative) {
    Document* doc = a->document;

    Animation* timelineAnimation = doc->GetTimelineAnimation();
    Node2D* selectedNode = doc->GetSelectedNode();
    i32 selectedFrame = doc->GetSelectedFrame();
    Track* animatedTrack = ContainsTrack(timelineAnimation, selectedNode, trackType);

    bool contained = animatedTrack != 0 && animatedTrack->numKeyFrames > 0;
    bool interpolated = false;
    bool tempChanged = false;

    if (selectedNode != 0) {
        if (contained) {
            PlatformAssert(timelineAnimation != 0, __LOCATION__); // Safe if contained

            if (selectedNode->IsPropertyDirty(trackType)) {
                interpolated = false;
                tempChanged = true;
                i32 print_val = selectedNode->GetPropertyBuffer<i32>(trackType);
                stbsp_snprintf(displayBuffer, displayBufferLen, "%d", print_val);
            }
            else {
                if (selectedFrame == -1) { // No frame selected, show frame 0
                    if (animatedTrack->frames[0].key) { // Frame 0 was a key, show it's value
                        interpolated = false;
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%d", animatedTrack->frames[0].iValue);
                    }
                    else { // Nothing is keyed for frame 0, show hierarchy value
                        interpolated = true;
                        i32 print_val = selectedNode->GetProperty<i32>(trackType);
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%d", print_val);
                    }
                }
                else {
                    PlatformAssert(selectedFrame < animatedTrack->frameCount, __LOCATION__);

                    if (animatedTrack->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                        interpolated = false;
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%d", animatedTrack->frames[selectedFrame].iValue);
                    }
                    else {
                        interpolated = true;

                        int interpolated_value = animatedTrack->InterpolateI(selectedFrame, selectedNode->GetProperty<i32>(trackType), timelineAnimation->loop);
                        stbsp_snprintf(displayBuffer, displayBufferLen, "%d", interpolated_value);
                    }
                }
            }
        }
        else {
            i32 print_val = selectedNode->GetProperty<i32>(trackType);
            stbsp_snprintf(displayBuffer, displayBufferLen, "%d", print_val);
        }
    }
    else {
        displayBuffer[0] = '\0';
    }

    bool disabled = selectedNode == 0;

    const char* newNumber = Imgui::TextArea(area, displayBuffer, label, disabled, false, true, contained, interpolated, tempChanged);
    if (selectedNode != 0 && newNumber != displayBuffer) {
        i32 new_val = MathAToI(newNumber);
        if (!allowNegative) {
            new_val = MathAbsI(new_val);
        }
        if (contained) {
            if (a->document->GetSelectedFrame() == a->document->GetLastSelectedFrame() && a->autoKey && animatedTrack != 0 && selectedFrame >= 0) {
                Frame dummy = { 0 };
                dummy.iValue = new_val;
                doc->AutoKeyFrameValue(animatedTrack, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
            }
            else {
                selectedNode->SetBuffered<i32>(trackType, new_val);
            }
        }
        else {
            if (doc->UpdateNodeTransformSingleI(selectedNode, new_val, trackType)) {
                if (a->autoKey && animatedTrack != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.iValue = new_val;
                    doc->AutoKeyFrameValue(animatedTrack, selectedFrame, doc->GetSelectedInterpolationType(), dummy.uValue);
                }
            }
        }
    }
}

static void ImguiSpriteEditor(const Imgui::Rect& area, Application* a) {
    StyleSheet* s = &a->style;
    Draw2D::DrawRect(area.x, area.y, area.w, area.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);
    f32 areaSize = 22;

    Imgui::Rect karrat(area.x + 10, area.y + 13, area.w - 20, 0);
    Imgui::Rect textArea = karrat;
    textArea.h = s->txtAreaHeight;
    textArea.w = karrat.w - (75 + 55 + 20);
    //float trippleWidth = (textArea.w - 20) / 3.0f;

    Node2D* selectedNode = a->document->GetSelectedNode();
    //const char* oldName = (selectedNode != 0) ? selectedNode->name : "";
    //const char* newName = Imgui::TextArea(textArea, oldName, "Resource", selectedNode == 0, false);

    const char* selected = "None";
    Resource* selectedResource = 0;
    if (selectedNode != 0) {
        selectedResource = a->document->FindResourceById(selectedNode->sprite.resourceUID);
    }

    i32 selectionIndex = -1;
    if (selectedResource != 0) {
        selected = selectedResource->name;
        selectionIndex = 0;

        for (Resource* iter = a->document->ResourceIterator(0); iter != 0; iter = a->document->ResourceIterator(iter), selectionIndex++) {
            if (iter == selectedResource) {
                break;
            }
        }
    }

    i32 selectedIndex = Imgui::BeginComboBox(textArea, selected, a->document->GetNumResources(), selectionIndex, "Image", false, selectedNode == 0);

    Resource* iter = a->document->ResourceIterator(0);
    int resourceCounter = 0;
    while (iter != 0) {
        const char* option = iter->name;
        if (option == 0) {
            option = "[NULL]";
        }
        Imgui::PushComboBoxItem(option);

        if (selectionIndex != selectedIndex) {
            if (resourceCounter == selectedIndex) {
                selectedResource = iter;
                a->document->UpdateSprite(selectedNode, selectedResource);
            }
        }

        resourceCounter += 1;
        iter = a->document->ResourceIterator(iter);
    }

    Imgui::EndComboBox();

    textArea.x = karrat.x + textArea.w + 10;
    textArea.w = 55;
    Imgui::Rect labelRect(textArea.x, textArea.y, textArea.w, 16);
    Imgui::Label(labelRect, "Tint", selectedNode == 0);
    Imgui::Rect toggleArea(textArea.x + 32, textArea.y - 1, areaSize, areaSize);


    ImguiAnimatedColorPicker(a, &a->style, toggleArea);
    /*rgb color = { 0 };
    if (selectedNode != 0) {
        color.r = selectedNode->sprite.tintR;
        color.g = selectedNode->sprite.tintG;
        color.b = selectedNode->sprite.tintB;
    }


    rgb newColor = hsv2rgb(Imgui::ColorPickerButton(toggleArea, rgb2hsv(color), selectedNode == 0));

    if (selectedNode != 0 && !compare(color, newColor)) {
        a->document->UpdateSpriteTint(selectedNode, newColor.r, newColor.g, newColor.b, selectedNode->sprite.tintA);
    }*/

    textArea.x += textArea.w + 10;
    textArea.w = 75;

    labelRect = Imgui::Rect(textArea.x, textArea.y, textArea.w, 16);
    Imgui::Label(labelRect, "Visible", selectedNode == 0);
    toggleArea = Imgui::Rect(textArea.x + 52, textArea.y - 1, areaSize, areaSize);

    ImguiAnimatedToggle(a, toggleArea, TrackType::SpriteVisibility);

    textArea.y += textArea.h + 5 + s->textAreaLabelSize;
    textArea.x = karrat.x;
#if 1
    float trippleWidth = ((karrat.w - 20.0f) - IMAGE_PREVIEW_SIZE) / 2.0f;

    textArea.w = textArea.h = IMAGE_PREVIEW_SIZE;

    Imgui::Rect sourceRect;
    if (selectedResource != 0 && selectedNode != 0) {
        sourceRect.x = selectedNode->sprite.sourceX;
        sourceRect.y = selectedNode->sprite.sourceY;
        sourceRect.w = selectedNode->sprite.sourceW;
        sourceRect.h = selectedNode->sprite.sourceH;
        vec3 xForm = Imgui::ImageBlock(textArea, "Preview", selectedResource->image, sourceRect, selectedNode == 0);
        vec2 scaledPivot(selectedNode->sprite.pivotX * xForm.z + xForm.x, selectedNode->sprite.pivotY * xForm.z + xForm.y);

        if (selectedNode->sprite.pivotX > 0.0f && selectedNode->sprite.pivotY > 0.0f) {
            if (selectedNode->sprite.pivotX < selectedNode->sprite.sourceW && selectedNode->sprite.pivotY < selectedNode->sprite.sourceW) {
                Draw2D::DrawRect(textArea.x + scaledPivot.x - 2, textArea.y + scaledPivot.y - 2, 5, 5, 0.0f, 0.0f, 0.0f);
                //Draw2D::DrawRect(textArea.x + scaledPivot.x - 2, textArea.y + scaledPivot.y - 1, 5, 3, 0.0f, 0.0f, 0.0f);

                Draw2D::DrawRect(textArea.x + scaledPivot.x, textArea.y + scaledPivot.y - 1, 1, 3, 1.0f, 1.0f, 1.0f);
                Draw2D::DrawRect(textArea.x + scaledPivot.x - 1, textArea.y + scaledPivot.y, 3, 1, 1.0f, 1.0f, 1.0f);
            }
        }
    }
    else {
        Imgui::ImageBlock(textArea, "Preview", 0, sourceRect, selectedNode == 0);
    }

    textArea.w = trippleWidth;
    textArea.h = s->txtAreaHeight;
    textArea.x = karrat.x + IMAGE_PREVIEW_SIZE + 10;

    char num_buffer[64] = { 0 };


    ImguiAnimatedIntTextField(a, textArea, "Source.x", TrackType::SpriteSourceX, num_buffer, 64, false);
    textArea.x += trippleWidth + 10;

    ImguiAnimatedIntTextField(a, textArea, "Source.y", TrackType::SpriteSourceY, num_buffer, 64, false);
    textArea.y += textArea.h + 5 + s->textAreaLabelSize;
    textArea.x = karrat.x + IMAGE_PREVIEW_SIZE + 10;

    ImguiAnimatedIntTextField(a, textArea, "Source.w", TrackType::SpriteSourceW, num_buffer, 64, false);
    textArea.x += trippleWidth + 10;

    ImguiAnimatedIntTextField(a, textArea, "Source.h", TrackType::SpriteSourceH, num_buffer, 64, false);
    textArea.y += textArea.h + 5 + s->textAreaLabelSize;
    textArea.x = karrat.x + IMAGE_PREVIEW_SIZE + 10;

    vec2 pivot;
    if (selectedNode != 0) {
        pivot.x = selectedNode->sprite.pivotX;
        pivot.y = selectedNode->sprite.pivotY;
    }
    bool updatePivot = false;
    
    stbsp_snprintf(num_buffer, 64, "%.2f", selectedNode == 0? 0.0 : selectedNode->sprite.pivotX);
    const char* text = Imgui::TextArea(textArea, num_buffer, "Pivot.X", selectedNode == 0, true, false, false, false, false);
    if (text != num_buffer) {
        pivot.x = MathAToF(text);
        updatePivot = true;
    }

    textArea.x += trippleWidth + 10;

    stbsp_snprintf(num_buffer, 64, "%.2f", selectedNode == 0 ? 0.0 : selectedNode->sprite.pivotY);
    text = Imgui::TextArea(textArea, num_buffer, "Pivot.Y", selectedNode == 0, true, false, false, false, false);
    if (text != num_buffer) {
        pivot.y = MathAToF(text);
        updatePivot = true;
    }

    if (selectedNode != 0 && updatePivot) {
        a->document->UpdateSprite(selectedNode, selectedNode->sprite.sourceX, selectedNode->sprite.sourceY,
            selectedNode->sprite.sourceW, selectedNode->sprite.sourceH, pivot.x, pivot.y);
    }

    textArea.x += trippleWidth + 10;
#endif
}

static void ImguiTransformEditor(const Imgui::Rect& area, Application* a) {
    StyleSheet* s = &a->style;
    char num_buffer[64] = { 0 };
    // Draw BG
    Draw2D::DrawRect(area.x, area.y, area.w, area.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);

    // Draw text boxes
    Imgui::Rect karrat(area.x + 10, area.y + 13, area.w - 20, 0);
    Imgui::Rect textArea = karrat;
    textArea.h = s->txtAreaHeight;

    const char* oldName = "";
    Node2D* selectedNode = a->document->GetSelectedNode();
    if (selectedNode != 0) {
        oldName = selectedNode->name;
    }

    Animation* timelineAnimation = a->document->GetTimelineAnimation();

    float trippleWidth = (textArea.w - 20) / 3.0f;
    //textArea.w -= trippleWidth;
    //textArea.w -= 10.0f;

    const char* newName = Imgui::TextArea(textArea, oldName, "Name", selectedNode == 0, false, false, false, false, false);
    if (selectedNode != 0 && newName != oldName) {
        a->document->RenameNode(selectedNode, newName);
    }

    textArea.w = trippleWidth;
    textArea.x += (trippleWidth + 10.0f) * 2.0f;

    textArea.y += textArea.h + 5 + s->textAreaLabelSize;
    textArea.x = karrat.x;

    ImguiAnimatedFloatTextField(a, textArea, "Rotation", TrackType::TransformRotation, num_buffer, 64);
    textArea.x += trippleWidth + 10;

    ImguiAnimatedFloatTextField(a, textArea, "Position.x", TrackType::TransformPositionX, num_buffer, 64);
    textArea.x += trippleWidth + 10;

    ImguiAnimatedFloatTextField(a, textArea, "Position.y", TrackType::TransformPositionY, num_buffer, 64);
    textArea.x = karrat.x;
    textArea.y += textArea.h + 5 + s->textAreaLabelSize;

    ImguiAnimatedIntTextField(a, textArea, "Sort Index", TrackType::SortIndex, num_buffer, 64, true);
    textArea.x += trippleWidth + 10;

    ImguiAnimatedFloatTextField(a, textArea, "Scale.x", TrackType::TransformScaleX, num_buffer, 64);
    textArea.x += trippleWidth + 10;

    ImguiAnimatedFloatTextField(a, textArea, "Scale.y", TrackType::TransformScaleY, num_buffer, 64);
    textArea.x += trippleWidth + 10;
}

static void ImguiMemoryViewer(const Imgui::Rect& area, Application* a) {
    StyleSheet* s = &a->style;

#define MEMORY_UI_HEADER_HEIGHT 55.0f
#define MEM_BLOCK_W 4.0f
#define MEM_BLOCK_H 7.0f

    Imgui::Rect headerArea(area.x + 1, area.y + 1, area.w - 2, MEMORY_UI_HEADER_HEIGHT);
    float chartAreaY = area.y + 1 + MEMORY_UI_HEADER_HEIGHT + 1;
    Imgui::Rect chartArea(area.x + 1, chartAreaY, area.w - s->scrollBarSize, area.h - (chartAreaY - area.y));

    char line[64] = { 0 };

    i32 pageSize = MemGetPageSize();
    i32 totalPageCount = MemGetHeapSize() / MemGetPageSize();
    i32 overheadCount = MemGetNumOverheadPages();
    i32 numPagesUsed = MemGetCurrentNumPages();
    i32 mostMemPages = MemGetMostPagesUsedAtOne();
    i32 freeCount = totalPageCount - overheadCount - numPagesUsed;
    f32 halfW = chartArea.w / 2.0f;

    StyleColor fontColor = s->textAreaFont_Normal;

    { // Header
        Imgui::Point karrat(headerArea.x + 5, headerArea.y + 5);

        Draw2D::DrawRect(headerArea.x + 1, headerArea.y + 1, headerArea.w - 2, headerArea.h - 2,
            s->textAreaBg_Normal.r, s->textAreaBg_Normal.g, s->textAreaBg_Normal.b);

        stbsp_snprintf(line, 64, "Page size: %d bytes", pageSize);
        Draw2D::DrawString(a->interfaceFont, 12, karrat.x, karrat.y + 12, line, fontColor.r, fontColor.g, fontColor.b);
        karrat.x += halfW;

        stbsp_snprintf(line, 64, "Total page count: %d", totalPageCount);
        Draw2D::DrawString(a->interfaceFont, 12, karrat.x, karrat.y + 12, line, fontColor.r, fontColor.g, fontColor.b);
        karrat.x -= halfW;
        karrat.y += 16;

        stbsp_snprintf(line, 64, "Overhead page count: %d", overheadCount);
        Draw2D::DrawString(a->interfaceFont, 12, karrat.x, karrat.y + 12, line, fontColor.r, fontColor.g, fontColor.b);
        karrat.x += halfW;

        stbsp_snprintf(line, 64, "Used page count: %d", numPagesUsed);
        Draw2D::DrawString(a->interfaceFont, 12, karrat.x, karrat.y + 12, line, fontColor.r, fontColor.g, fontColor.b);
        karrat.x -= halfW;
        karrat.y += 16;

        float most_used_precent = (float)mostMemPages / (float)totalPageCount + 0.005;;
        i32 most_used_int = Math01(most_used_precent) * 100.0f;
        stbsp_snprintf(line, 64, "Most pages used: %d (~%d%c)", mostMemPages, most_used_int, '%');
        Draw2D::DrawString(a->interfaceFont, 12, karrat.x, karrat.y + 12, line, fontColor.r, fontColor.g, fontColor.b);
        karrat.x += halfW;

        float free_precent = (float)freeCount / (float)totalPageCount + 0.005;
        i32 free_int = Math01(free_precent) * 100.0f;
        stbsp_snprintf(line, 64, "Free page count: %d (~%d%c)", freeCount, free_int, '%');
        Draw2D::DrawString(a->interfaceFont, 12, karrat.x, karrat.y + 12, line, fontColor.r, fontColor.g, fontColor.b);
    }

    { // Body
        Imgui::Point karrat(chartArea.x + 2, chartArea.y + 2);

        u32 numColumns = (chartArea.w - 2) / MEM_BLOCK_W;
        u32 numRows = totalPageCount / numColumns + (totalPageCount % numColumns == 0 ? 0 : 1);


        f32 contentHeight = (f32)numRows * MEM_BLOCK_H;
        float scrollerHeight = FigureOutScrollBarHeight(contentHeight, chartArea.h - 4);

        Imgui::Rect scrollableArea(chartArea.x + chartArea.w, chartArea.y, s->scrollBarSize, chartArea.h);
        a->memoryScroll = Imgui::VScroll(scrollableArea, a->memoryScroll, scrollerHeight, Imgui::HandleScroll(chartArea));

        f32 scroll_adj = (contentHeight - (chartArea.h - 4)) * a->memoryScroll;

        Draw2D::PushClip(chartArea.x, chartArea.y, chartArea.w, chartArea.h);
        for (i32 y = 0; y < numRows; ++y) {
            for (i32 x = 0; x < numColumns; ++x) {
                i32 index = y * numColumns + x;
                if (index >= totalPageCount) {
                    continue;
                }
                if (MemIsPageUsed(index)) {
                    Draw2D::DrawRect(karrat.x + x * MEM_BLOCK_W, karrat.y + y * MEM_BLOCK_H - scroll_adj, MEM_BLOCK_W - 1, MEM_BLOCK_H - 1,
                        s->memoryPageInUse.r, s->memoryPageInUse.g, s->memoryPageInUse.b);
                }
                else {
                    Draw2D::DrawRect(karrat.x + x * MEM_BLOCK_W, karrat.y + y * MEM_BLOCK_H - scroll_adj, MEM_BLOCK_W - 1, MEM_BLOCK_H - 1,
                        s->memoryFreePage.r, s->memoryFreePage.g, s->memoryFreePage.b);
                }


            }
        }
        Draw2D::PopClip();
    }

    //Draw2D::DrawRect(chartArea.x, chartArea.y, chartArea.w, chartArea.h);
}


bool KeyboardButton(u32 font, const Imgui::Rect& screenPos, const char* label, Application* a) {
    u32 widgetId = ++a->kbdGen;

    StyleSheet* s = &a->style;
    Imgui::Point mouse = Imgui::GetPointer();
    bool pressed = Imgui::PointerPressed();
    bool released = Imgui::PointerReleased();

    // Make hot
    if (Imgui::Contains(screenPos, mouse)) {
        if (a->kbdActive == widgetId || a->kbdActive == 0) {
            a->kbdHot = widgetId;
        }
    }

    // Make active
    if (a->kbdHot == widgetId && a->kbdActive == 0) {
        if (pressed) {
            a->kbdActive = widgetId;
        }
    }

    if (a->kbdActive == widgetId) {
        Imgui::VirtualKeyWasPressed();
    }

    // Render
    StyleColor outline = s->textAreaOutline_Normal;
    StyleColor fill = s->textAreaBg_Normal;

    if (a->kbdActive == widgetId) {
        outline = s->textAreaOutline_Active;
        fill = s->textAreaBg_Active;
    }
    else if (a->kbdHot == widgetId) {
        outline = s->textAreaOutline_Hot;
        fill = s->textAreaBg_Hot;
    }

    Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, outline.r, outline.g, outline.b);
    Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2, fill.r, fill.g, fill.b);
    if (label != 0) {
        Draw2D::DrawString(font, 28,
            screenPos.x + screenPos.w / 2 - Draw2D::MeasureString(a->interfaceFont, 28, label).w / 2,
            //screenPos.x + 20,
            screenPos.y + 33, label);
    }

    // Activate return
    if (a->kbdActive == widgetId && a->kbdHot == widgetId) {
        if (released) {
            a->kbdHandled = true;
            if (Imgui::GetFakeShiftToggle() && !(label[0] == 'S' && (label[1] == 'h' || label[1] == 'H'))) {
                Imgui::SetFakeShiftToggle(false);
            }
            if (Imgui::GetFakeControlToggle() && !(label[0] == 'C' && (label[1] == 't' || label[1] == 'T'))) {
                if (!(label[0] == 'c' || label[0] == 'C' || label[0] == 'v' || label[0] == 'V' || label[0] == 'x' || label[0] == 'X' || label[0] == 'a' || label[0] == 'A'))
                Imgui::SetFakeControlToggle(false);
                a->ctrlLock = false;
            }
            return true;
        }
    }
    return false;
}

bool KeyboardButton(const Imgui::Rect& screenPos, const char* label, Application* a) {
    return KeyboardButton(a->interfaceFont, screenPos, label, a);
}

bool KeyboardButtonIcon(const Imgui::Rect& screenPos, const char* label, Application* a) {
    return KeyboardButton(a->iconFont, screenPos, label, a);
}

static void ImguiKeyboard(const Imgui::Rect& area, Application* a) {
    float buttonSize = 30.0f;
    float sp = 5.0f;
    bool shift = KeyboardDown(KeyboardCodeShift);
    if (Imgui::GetFakeShiftToggle() || Imgui::GetFakeCapsToggle()) {
        shift = true;
    }

    float keyboard_width = (buttonSize * 1.5f + sp) + (buttonSize * 2.0f + sp) + ((buttonSize + sp) * 12.0f) + 5.0f;
    float keyboard_height = ((buttonSize + sp) * 5.0f) + 5.0f;

    float w_ratio = area.w / keyboard_width;
    buttonSize *= w_ratio;
    sp *= w_ratio;
    keyboard_height *= w_ratio;
    keyboard_width *= w_ratio;

    if (keyboard_height > area.h) {
        float h_ratio = area.h / keyboard_height;
        buttonSize *= h_ratio;
        sp *= h_ratio;
        keyboard_height *= h_ratio;
        keyboard_width *= h_ratio;
    }

    float x_offset = MathAbsF(keyboard_width - area.w) * 0.5f;;
    vec2 karrat(x_offset + area.x + 5, area.y + 5);

#define VirtualButton(widthMul, lowerString, upperString, daKeyCode) \
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (widthMul), buttonSize), shift ? (upperString) : (lowerString), a)) { \
            PushKey(daKeyCode); \
        } \
        karrat.x += buttonSize * (widthMul) + sp;

#define VirtualButtonIcn(widthMul, lowerString, upperString, daKeyCode) \
        if (KeyboardButtonIcon(Imgui::Rect(karrat.x, karrat.y, buttonSize * (widthMul), buttonSize), shift ? (upperString) : (lowerString), a)) { \
            PushKey(daKeyCode); \
        } \
        karrat.x += buttonSize * (widthMul) + sp;


    { // Num row
        

        VirtualButton(1.5f, "`", "~", KeyboardCodeTilde);
        VirtualButton(1.0f, "1", "!", KeyboardCode1);
        VirtualButton(1.0f, "2", "@", KeyboardCode2);
        VirtualButton(1.0f, "3", "#", KeyboardCode3);
        VirtualButton(1.0f, "4", "$", KeyboardCode4);
        VirtualButton(1.0f, "5", "%", KeyboardCode5);
        VirtualButton(1.0f, "6", "^", KeyboardCode6);
        VirtualButton(1.0f, "7", "&", KeyboardCode7);
        VirtualButton(1.0f, "8", "*", KeyboardCode8);
        VirtualButton(1.0f, "9", "(", KeyboardCode9);
        VirtualButton(1.0f, "0", ")", KeyboardCode0);
        VirtualButton(1.0f, "-", "_", KeyboardCodeMinus);
        VirtualButton(1.0f, "=", "+", KeyboardCodeEquals);
        VirtualButton(2.0f, "Back", "Back", KeyboardCodeBackspace);
    }

    karrat.y += buttonSize + sp;
    karrat.x = x_offset + area.x + 5;

    { // qwerty
        VirtualButton(2.0f, "Tab", "Tab", KeyboardCodeTab);
        VirtualButton(1.0f, "q", "Q", KeyboardCodeQ);
        VirtualButton(1.0f, "w", "W", KeyboardCodeW);
        VirtualButton(1.0f, "e", "E", KeyboardCodeE);
        VirtualButton(1.0f, "r", "R", KeyboardCodeR);
        VirtualButton(1.0f, "t", "T", KeyboardCodeT);
        VirtualButton(1.0f, "y", "Y", KeyboardCodeY);
        VirtualButton(1.0f, "u", "U", KeyboardCodeU);
        VirtualButton(1.0f, "i", "I", KeyboardCodeI);
        VirtualButton(1.0f, "o", "O", KeyboardCodeO);
        VirtualButton(1.0f, "p", "P", KeyboardCodeP);
        VirtualButton(1.0f, "[", "{", KeyboardCodeLBracket);
        VirtualButton(1.0f, "]", "}", KeyboardCodeRbracket);
        VirtualButton(1.5f, "\\", "|", KeyboardCodeBackslash);
    }

    karrat.y += buttonSize + sp;
    karrat.x = x_offset + area.x + 5;

    { // asdf
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (2.5f), buttonSize), Imgui::GetFakeCapsToggle()? "CAP" : "Cap", a)) {
            Imgui::SetFakeCapsToggle(!Imgui::GetFakeCapsToggle());
        }
        karrat.x += buttonSize * (2.5) + sp;

        VirtualButton(1.0f, "a", "A", KeyboardCodeA);
        VirtualButton(1.0f, "s", "S", KeyboardCodeS);
        VirtualButton(1.0f, "d", "D", KeyboardCodeD);
        VirtualButton(1.0f, "f", "F", KeyboardCodeF);
        VirtualButton(1.0f, "g", "G", KeyboardCodeG);
        VirtualButton(1.0f, "h", "H", KeyboardCodeH);
        VirtualButton(1.0f, "j", "J", KeyboardCodeJ);
        VirtualButton(1.0f, "k", "K", KeyboardCodeK);
        VirtualButton(1.0f, "l", "L", KeyboardCodeL);
        VirtualButton(1.0f, ";", ":", KeyboardCodeColon);
        VirtualButton(1.0f, "'", "\"", KeyboardCodeQoute);
        VirtualButton(2.2f, "Enter", "Enter", KeyboardCodeReturn);
    }

    karrat.y += buttonSize + sp;
    karrat.x = x_offset + area.x + 5;

    { // zxcv
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (3.0f), buttonSize), Imgui::GetFakeShiftToggle() ? ("SHIFT") : ("Shift"), a)) {
            Imgui::SetFakeShiftToggle(!Imgui::GetFakeShiftToggle());
        }
        karrat.x += buttonSize * (3.0f)+sp;

        VirtualButton(1.0f, "z", "Z", KeyboardCodeZ);
        VirtualButton(1.0f, "x", "X", KeyboardCodeX);
        VirtualButton(1.0f, "c", "C", KeyboardCodeC);
        VirtualButton(1.0f, "v", "V", KeyboardCodeV);
        VirtualButton(1.0f, "b", "B", KeyboardCodeB);
        VirtualButton(1.0f, "n", "N", KeyboardCodeN);
        VirtualButton(1.0f, "m", "M", KeyboardCodeM);
        VirtualButton(1.0f, ",", "<", KeyboardCodeComma);
        VirtualButton(1.0f, ".", ">", KeyboardCodePeriod);
        VirtualButton(1.0f, "/", "?", KeyboardCodeSlash);
        VirtualButtonIcn(1.0f, "#", "#", KeyboardCodeSlash);
        
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.7f), buttonSize), Imgui::GetFakeShiftToggle() ? ("SHIFT") : ("Shift"), a)) {
            Imgui::SetFakeShiftToggle(!Imgui::GetFakeShiftToggle());
        }
        karrat.x += buttonSize * (1.7f) + sp;
    }

    karrat.y += buttonSize + sp;
    karrat.x = x_offset + area.x + 5;

    { // Spacebar
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.5f), buttonSize), a->ctrlLock ? "CTRL" : "Ctrl", a)) {
            a->ctrlLock = !a->ctrlLock;
            Imgui::SetFakeControlToggle(a->ctrlLock);
        }
        karrat.x += buttonSize * (1.5f) + sp;

        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.5f), buttonSize), a->altLock ? "ALT" : "Alt", a)) {
            a->altLock = !a->altLock;
        }
        karrat.x += buttonSize * (1.5f) + sp;

        VirtualButtonIcn(6.5, (const char*)0, (const char*)0, KeyboardCodeSpace);
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.5f), buttonSize), a->ctrlLock ? "CTRL" : "Ctrl", a)) {
            a->ctrlLock = !a->ctrlLock;
            Imgui::SetFakeControlToggle(a->ctrlLock);
        }
        karrat.x += buttonSize * (1.5f) + sp;
        if (KeyboardButton(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.5f), buttonSize), a->altLock ? "ALT" : "Alt", a)) {
            a->altLock = !a->altLock;
        }
        karrat.x += buttonSize * (1.5f) + sp;
        //VirtualButtonIcn(1.35f, "!", "!", KeyboardCodeLeft);
        if (KeyboardButtonIcon(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.35f), buttonSize), "!", a)) {
            Imgui::SetFakeLeftPressed(true);
        }
        karrat.x += buttonSize * (1.35f) + sp;

        VirtualButtonIcn(1.0f, "$", "$", KeyboardCodeDown);
        //VirtualButtonIcn(1.7f, "\"", "\"", KeyboardCodeRight);
        if (KeyboardButtonIcon(Imgui::Rect(karrat.x, karrat.y, buttonSize * (1.7f), buttonSize), "\"", a)) {
            Imgui::SetFakeRightPressed(true);
        }
        karrat.x += buttonSize * (1.7f) + sp;
    }
}

static void ImguiAnimationInspector(const Imgui::Rect& area, Application* a) {
    StyleSheet* s = &a->style;

    Animation* timelineAnimation = a->document->GetTimelineAnimation();

    Imgui::Rect header = area;
    Imgui::Rect body = area;
    Imgui::Rect footer = area;

    header.h = s->headerHeight;
    footer.h = s->scrollBarSize;

    body.h -= header.h;
    body.h -= footer.h;
    body.y += s->headerHeight;

    footer.y = body.y + body.h;

    // Draw header bg
    Draw2D::DrawRect(header.x, header.y, header.w, header.h,
        s->toolBarBg.r, s->toolBarBg.g, s->toolBarBg.b);

    Draw2D::DrawRect(header.x, header.y + header.h - 2, header.w, 1,
        s->headerBgColor.r, s->headerBgColor.g, s->headerBgColor.b);

    // Draw splitter
    a->timelineSplit = Imgui::VSplit(area, s->timelineMinWidth, a->timelineSplit);
    Imgui::Rect timelineLeft = Imgui::VSplitFirstArea(area, a->timelineSplit);
    Imgui::Rect timelineRight = Imgui::VSplitSecondArea(area, a->timelineSplit);

    float scrollableHeight = 0.0f;
    u32 numTracks = 0;
    if (timelineAnimation != 0) {
        numTracks = timelineAnimation->numTracks;
        Track* tmp = timelineAnimation->tracks;
        for (u32 i = 0; i < timelineAnimation->numTracks; ++i) {
            if (!tmp->avtive) {
                numTracks -= 1;
            }
            tmp = tmp->next;
        }
        scrollableHeight = numTracks * s->listBoxItemHeight;
    }

    Imgui::Rect nodesScrollARea;
    float tracksScrollOffset = 0.0f;
    if (timelineAnimation != 0) { // Draw the list of tracks
        nodesScrollARea.x = timelineLeft.x;
        nodesScrollARea.w = timelineLeft.w - s->scrollBarSize;
        nodesScrollARea.y = timelineLeft.y + header.h;
        nodesScrollARea.h = timelineLeft.h - header.h - footer.h;

        Imgui::Rect listItemKarrat = nodesScrollARea;
        listItemKarrat.h = s->listBoxItemHeight;

        {
            tracksScrollOffset = (scrollableHeight - nodesScrollARea.h) * a->frameUpScroll;
            listItemKarrat.y -= tracksScrollOffset;

            char uiName[128] = { 0 };
            int i = 0;
            Track* activeTrack = a->document->GetSelectedTrack();
            for (Track* iter = timelineAnimation->tracks; iter != 0; iter = iter->next) {
                if (!iter->avtive) {
                    if (iter == activeTrack) {
                        a->document->SelectTrack(0);
                    }
                    continue;
                }

#if _DEBUG
                u32 numKeys = 0;
                for (int i = 0; i < iter->frameCount; ++i) {
                    if (iter->frames[i].key) {
                        numKeys += 1;
                    }
                }
                PlatformAssert(numKeys == iter->numKeyFrames, __LOCATION__);
#endif

                Node2D* targetNode = a->document->FindNodeById(iter->targetNode);
                if (targetNode == 0) {
                    iter->avtive = false;
                    continue;
                }
                PlatformAssert(targetNode != 0, __LOCATION__);

                stbsp_snprintf(uiName, 128, "%s%s", targetNode->name, TrackTypeToString(iter->targetProp));
                Imgui::DeletableListItemResult actionResult = Imgui::DeletableListItem(listItemKarrat, nodesScrollARea, uiName, (i++) % 2, false, activeTrack == iter);
                //if (Imgui::UndoListItem(listItemKarrat, nodesScrollARea, uiName, (i++) % 2, false, activeTrack == iter)) {

                if (actionResult.activated) {
                    a->document->SelectTrack(iter);
                }
                else if (actionResult.deleted) {
                    a->document->RemoveTrack(iter);
                }

                listItemKarrat.y += s->listBoxItemHeight;
            }
        }

        // Fill any missing space
        float bottom = nodesScrollARea.y + nodesScrollARea.h;
        float delta = bottom - (listItemKarrat.y);
        if (delta > 0.0f) {
            Imgui::Rect fillMissing = listItemKarrat;
            fillMissing.h = delta;

            Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
            if (Imgui::ClickArea(fillMissing)) {
                a->document->SelectTrack(0);
            }
        }
    }

    {
        Imgui::Rect leftHeader = timelineLeft;
        leftHeader.h = header.h;

        Imgui::Rect control = leftHeader;
        control.h = s->hierarchyFooterButtonSize + 1;// s->scrollBarSize;
        control.w = s->hierarchyFooterButtonSize + 1;// s->scrollBarSize;
        control.y += 2;
        control.w -= 2;
        control.x += 2;
        StyleColor tint = { 0 };
        bool now_playing = Imgui::ToggleButton(control, IMGUI_ICON_STOP, IMGUI_ICON_PLAY,
            a->playSelectedAnimation, timelineAnimation == 0, a->playSelectedAnimation ? "Stop" : "Play", tint);

        if (now_playing != a->playSelectedAnimation) {
            a->playSelectedAnimation = now_playing;
            a->playingAnimationFrame = 0;
            a->playingAnimationTimer = 0;

            if (now_playing) {
                a->document->SelectTrack(0);
            }
        }

        control.x += control.w;
        control.x += 2;
        StyleColor tint2 = { 0 };

        if (a->autoKey) {
            tint.r = 0.07f;
            tint2.r = 0.5f;
        }
        bool new_autokey = Imgui::ToggleButton(control, IMGUI_ICON_PAUSE, IMGUI_ICON_RECORD, a->autoKey, timelineAnimation == 0, "Autokey", tint, &tint2);
        tint.r = 0.0f;

        if (new_autokey != a->autoKey) {
            a->autoKey = new_autokey;
        }

        float x_pos_for_dropdown = control.x + control.w + 2;

        control = leftHeader;
        control.h = s->hierarchyFooterButtonSize + 1;
        control.w = s->hierarchyFooterButtonSize + 1;
        control.y += 2;
        control.x = leftHeader.x + leftHeader.w - control.w;
        control.w -= 2;
        control.x += 1;
        // Imgui::IconToggle(control, IMGUI_ICON_RECORD, IMGUI_ICON_RECORD, true, timelineAnimation == 0, "Autokey");

        if (timelineAnimation == 0) {
            Imgui::Rect iconRect = control;
            iconRect.x += 2; // Footer specific
            iconRect.y += 3;
            Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CODEPOINT_DESELECT, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
        }
        else {
            if (Imgui::FooterButton(control, IMGUI_ICON_CODEPOINT_DESELECT, "Deselect track", s->hierarchyFooterButtonIcon)) {
                a->document->SelectTrack(0);
                a->document->SetSelectedFrame(-1);
                a->playSelectedAnimation = false;
            }
        }

        control.x -= control.w;
        control.x -= 2;

        { // tmp scope for selectedTrak and selectedFrame
            Track* selectedTrack = a->document->GetSelectedTrack();
            i32 selectedFrame = a->document->GetSelectedFrame();
            bool disable_delete = timelineAnimation == 0 || selectedTrack == 0 || selectedFrame < 0;
            if (!disable_delete) {
                PlatformAssert(selectedFrame < selectedTrack->frameCount, __LOCATION__);
                disable_delete = !selectedTrack->frames[selectedFrame].key;
            }

            if (timelineAnimation != 0 && selectedTrack == 0 && selectedFrame >= 0) {
                for (Track* t = a->document->TrackIterator(timelineAnimation); t != 0; t = a->document->TrackIterator(t)) {
                    if (t->frames[selectedFrame].key) {
                        disable_delete = false;
                        break;
                    }
                }
            }

            if (disable_delete) {
                Imgui::Rect iconRect = control;
                iconRect.x += 2; // Footer specific
                iconRect.y += 3;
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_CLEARKEY, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }
            else {
                if (timelineAnimation != 0 && selectedTrack == 0 && selectedFrame >= 0) {
                    if (Imgui::FooterButton(control, IMGUI_ICON_CLEARKEY, "Remove all keys for frame", s->hierarchyFooterButtonIcon)) {
                        for (Track* t = a->document->TrackIterator(timelineAnimation); t != 0; t = a->document->TrackIterator(t)) {
                            if (t->avtive && t->frames[selectedFrame].key) {
                                a->document->ClearFrame(t, selectedFrame);
                            }
                        }
                    }
                }
                else {
                    if (Imgui::FooterButton(control, IMGUI_ICON_CLEARKEY, "Remove keyframe", s->hierarchyFooterButtonIcon)) {
                        a->document->ClearFrame(selectedTrack, selectedFrame);
                    }
                }
            }

            control.x -= control.w;
            control.x -= 2;
            //ImguiKeyButton(control, a, s);

            if (timelineAnimation != 0 && selectedTrack != 0 && selectedFrame >= 0) {
                const char* label = "Update keyframe";
                if (!selectedTrack->frames[selectedFrame].key) {
                    label = "Insert keyframe";
                }
                if (Imgui::FooterButton(control, IMGUI_ICON_TIMELINEKEY, label, s->hierarchyFooterButtonIcon)) {
                    a->document->SetFrame(selectedTrack, selectedFrame, a->document->GetSelectedInterpolationType());
                }
            }
            else if (timelineAnimation != 0 && selectedFrame >= 0 && selectedTrack == 0) {
                if (Imgui::FooterButton(control, IMGUI_ICON_TIMELINEKEY, "Keyframe all tracks", s->hierarchyFooterButtonIcon)) {
                    for (Track* t = a->document->TrackIterator(timelineAnimation); t != 0; t = a->document->TrackIterator(t)) {
                        if (t->avtive) {
                            a->document->SetFrame(t, selectedFrame, a->document->GetSelectedInterpolationType());
                        }
                    }
                }
            }
            else {
                Imgui::Rect iconRect = control;
                iconRect.x += 2; // Footer specific
                iconRect.y += 3;
                Imgui::Icon(iconRect, s->hierarchyFooterButtonSize - 4, IMGUI_ICON_TIMELINEKEY, s->hierarchyFooterDisabledIconColor.r, s->hierarchyFooterDisabledIconColor.g, s->hierarchyFooterDisabledIconColor.b);
            }

            control.x -= control.w * 4.5;
            control.w *= 4.5;
            control.x -= 2;

            const char* labels[5] = {
                "Linear", "Step", "Ease In", "Ease Out", "Ease In Out"
            };

            float right_side_of_dropdown = control.x + control.w;
            float control_width = right_side_of_dropdown - x_pos_for_dropdown;
            if (control_width >= control.w) {
                control.w = control_width;
                control.x = x_pos_for_dropdown;
            }

            i32 interpolationSelectionIndex = a->document->GetInterpolationIndex();
            i32 newINdex = Imgui::BeginComboBox(control, labels[interpolationSelectionIndex], 5, interpolationSelectionIndex, 0, false, timelineAnimation == 0);
            if (timelineAnimation != 0) {
                for (int xxy = 0; xxy < 5; ++xxy) {
                    Imgui::PushComboBoxItem(labels[xxy]);
                }
            }
            if (newINdex != interpolationSelectionIndex) {
                a->document->SetInterpolationIndex(newINdex);
            }
            Imgui::EndComboBox();
        }

        // Scroll bar
        Imgui::Rect sideScrollBar = timelineLeft;
        sideScrollBar.x = timelineLeft.x + timelineLeft.w - s->scrollBarSize;
        sideScrollBar.y = body.y;
        sideScrollBar.h = body.h;
        sideScrollBar.w = s->scrollBarSize;

        float sideScrollContentHeight = 0.0f;
        if (timelineAnimation != 0) {
            sideScrollContentHeight = numTracks * s->listBoxItemHeight;
        }
        a->frameUpScroll = Imgui::VScroll(sideScrollBar, a->frameUpScroll, FigureOutScrollBarHeight(sideScrollContentHeight, sideScrollBar.h), Imgui::HandleScroll(nodesScrollARea));

        // Footer
        {
            Imgui::Rect selectedTimelineArea = footer;
            selectedTimelineArea.x = timelineLeft.x;
            selectedTimelineArea.w = timelineLeft.w;

            selectedTimelineArea.w -= s->scrollBarSize;
            selectedTimelineArea.w -= 2;
            //Imgui::HScroll(bottomScrollBar, 0.0f, 15.0f, false);

            Imgui::Rect synchButton = selectedTimelineArea;
            synchButton.x = selectedTimelineArea.x + selectedTimelineArea.w + 2;
            synchButton.y += 1;
            synchButton.h -= 2;
            synchButton.w = s->scrollBarSize;
            synchButton.w -= 1;

            // Draw synch button
            ImguiSynchAnimViewIcon(synchButton, a, s, false);

            // Draw dropdown box
            u32 numAnims = a->document->GetNumAnimations() + 1;
            i32 animIndex = -1;
            const char* displayString = "None";
            if (timelineAnimation != 0) {
                int counter = 1;
                for (Animation* iter = a->document->AnimationIterator(0); iter != 0; iter = a->document->AnimationIterator(iter), ++counter) {
                    if (iter == timelineAnimation) {
                        animIndex = counter;
                        displayString = iter->name;
                        break;
                    }
                }
            }

            bool disabled = numAnims == 1;
            if (a->synchAnimView) {
                disabled = true;
            }

            i32 selection = Imgui::BeginComboBox(selectedTimelineArea, displayString, disabled ? 0 : numAnims, animIndex, 0, true, disabled);
            if (a->synchAnimView) {
                selection = animIndex;
            }

            Animation* selectionPtr = 0;
            if (numAnims > 1) {
                Imgui::PushComboBoxItem("None");
                if (selection == 0) {
                    selectionPtr = 0;
                }
                i32 counter = 1;
                for (Animation* iter = a->document->AnimationIterator(0); iter != 0; iter = a->document->AnimationIterator(iter), ++counter) {
                    if (counter == selection) {
                        selectionPtr = iter;
                    }
                    Imgui::PushComboBoxItem(iter->name);
                }
            }
            Imgui::EndComboBox();

            if (selection != animIndex) {
                a->document->SelectTimeline(selectionPtr);
                a->playSelectedAnimation = false;
                a->autoKey = false;
            }
        }
    }

    u32 numFrames = 0;
    if (timelineAnimation != 0) {
        numFrames = timelineAnimation->frameCount;
    }

    float scrollableWidth = numFrames * s->animationFrameWidth;

    { // Frame area
        Imgui::Rect control = timelineRight;
        control.h = header.h;

        Imgui::Rect bottomScrollBar = footer;
        bottomScrollBar.x = timelineRight.x;
        bottomScrollBar.w = timelineRight.w;

        u32 timelineSelectedFrame = a->document->GetSelectedFrame();

        a->frameSideScroll = Imgui::HScroll(bottomScrollBar, a->frameSideScroll, FigureOutScrollBarHeight(scrollableWidth, bottomScrollBar.w), false);
        u32 timelineFrame = timelineSelectedFrame;

        Imgui::Rect timelineRect(control.x, control.y, bottomScrollBar.w, control.h - 2);
        Imgui::Rect clipRect(timelineRect.x, timelineRect.y, timelineRect.w, timelineRight.h - timelineRect.h);
        if (timelineAnimation != 0) {
            Draw2D::PushClip(clipRect.x, clipRect.y, clipRect.w, clipRect.h);

            Imgui::Rect row = clipRect;
            row.y += timelineRect.h;
            row.h = s->listBoxItemHeight;

            float highlight_w = row.w;
            if (scrollableWidth < highlight_w) {
                highlight_w = scrollableWidth;
            }

            float x_offset = 0.0f;
            if (timelineRect.w < scrollableWidth) {
                float delta = scrollableWidth - timelineRect.w;
                x_offset = Math01(a->frameSideScroll) * delta;
            }

            // Draw BG pattern
            int i = 0;
            int k = 0;
            Track* selectedTrack = a->document->GetSelectedTrack();
            for (Track* t = a->document->TrackIterator(timelineAnimation); t != 0; t = a->document->TrackIterator(t), ++i) {
                if (!t->avtive) {
                    continue;
                }

                StyleColor bg = a->style.hierarchyItemBG_A;
                if ((k++) % 2 != 0) {
                    bg = a->style.hierarchyItemBG_B;
                }


                Draw2D::DrawRect(row.x, row.y - tracksScrollOffset, row.w, row.h, bg.r, bg.g, bg.b);
                if (t == selectedTrack) {
                    bg.r *= 1.2f;//a->style.hierarchyItemBG_Selected.r;
                    bg.g *= 1.2f;//a->style.hierarchyItemBG_Selected.g;
                    bg.b *= 1.5f;// a->style.hierarchyItemBG_Selected.b;
                    Draw2D::DrawRect(row.x, row.y - tracksScrollOffset, highlight_w, row.h, bg.r, bg.g, bg.b);
                }


                for (int f = 0; f < t->frameCount; ++f) {
                    Imgui::Rect frame = row;

                    frame.x = row.x + (float)f * s->animationFrameWidth;
                    frame.w = s->animationFrameWidth;
                    frame.y -= tracksScrollOffset;
                    frame.x -= x_offset;

                    if (f == timelineFrame) {
                        if (t != selectedTrack) {
                            bg.r *= 1.2f;//a->style.hierarchyItemBG_Selected.r;
                            bg.g *= 1.2f;//a->style.hierarchyItemBG_Selected.g;
                            bg.b *= 1.5f;// a->style.hierarchyItemBG_Selected.b;
                        }
                        else {
                            bg = s->hierarchyItemBG_Selected;
                            bg.r *= 0.8f;//a->style.hierarchyItemBG_Selected.r;
                            bg.g *= 0.8f;//a->style.hierarchyItemBG_Selected.g;
                            bg.b *= 0.9f;// a->style.hierarchyItemBG_Selected.b;
                        }
                        Draw2D::DrawRect(frame.x, frame.y, frame.w, frame.h, bg.r, bg.g, bg.b);
                    }

                    if (t->frames[f].key) {
                        frame.y += 11;
                        frame.x += 1;
                        Imgui::Icon(frame, 8, IMGUI_ICON_DIAMOND,
                            s->keyframeDiamond.r, s->keyframeDiamond.g, s->keyframeDiamond.b);
                        frame.y -= 11;

                        Imgui::Rect label_rect = frame;
                        label_rect.h = 8;
                        label_rect.y += 1;
                        label_rect.x += 2;
                        if (t->frames[f].interp == InterpolationType::Linear) {
                            Imgui::Label(label_rect, "L", false, false, true, &s->keyframeDiamond);
                        }
                        else if (t->frames[f].interp == InterpolationType::Step) {
                            Imgui::Label(label_rect, "S", false, false, true, &s->keyframeDiamond);
                        }
                        else if (t->frames[f].interp == InterpolationType::EaseIn) {
                            Imgui::Label(label_rect, "I", false, false, true, &s->keyframeDiamond);
                        }
                        else if (t->frames[f].interp == InterpolationType::EaseOut) {
                            Imgui::Label(label_rect, "O", false, false, true, &s->keyframeDiamond);
                        }
                        else if (t->frames[f].interp == InterpolationType::EaseInOut) {
                            Imgui::Label(label_rect, "E", false, false, true, &s->keyframeDiamond);
                        }
                    }
                }

                row.y += row.h;
            }

            // Draw timeline
            i32 selectedFrame = Imgui::Timeline(timelineRect, timelineAnimation->frameCount, timelineSelectedFrame, Math01(a->frameSideScroll));
            if (selectedFrame != timelineSelectedFrame) {
                a->document->SetSelectedFrame(selectedFrame);
                a->playSelectedAnimation = false;
            }

            Draw2D::PopClip();

            Imgui::Rect clickArea = clipRect;
            clickArea.y += timelineRect.h;

            float areaHeight = clickArea.h - timelineRect.h;
            float rowsHeight = s->listBoxItemHeight * (float)(numTracks);

            if (areaHeight < rowsHeight) {
                clickArea.h = areaHeight;
            }
            else {
                clickArea.h = rowsHeight;
            }



            //Draw2D::DrawRect(clickArea.x, clickArea.y, clickArea.w, clickArea.h);
            if (Imgui::HoldArea(clickArea) && timelineAnimation != 0) {
                Imgui::Point mousePos = Imgui::GetPointer();
                if (mousePos.x >= clickArea.x && mousePos.x <= clickArea.x + clickArea.w) {
                    if (mousePos.y >= clickArea.y && mousePos.y <= clickArea.y + clickArea.h) {
                        Imgui::Point adjustedMouse(
                            mousePos.x - clickArea.x + x_offset,
                            mousePos.y - clickArea.y + tracksScrollOffset
                        );

                        float t_x = adjustedMouse.x / scrollableWidth;
                        float t_y = adjustedMouse.y / scrollableHeight;
                        if (t_x >= 0.0f && t_x < 1.0f) {
                            if (t_y >= 0.0f && t_y < 1.0f) {
                                i32 x_index = t_x * (float)(numFrames);
                                i32 y_index = t_y * (float)(numTracks);
                                PlatformAssert(x_index >= 0, __LOCATION__);
                                PlatformAssert(y_index >= 0, __LOCATION__);
                                PlatformAssert(x_index < numFrames, __LOCATION__);
                                PlatformAssert(y_index < numTracks, __LOCATION__);

                                int xxx = 0;
                                Track* iter = timelineAnimation->tracks;
                                for (int y = 0; y < timelineAnimation->numTracks; ++y) {
                                    if (!iter->avtive) {
                                        iter = iter->next;
                                        continue;
                                    }
                                    
                                    PlatformAssert(iter != 0, __LOCATION__);
                                    
                                    
                                    if (xxx == y_index) {
                                        break;
                                    }
                                    iter = iter->next;

                                    xxx += 1;

                                    if (iter == 0) {
                                        break;
                                    }
                                }
                                if (iter != 0) {
                                    a->document->SelectTrack(iter);
                                }

                                a->document->SetSelectedFrame(x_index);
                                a->playSelectedAnimation = false;
                            }
                        }
                    }
                }
            }

            if (rowsHeight < areaHeight) {
                float delta = areaHeight - rowsHeight;
                if (delta > 0.0f) {
                    Imgui::Rect fillMissing = clickArea;
                    fillMissing.y += clickArea.h;
                    fillMissing.h = delta;

                    //Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
                    //Draw2D::DrawRect(fillMissing.x, fillMissing.y, fillMissing.w, fillMissing.h);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
                    if (Imgui::ClickArea(fillMissing)) {
                        a->document->SetSelectedFrame(-1);
                        a->playSelectedAnimation = false;
                    }
                }
            }

        }
    }
}

// Used as a helper for AdjustNodeTransformForGizmoPerTrack
static void UpdateSingleValueByTrackType(Application* a, float* target, TrackType trackType) {
    Animation* timelineAnim = a->document->GetTimelineAnimation();
    Node2D* selectedNode = a->document->GetSelectedNode();
    i32 selectedFrame = a->document->GetSelectedFrame();

    Track* track = 0;
    if (timelineAnim != 0) {
        track = ContainsTrack(timelineAnim, selectedNode, trackType);
    }
    if (track != 0) {
        if (selectedNode->IsPropertyDirty(trackType)) {
            *target = selectedNode->GetPropertyBuffer<f32>(trackType);
        }
        else {
            if (selectedFrame == -1) { // No frame selected, show frame 0
                if (track != 0 && track->frames[0].key) { // Frame 0 was a key, show it's value
                    *target = track->frames[0].fValue;
                } // else use default
            }
            else {
                if (track != 0 && track->frames[selectedFrame].key) { // Frame 0 was a key, show it's value
                    *target = track->frames[selectedFrame].fValue;
                }
                else {
                    f32 interpolated_value = track->InterpolateF(selectedFrame, selectedNode->GetProperty<f32>(trackType), timelineAnim->loop);
                    *target = interpolated_value;
                }
            }
        }
    } // Else use default
}

// Used to get the actual node transform (since it can be effected by application state) for ImguiSceneView
static Transform AdjustNodeTransformForGizmoPerTrack(Application* a, Transform result) {
    Node2D* selectedNode = a->document->GetSelectedNode();
    i32 selectedFrame = a->document->GetSelectedFrame();
    Animation* timelineAnim = a->document->GetTimelineAnimation();

    UpdateSingleValueByTrackType(a, &result.position.x, TrackType::TransformPositionX);
    UpdateSingleValueByTrackType(a, &result.position.y, TrackType::TransformPositionY);
    UpdateSingleValueByTrackType(a, &result.scale.x, TrackType::TransformScaleX);
    UpdateSingleValueByTrackType(a, &result.scale.y, TrackType::TransformScaleY);

    f32 rotationInDegrees = getAngle(result.rotation) * MATH_RAD2DEG;
    if (result.rotation.z < 0.0f) {
        rotationInDegrees *= -1.0f;
    }
    UpdateSingleValueByTrackType(a, &rotationInDegrees, TrackType::TransformRotation);
    result.rotation = angleAxis(rotationInDegrees * MATH_DEG2RAD, vec3(0, 0, 1));

    return result;
}

void SetGizmoPropInNode(Application* a, Node2D* targetNode, TrackType trackType, float new_val) {
    Animation* timelineAnim = a->document->GetTimelineAnimation();
    i32 selectedFrame = a->document->GetSelectedFrame();

    if (targetNode != 0) {
        Track* track = 0;
        if (timelineAnim != 0) {
            track = ContainsTrack(timelineAnim, targetNode, trackType);
        }

        if (track != 0) {
            if (selectedFrame == a->document->GetLastSelectedFrame() && a->autoKey && track != 0 && selectedFrame >= 0) {
                Frame dummy = { 0 };
                dummy.fValue = new_val;
                targetNode->SetBuffered<f32>(trackType, new_val);
                a->document->AutoKeyFrameValue(track, selectedFrame, a->document->GetSelectedInterpolationType(), dummy.uValue);
            }
            else if (track != 0 && track->numKeyFrames > 0) {
                targetNode->SetBuffered<f32>(trackType, new_val);
            }
            else {
                if (a->document->UpdateNodeTransformSingleF(targetNode, new_val, trackType)) {
                    if (a->autoKey && track != 0 && selectedFrame >= 0) {
                        Frame dummy = { 0 };
                        dummy.fValue = new_val;
                        a->document->AutoKeyFrameValue(track, selectedFrame, a->document->GetSelectedInterpolationType(), dummy.uValue);
                    }
                }
            }
        }
        else {
            if (a->document->UpdateNodeTransformSingleF(targetNode, new_val, trackType)) {
                if (a->autoKey && track != 0 && selectedFrame >= 0) {
                    Frame dummy = { 0 };
                    dummy.fValue = new_val;
                    a->document->AutoKeyFrameValue(track, selectedFrame, a->document->GetSelectedInterpolationType(), dummy.uValue);
                }
            }
        }
    }
}

static void ImguiSceneView(const Imgui::Rect& area, Application* a) {
    StyleSheet* s = &a->style;

    Imgui::Rect toolBar = area;
    toolBar.w = s->toolbarWidth;

    Imgui::Rect header = area;
    header.h = s->headerHeight;
    //header.w -= toolBar.w;
    //header.x += toolBar.w;

    Imgui::Rect content = area;
    content.w -= toolBar.w;
    content.x += toolBar.w;
    content.h -= header.h;
    content.y += header.h;

    Imgui::Point _mouse = Imgui::GetPointer();
    Imgui::Point _prevMouse = Imgui::GetPrevPointer();
    Imgui::Point _mouseDelta = Imgui::GetPointerDelta();
    vec2 mouse(_mouse.x, _mouse.y);
    vec2 prevMouse(_prevMouse.x, _prevMouse.y);
    vec2 mouseDelta(_mouseDelta.x, _mouseDelta.y);
    vec2 offset(content.x, content.y);

    Document* d = a->document;

    bool zoom_to_point = false;
    bool pan_delta_mouse = false;

    float ZOOM_TOOL_STEP = 15.0f;
    bool zoomIn = a->zoomIn;

    if (a->activeTool == ActiveTool::Pan && content.Contains(_mouse)) {
        Imgui::SetTooltipIcon(IMGUI_ICON_MOVETOOL);
        if (Imgui::HoldArea(content)) {
            pan_delta_mouse = true;
        }
    }
    else if (a->activeTool == ActiveTool::Zoom && content.Contains(_mouse)) {
        Imgui::SetTooltipIcon(a->zoomIn ? IMGUI_ICON_ZOOM_IN : IMGUI_ICON_ZOOM_OUT);

        vec2 viewPos(a->view.x, a->view.y);
        Imgui::HoldArea(content, &a->zoomDetails, &a->view.z, &viewPos);

        if (a->zoomDetails.activated) {
            vec2 delta = mouse - a->zoomDetails.mouseDown;
            if (lenSq(delta) <= 3.0f * 3.0f) {
                zoom_to_point = true;
            }
        }
        else if (a->zoomDetails.active) {
            vec2 delta = mouse - a->zoomDetails.mouseDown;
            float sqLen = lenSq(delta);

            if (sqLen >= 3.0f * 3.0f) {
                float old_t = (a->zoomDetails.rememberF32OnClick + 500.0f) / 1000.0f;

                float tt = sqLen / (400.0f * 400.0f);
                float inv_t = 1.0f - old_t;
                float step_t = inv_t * tt;
                if (!a->zoomIn) {
                    step_t *= -1.0f;
                }
                float new_t = old_t + step_t;
                float new_tt = old_t + step_t;

                old_t *= 2.0f;
                old_t += 0.1f;
                new_t *= 2.0f;
                new_t += 0.1f;

                float oldMouseX = old_t * (mouse.x);
                float newMouseX = new_t * (mouse.x);
                float oldMouseY = old_t * (mouse.y);
                float newMouseY = new_t * (mouse.y);

                a->view.z = (new_tt * 1000.0f) - 500.0f;


                if (a->view.z < -500.0f) {
                    a->view.z = -500.0f;
                }
                else if (a->view.z > 500.0f) {
                    a->view.z = 500.0f;
                }
                else {
                    if (!a->zoomIn) {
                        a->view.x = a->zoomDetails.rememberVec2OnClick.x + (newMouseX - oldMouseX);
                        a->view.y = a->zoomDetails.rememberVec2OnClick.y + (newMouseY - oldMouseY);
                    }
                    else {
                        a->view.x = a->zoomDetails.rememberVec2OnClick.x + (newMouseX - oldMouseX);
                        a->view.y = a->zoomDetails.rememberVec2OnClick.y + (newMouseY - oldMouseY);
                    }
                }
            }
            else {
                a->view.z = a->zoomDetails.rememberF32OnClick;
                a->view.x = a->zoomDetails.rememberVec2OnClick.x;
                a->view.y = a->zoomDetails.rememberVec2OnClick.y;
            }

            if (a->view.z < -500.0f) {
                a->view.z = -500.0f;
            }
            if (a->view.z > 500.0f) {
                a->view.z = 500.0f;
            }
        }
    }
    else {
        Imgui::Dummy();
    }

    // No matter the tool, zoom on mouse wheel
    if (content.Contains(_mouse)) {
        i32 scroll = MouseGetScroll();
        if (scroll < 0.0f) {
            zoom_to_point = true;
            zoomIn = false;
        }
        else if (scroll > 0.0f) {
            zoom_to_point = true;
        }

        if (MouseDown(MouseButtonMiddle) ||
            MouseDown(MouseButtonRight)) {
            pan_delta_mouse = true;
        }
    }

    if (pan_delta_mouse) {
        a->view.x -= _mouseDelta.x;
        a->view.y -= _mouseDelta.y;
    }

    if (zoom_to_point) {
        float old_t = (a->view.z + 500.0f) / 1000.0f;
        old_t *= 2.0f;
        old_t += 0.1f;

        a->view.z += ZOOM_TOOL_STEP * (zoomIn ? 1.0f : -1.0f);
        if (a->view.z > 500.0f) {
            a->view.z = 500.0f;
        }
        if (a->view.z < -500.0f) {
            a->view.z = -500.0f;
        }

        float new_t = (a->view.z + 500.0f) / 1000.0f;
        new_t *= 2.0f;
        new_t += 0.1f;

        float oldMouseX = old_t * (mouse.x);
        float newMouseX = new_t * (mouse.x);
        float oldMouseY = old_t * (mouse.y);
        float newMouseY = new_t * (mouse.y);

        a->view.x += (newMouseX - oldMouseX);
        a->view.y += (newMouseY - oldMouseY);
    }

    bool autoSelect = false;
    bool autoSelectFound = false;
    Draw2D::OBB autoSelectOBB = { 0 };
    Transform autoSelectTransform;
    Node2D* autoHighlightedNode = 0;

    Node2D* selectedNode = a->document->GetSelectedNode();
    Draw2D::OBB selectedNodeObb = { 0 };
    Transform selectedNodeTransform;

    if (a->activeTool == ActiveTool::Move && a->moveSelect) {
        autoSelect = true;
    }
    else if (a->activeTool == ActiveTool::Rotate && a->rotateSelect) {
        autoSelect = true;
    }
    else if (a->activeTool == ActiveTool::Scale && a->scaleSelect) {
        autoSelect = true;
    }
    else if (a->activeTool == ActiveTool::Pivot && a->pivotSelect) {
        autoSelect = true;
    }

    Draw2D::PushClip(content.x, content.y, content.w, content.h);

    float val = (a->view.z + 500.0f) / 1000.0f;
    val *= 2.0f;
    val += 0.1f;
    Transform view(vec3(-a->view.x, -a->view.y, 0), quat(), vec3(val, val, 1));
    vec2 motion = (mouse - a->effectorMousePos) * vec2(1.0f / view.scale.x, 1.0f / view.scale.y);
    f32 invScaleUnitSize = 1.0f / 60.0f;

    vec2 backup_position(0, 0);
    float backup_rotation = 0.0f;
    vec2 backup_scale(1, 1);
    // If there is an effecting tool active, temp replace the nodes local transform.
    if (a->effectorNode != 0) {
        backup_position = a->effectorNode->position;
        backup_rotation = a->effectorNode->rotationAngles;
        backup_scale = a->effectorNode->scale;

        a->effectorNode->position = a->effectorTransform.position.asVec2;
        a->effectorNode->rotationAngles = getAngle(a->effectorTransform.rotation) * MATH_RAD2DEG;
        if (a->effectorTransform.rotation.z < 0.0f) {
            a->effectorNode->rotationAngles *= -1.0f;
        }
        a->effectorNode->scale = a->effectorTransform.scale.asVec2;

        

        if (a->effector == ActiveTool::Move) {
            if (a->moveSnap) {
                motion.x = motion.x - MathFmod(motion.x, a->moveSnapVal);
                motion.y = motion.y - MathFmod(motion.y, a->moveSnapVal);
            }

            vec2 axis = a->effectorAxis;

            if (a->moveWorld) {
                quat rot = quat();
                if (a->effectorNode->parent != 0) {
                    rot = a->document->GetWorldTransform(a->effectorNode->parent).rotation;
                }
                f32 rotationAngles = getAngle(rot);
                if (rot.z < 0.0f) {
                    rotationAngles *= -1.0f;
                }
                if (lenSq(axis) > 0.0f) {
                    axis = rotate(axis, -rotationAngles);
                }
            }
            vec2 new_pos = a->effectorNode->position + motion;

            if (lenSq(axis) > 0) {
                if (lenSq(motion) > 0) {
                    new_pos = a->effectorNode->position + (axis * dot(motion, axis));
                }
            }
            else if (lenSq(motion) > 0) {
                quat rot = quat();
                    rot = a->document->GetWorldTransform(a->effectorNode->parent).rotation;
                f32 rotationAngles = getAngle(rot);
                if (rot.z < 0.0f) {
                    rotationAngles *= -1.0f;
                }
                
                new_pos = a->effectorNode->position + rotate(motion, -rotationAngles);
            }

            a->effectorNode->position = new_pos;
            a->effectorNode->transformToolActive = true;
        }
        else if (a->effector == ActiveTool::Scale) {
            if (a->scaleSnap) {
                motion.x = motion.x - MathFmod(motion.x, a->scaleSnapVal);
                motion.y = motion.y - MathFmod(motion.y, a->scaleSnapVal);
            }

            vec2 axis = normalized(a->effectorAxis);

            vec2 new_scale = (a->effectorTransform.scale.asVec2) + rotate(motion * invScaleUnitSize * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation));
            
            if (lenSq(axis) > 0) {
                if (lenSq(motion) > 0) {
                    new_scale = a->effectorTransform.scale.asVec2 + (axis * dot(rotate(motion * invScaleUnitSize * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation)), axis));
                }
            }
            else if (lenSq(motion) > 0) {
                quat rot = quat();
                rot = a->document->GetWorldTransform(a->effectorNode->parent).rotation;
                f32 rotationAngles = getAngle(rot);
                if (rot.z < 0.0f) {
                    rotationAngles *= -1.0f;
                }

                new_scale = (a->effectorTransform.scale.asVec2) + rotate((motion * invScaleUnitSize) * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation));
            }

            a->effectorNode->scale = new_scale;
            a->effectorNode->transformToolActive = true;
        }
        else if (a->effector == ActiveTool::Rotate) {
            float angle = getAngle(a->effectorTransform.rotation); 
            if (a->effectorTransform.rotation.z < 0.0f) {
                angle *= -1.0f;
            }
            angle = angle + a->effectorAxis.y;

            a->effectorNode->rotationAngles = angle * MATH_RAD2DEG;;
            a->effectorNode->transformToolActive = true;
        }
    }

    for (Node2D* iter = d->Sorted(0); iter != 0; iter = d->Sorted(iter)) {
        Transform worldTransform = d->GetWorldTransform(iter);
        Transform nodeXForm = combine(view, worldTransform);

        vec3 origin(0, 0, 0);
        origin = transformPoint(nodeXForm, origin);
        float rotAngle = getAngle(nodeXForm.rotation);

        if (nodeXForm.rotation.z < 0.0f) {
            rotAngle *= -1.0f;
        }
        Draw2D::OBB boundingBox = { 0 };
        if (iter->sprite.visible && iter->sprite.resourceUID != 0) {
            boundingBox = Draw2D::DrawImage(iter->sprite.resourceUID,
                offset.x + origin.x, offset.y + origin.y,
                iter->sprite.sourceW, iter->sprite.sourceH,
                iter->sprite.sourceX, iter->sprite.sourceY,
                iter->sprite.sourceW, iter->sprite.sourceH,
                nodeXForm.scale.x, nodeXForm.scale.y,
                iter->sprite.pivotX, iter->sprite.pivotY,
                rotAngle,
                iter->sprite.tintR, iter->sprite.tintG, iter->sprite.tintB);
        }
        else {
            boundingBox = Draw2D::ImageTransform(
                offset.x + origin.x, offset.y + origin.y,
                0, 0,
                nodeXForm.scale.x, nodeXForm.scale.y,
                iter->sprite.pivotX, iter->sprite.pivotY,
                rotAngle);
        }

        if (selectedNode == iter) {
            selectedNodeObb = boundingBox;
            selectedNodeTransform = nodeXForm;
        }

        if (autoSelect) {
            if (boundingBox.Contains(mouse)) {
                autoSelectFound = true;
                autoSelectOBB = boundingBox;
                autoSelectTransform = nodeXForm;
                if (Imgui::PointerPressed()) {
                    a->document->SelectNode(iter);
                }
            }
        }
    }

    if ((a->effector == ActiveTool::Move && a->effectorNode != 0) ||
        (a->effector == ActiveTool::Scale && a->effectorNode != 0) ||
        (a->effector == ActiveTool::Rotate && a->effectorNode != 0)) {
        a->effectorNode->transformToolActive = false;
        a->effectorNode->position = backup_position;
        a->effectorNode->rotationAngles = backup_rotation;
        a->effectorNode->scale = backup_scale;
    }

    if (selectedNode != 0 && selectedNode->sprite.visible && selectedNode->sprite.resourceUID != 0 && a->gridHighlight) {
        float rotAngleRadians = getAngle(selectedNodeTransform.rotation);
        if (selectedNodeTransform.rotation.z < 0.0f) {
            rotAngleRadians *= -1.0f;
        }

        StyleColor highlight = s->hierarchyItemBG_Selected;

        vec2 rotated = rotate(selectedNodeObb.extents, rotAngleRadians);
        vec2 topLeft = selectedNodeObb.center - rotated;
        Draw2D::DrawRect(topLeft.x, topLeft.y, selectedNodeObb.extents.x * 2.0f, 2,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngleRadians);

        Draw2D::DrawRect(topLeft.x, topLeft.y, 2, selectedNodeObb.extents.y * 2.0f,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngleRadians);

        rotated = rotate(vec2(selectedNodeObb.extents.x, selectedNodeObb.extents.y * -1.0f), rotAngleRadians);
        vec2 topRight = selectedNodeObb.center + rotated;
        Draw2D::DrawRect(topRight.x, topRight.y, 2, selectedNodeObb.extents.y * 2.0f,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngleRadians);

        rotated = rotate(vec2(selectedNodeObb.extents.x, selectedNodeObb.extents.y * -1.0f), rotAngleRadians);
        vec2 bottomLeft = selectedNodeObb.center - rotated;
        Draw2D::DrawRect(bottomLeft.x, bottomLeft.y, selectedNodeObb.extents.x * 2.0f + 2, 2,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngleRadians);

        float min_size = 2.0f;
        float max_size = 25.0f;
        float tsize = (a->view.z + 500.0f) / 1000.0f;
        float quarter = min_size + (max_size - min_size) * tsize;

        vec3 origin(0, -quarter / selectedNodeTransform.scale.y, 0);
        origin = transformPoint(selectedNodeTransform, origin);

        Draw2D::DrawRect(origin.x + offset.x,
            origin.y + offset.y,
            2, quarter * 2.0f,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1.0f,
            0, 0.0f,
            rotAngleRadians);

        origin = vec3(-quarter / selectedNodeTransform.scale.x, 0, 0);
        origin = transformPoint(selectedNodeTransform, origin);

        Draw2D::DrawRect(origin.x + offset.x,
            origin.y + offset.y,
            quarter * 2.0f, 2,
            highlight.r, highlight.g, highlight.b, 1,
            1.0f, 1,
            0.0f, 0,
            rotAngleRadians);
    }

    if (autoSelectFound) {
        StyleColor highlight = s->hierarchyItemBG_Movable;

        float rotAngle = getAngle(autoSelectTransform.rotation);
        if (selectedNodeTransform.rotation.z < 0.0f) {
            rotAngle *= -1.0f;
        }

        vec2 rotated = rotate(autoSelectOBB.extents, rotAngle);
        vec2 topLeft = autoSelectOBB.center - rotated;
        Draw2D::DrawRect(topLeft.x, topLeft.y, autoSelectOBB.extents.x * 2.0f, 4,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngle);

        Draw2D::DrawRect(topLeft.x, topLeft.y, 4, autoSelectOBB.extents.y * 2.0f,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngle);

        rotated = rotate(vec2(autoSelectOBB.extents.x, autoSelectOBB.extents.y * -1.0f), rotAngle);
        vec2 topRight = autoSelectOBB.center + rotated;
        Draw2D::DrawRect(topRight.x, topRight.y, 4, autoSelectOBB.extents.y * 2.0f,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngle);

        rotated = rotate(vec2(autoSelectOBB.extents.x, autoSelectOBB.extents.y * -1.0f), rotAngle);
        vec2 bottomLeft = autoSelectOBB.center - rotated;
        Draw2D::DrawRect(bottomLeft.x, bottomLeft.y, autoSelectOBB.extents.x * 2.0f + 4, 4,
            highlight.r, highlight.g, highlight.b, 1,
            1, 1, 0, 0, rotAngle);
    }

    Animation* timelineAnim = a->document->GetTimelineAnimation();
    i32 selectedFrame = a->document->GetSelectedFrame();
    // Draw active Gizmo
    {
        float gizmoSize = 60.0f;
        float qSize = gizmoSize / 3.0f;
        vec3 gizmoOrigin = transformPoint(selectedNodeTransform, vec3(0.0f, 0.0f, 0.0f));

        float rotAngle = getAngle(selectedNodeTransform.rotation);
        if (selectedNodeTransform.rotation.z < 0.0f) {
            rotAngle *= -1.0f;
        }
        if (a->activeTool == ActiveTool::Move && a->moveWorld) {
            rotAngle = 0.0f;
        }

        if ((a->activeTool == ActiveTool::Move || a->activeTool == ActiveTool::Scale || a->activeTool == ActiveTool::Pivot) && selectedNode != 0) {
            bool multi_active = false;
            bool y_active = false;
            bool x_active = false;
            StyleColor handleColor = a->style.gizmoY;

            // X Y Handle
            {
                Draw2D::OBB gizmoObb = { 0 };
                gizmoObb.rotation = rotAngle;
                gizmoObb.extents = vec2(qSize * 0.5f, qSize * 0.5f);
                gizmoObb.center = vec2(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y) + rotate(vec2(qSize * 0.5f + 6, qSize * -0.5f - 6), rotAngle);

                bool handleActive = false;
                if (gizmoObb.Contains(mouse)) {
                    handleActive = true;
                    handleColor = a->style.gizmoY_Hover;
                    multi_active = true;

                    if (Imgui::PointerPressed()) {
                        a->effector = a->activeTool;
                        a->effectorAxis = vec2(0, 0); // Unlocked
                        a->effectorMousePos = mouse;
                        a->effectorNode = selectedNode;

                        a->effectorTransform = a->document->GetLocalTransform(selectedNode);
                        a->effectorTransform = AdjustNodeTransformForGizmoPerTrack(a, a->effectorTransform);
                    }
                }

                vec2 rotationRectTopLeft = vec2(gizmoOrigin.x, gizmoOrigin.y) + rotate(vec2(qSize * 0.5f, qSize * -0.5f), rotAngle);
                Draw2D::DrawRect(
                    rotationRectTopLeft.x + offset.x, rotationRectTopLeft.y + offset.y,
                    qSize, qSize,
                    handleColor.r, handleColor.g, handleColor.b, 1,
                    1, 1,
                    qSize * 0.5f - 6, qSize * 0.5f + 6,
                    rotAngle);
            }

            // Y handle
            {
                Draw2D::OBB gizmoObb = { 0 };
                gizmoObb.rotation = rotAngle;
                gizmoObb.extents = vec2(4, gizmoSize / 2.0f);

                vec2 gizmoAxis(0, -1);
                vec2 gizmoPerp(1, 0);

                gizmoObb.center = vec2(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y) + (gizmoAxis * gizmoSize * 0.5f);
                vec2 p0 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) - (gizmoPerp * qSize * 0.5f) + (gizmoAxis * gizmoSize);
                vec2 p1 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (gizmoPerp * qSize * 0.5f) + (gizmoAxis * gizmoSize);
                vec2 p2 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (gizmoAxis * qSize * 0.5f) + (gizmoAxis * gizmoSize);
                vec2 _o = vec2(gizmoOrigin.x, gizmoOrigin.y) + offset + (normalized(gizmoAxis) * gizmoSize);

                if ((a->activeTool == ActiveTool::Move && !a->moveWorld) ||
                    (a->activeTool == ActiveTool::Scale)) {
                   
                    float rotAngle = getAngle(selectedNodeTransform.rotation);
                    if (selectedNodeTransform.rotation.z < 0.0f) {
                        rotAngle *= -1.0f;
                    }
                    if (a->activeTool == ActiveTool::Move && a->moveWorld) {
                        rotAngle = 0.0f;
                    }
                    vec2 _gizmoAxis = rotate(gizmoAxis, rotAngle);
                    vec2 _gizmoPerp = rotate(gizmoPerp, rotAngle);

                    p0 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) - (_gizmoPerp * qSize * 0.5f) + (_gizmoAxis * gizmoSize);
                    p1 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (_gizmoPerp * qSize * 0.5f) + (_gizmoAxis * gizmoSize);
                    p2 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (_gizmoAxis * qSize * 0.5f) + (_gizmoAxis * gizmoSize);
                    gizmoObb.center = vec2(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y) + (_gizmoAxis * gizmoSize * 0.5f);
                    _o = vec2(gizmoOrigin.x, gizmoOrigin.y) + offset + (_gizmoAxis * gizmoSize);

                    rotAngle = selectedNode->rotationAngles * MATH_DEG2RAD;
                    gizmoAxis = rotate(gizmoAxis, rotAngle);
                    gizmoPerp = rotate(gizmoPerp, rotAngle);
                }


                handleColor = a->style.gizmoG;

                bool handleActive = false;
                if (gizmoObb.Contains(mouse)) {
                    if (!multi_active) {
                        handleActive = true;
                        handleColor = a->style.gizmoG_Hover;
                        y_active = true;
                    }
                }
                else {
                    gizmoObb.center = gizmoObb.center + gizmoAxis * gizmoSize * 0.5f;
                    gizmoObb.extents = vec2(qSize * 0.5f, qSize * 0.5f);
                    if (gizmoObb.Contains(mouse)) {
                        if (!multi_active) {
                            handleActive = true;
                            handleColor = a->style.gizmoG_Hover;
                            y_active = true;
                        }
                    }
                }

                if (handleActive) {
                    if (Imgui::PointerPressed()) {
                        a->effector = a->activeTool;
                        a->effectorMousePos = mouse;
                        a->effectorAxis = normalized(gizmoAxis);
                        a->effectorNode = selectedNode;

                        a->effectorTransform = a->document->GetLocalTransform(selectedNode);
                        a->effectorTransform = AdjustNodeTransformForGizmoPerTrack(a, a->effectorTransform);
                    }
                }

                if (a->activeTool == ActiveTool::Move) {
                    Draw2D::DrawTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, handleColor.r, handleColor.g, handleColor.b);
                }
                else {

                    Draw2D::DrawRect(_o.x, _o.y,
                        qSize, qSize, handleColor.r, handleColor.g, handleColor.b, 1,
                        1, 1, qSize * 0.5f, qSize * 0.5f, rotAngle);
                }


                Draw2D::DrawRect(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y,
                    4, gizmoSize,
                    handleColor.r, handleColor.g, handleColor.b, 1,
                    1, 1,
                    2, gizmoSize,
                    rotAngle);
            }

            // X handle
            {
                handleColor = a->style.gizmoR;

                Draw2D::OBB gizmoObb = { 0 };
                gizmoObb.rotation = rotAngle;
                gizmoObb.extents = vec2(gizmoSize / 2.0f, 4);

                vec2 gizmoAxis = vec2(1, 0);
                vec2 gizmoPerp = vec2(0, -1);

                vec2 p0 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) - (gizmoPerp * qSize * 0.5f) + (gizmoAxis * gizmoSize);
                vec2 p1 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (gizmoPerp * qSize * 0.5f) + (gizmoAxis * gizmoSize);
                vec2 p2 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (gizmoAxis * qSize * 0.5f) + (gizmoAxis * gizmoSize);
                gizmoObb.center = vec2(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y) + (gizmoAxis * gizmoSize * 0.5f);
                vec2 _o = vec2(gizmoOrigin.x, gizmoOrigin.y) + offset + (gizmoAxis * gizmoSize);

                if ((a->activeTool == ActiveTool::Move && !a->moveWorld) ||
                    (a->activeTool == ActiveTool::Scale)) {

                    float rotAngle = getAngle(selectedNodeTransform.rotation);
                    if (selectedNodeTransform.rotation.z < 0.0f) {
                        rotAngle *= -1.0f;
                    }
                    if (a->activeTool == ActiveTool::Move && a->moveWorld) {
                        rotAngle = 0.0f;
                    }
                    vec2 _gizmoAxis = rotate(gizmoAxis, rotAngle);
                    vec2 _gizmoPerp = rotate(gizmoPerp, rotAngle);

                    p0 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) - (_gizmoPerp * qSize * 0.5f) + (_gizmoAxis * gizmoSize);
                    p1 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (_gizmoPerp * qSize * 0.5f) + (_gizmoAxis * gizmoSize);
                    p2 = (vec2(gizmoOrigin.x, gizmoOrigin.y) + offset) + (_gizmoAxis * qSize * 0.5f) + (_gizmoAxis * gizmoSize);
                    gizmoObb.center = vec2(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y) + (_gizmoAxis * gizmoSize * 0.5f);
                    _o = vec2(gizmoOrigin.x, gizmoOrigin.y) + offset + (_gizmoAxis * gizmoSize);

                    rotAngle = selectedNode->rotationAngles * MATH_DEG2RAD;
                    gizmoAxis = rotate(gizmoAxis, rotAngle);
                    gizmoPerp = rotate(gizmoPerp, rotAngle);
                }

                bool handleActive = false;
                if (gizmoObb.Contains(mouse)) {
                    if (!(y_active || multi_active)) {
                        handleActive = true;
                        handleColor = a->style.gizmoR_Hover;
                        x_active = true;
                    }
                }
                else {
                    gizmoObb.center = gizmoObb.center + gizmoAxis * gizmoSize * 0.5f;
                    gizmoObb.extents = vec2(qSize * 0.5f, qSize * 0.5f);
                    if (gizmoObb.Contains(mouse)) {
                        if (!(y_active || multi_active)) {
                            handleActive = true;
                            handleColor = a->style.gizmoR_Hover;
                            x_active = true;
                        }
                    }
                }

                if (handleActive) {
                    if (Imgui::PointerPressed()) {
                        a->effector = a->activeTool;
                        a->effectorMousePos = mouse;
                        a->effectorAxis = normalized(gizmoAxis);
                        a->effectorNode = selectedNode;

                        a->effectorTransform = a->document->GetLocalTransform(selectedNode);
                        a->effectorTransform = AdjustNodeTransformForGizmoPerTrack(a, a->effectorTransform);
                    }
                }

                if (a->activeTool == ActiveTool::Move || a->activeTool == ActiveTool::Pivot) {
                    Draw2D::DrawTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, handleColor.r, handleColor.g, handleColor.b);
                }
                else {
                    Draw2D::DrawRect(_o.x, _o.y,
                        qSize, qSize, handleColor.r, handleColor.g, handleColor.b, 1,
                        1, 1, qSize * 0.5f, qSize * 0.5f, rotAngle);
                }

                Draw2D::DrawRect(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y, gizmoSize, 4, handleColor.r, handleColor.g, handleColor.b, 1, 1, 1, 0, 2, rotAngle);
            }
        }
        else if (a->activeTool == ActiveTool::Rotate && selectedNode != 0) {
            float innerR = gizmoSize;
            float outerR = gizmoSize + gizmoSize * 0.1f;

            StyleColor handleColor = a->style.gizmoB;
            vec2 delta = mouse - vec2(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y);
            float lSq = lenSq(delta);

            innerR -= 1.0f;
            outerR += 1.0f;
            if (lSq >= innerR * innerR && lSq <= outerR * outerR) {
                handleColor = a->style.gizmoB_Hover;
                if (Imgui::PointerPressed()) {
                    a->effector = a->activeTool;
                    a->effectorMousePos = mouse;
                    a->effectorAxis = vec2(0,0); // effectorAxis.x will be the angle accumulator
                    a->effectorNode = selectedNode;

                    a->effectorTransform = a->document->GetLocalTransform(selectedNode);
                    a->effectorTransform = AdjustNodeTransformForGizmoPerTrack(a, a->effectorTransform);
                }
            }
            innerR += 1.0f;
            outerR -= 1.0f;

            if (a->effector == a->activeTool) {
                vec2 center(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y);

                vec2 prevMouseOnCircle = normalized(prevMouse - center);// *(outerR - 1.0f);
                vec2 thisMouseOnCircle = normalized(mouse - center);// *(outerR - 1.0f);
                vec2 clickedOnCircle = normalized(a->effectorMousePos - center);// *(outerR - 1.0f);

                float angl = angle(prevMouseOnCircle, thisMouseOnCircle);
                if (angl > 0.00001f) {
                    if (dot(thisMouseOnCircle, perp(prevMouseOnCircle)) >= 0.0f) {
                        angl *= -1.0f;
                    }
                    a->effectorAxis.x += angl;
                    a->effectorAxis.y = a->effectorAxis.x;
                    if (a->rotateSnap) {
                        float snapInRadians = a->rotateSnapVal * MATH_DEG2RAD;
                        a->effectorAxis.y = a->effectorAxis.x - MathFmod(a->effectorAxis.x, snapInRadians);
                    }
                }
               
                float startAngle = MathAtan2(clickedOnCircle.y, clickedOnCircle.x);
                float endAngle = startAngle + a->effectorAxis.y;

                if (MathAbsF(startAngle - endAngle) > 2.0f * MATH_PI) {
                    endAngle = startAngle + 2.0f * MATH_PI - 0.001f;
                }
                else if (endAngle < startAngle) {
                    startAngle = MathFmod(startAngle, 2.0f * MATH_PI);
                    endAngle = MathFmod(endAngle, 2.0f * MATH_PI);
                    if (endAngle < startAngle) {
                        endAngle += 2.0f * MATH_PI;
                    }

                    float tmp = endAngle;
                    endAngle = startAngle;
                    startAngle = tmp;
                }

                Draw2D::DrawCircleSlice(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y,
                    innerR * 0.3f, startAngle, endAngle,
                    s->gizmoY_Hover.r, s->gizmoY_Hover.g, s->gizmoY_Hover.b
                );

                if (a->rotateSnap) {
                    thisMouseOnCircle = rotate(clickedOnCircle, a->effectorAxis.y);
                }

                clickedOnCircle = clickedOnCircle * (outerR - 1.0f);
                thisMouseOnCircle = thisMouseOnCircle * (outerR - 1.0f);
                float points[4] = { center.x, center.y,
                    center.x + clickedOnCircle.x, center.y + clickedOnCircle.y
                };
                Draw2D::DrawLine(points, 2, 2.0f, s->gizmoY_Hover.r, s->gizmoY_Hover.g, s->gizmoY_Hover.b);
                points[2] = center.x + thisMouseOnCircle.x;
                points[3] = center.y + thisMouseOnCircle.y;
                Draw2D::DrawLine(points, 2, 2.0f, s->gizmoY_Hover.r, s->gizmoY_Hover.g, s->gizmoY_Hover.b);
            }

            Draw2D::DrawHollowCircle(gizmoOrigin.x + offset.x, gizmoOrigin.y + offset.y,
                innerR, outerR,
                handleColor.r, handleColor.g, handleColor.b);
        }
    }

    // Apply actual move logic
    if (a->effector == ActiveTool::Move) {
        if (a->effectorNode != 0 && Imgui::PointerReleased()) {
            Node2D* effector = a->effectorNode;
            
            // Copy
            if (a->moveSnap) {
                motion.x = motion.x - MathFmod(motion.x, a->moveSnapVal);
                motion.y = motion.y - MathFmod(motion.y, a->moveSnapVal);
            }

            vec2 axis = a->effectorAxis;

            if (a->moveWorld) {
                quat rot = a->document->GetWorldTransform(a->effectorNode->parent).rotation;
                f32 rotationAngles = getAngle(rot);
                if (rot.z < 0.0f) {
                    rotationAngles *= -1.0f;
                }
                if (lenSq(axis) > 0.0f) {
                    axis = rotate(axis, -rotationAngles);
                }
            }
            vec2 new_pos = a->effectorNode->position + motion;

            if (lenSq(axis) > 0) {
                if (lenSq(motion) > 0) {
                    new_pos = a->effectorNode->position + (axis * dot(motion, axis));
                }
            }
            else if (lenSq(motion) > 0) {
                quat rot = a->document->GetWorldTransform(a->effectorNode->parent).rotation;
                f32 rotationAngles = getAngle(rot);
                if (rot.z < 0.0f) {
                    rotationAngles *= -1.0f;
                }

                new_pos = a->effectorNode->position + rotate(motion, -rotationAngles);
            }
            // End copy

            SetGizmoPropInNode(a, effector, TrackType::TransformPositionX, new_pos.x);
            SetGizmoPropInNode(a, effector, TrackType::TransformPositionY, new_pos.y);
        }
    }
    else if (a->effector == ActiveTool::Scale) {
        if (a->effectorNode != 0 && Imgui::PointerReleased()) {
            Node2D* effector = a->effectorNode;
            vec2 scale = a->effectorTransform.scale.asVec2;

            /*vec2 new_motion = motion;
            if (lenSq(a->effectorAxis) > 0.0f) {
                new_motion = (a->effectorAxis * dot(new_motion, a->effectorAxis));
            }
            vec2 new_scale = (a->effectorTransform.scale.asVec2) + rotate((new_motion * invScaleUnitSize) * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation));
            */

            vec2 axis = normalized(a->effectorAxis);

            vec2 new_scale = (a->effectorTransform.scale.asVec2) + rotate(motion * invScaleUnitSize * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation));

            if (lenSq(axis) > 0) {
                if (lenSq(motion) > 0) {
                    new_scale = a->effectorTransform.scale.asVec2 + (axis * dot(rotate(motion * invScaleUnitSize * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation)), axis));
                }
            }
            else if (lenSq(motion) > 0) {
                quat rot = quat();
                rot = a->document->GetWorldTransform(a->effectorNode->parent).rotation;
                f32 rotationAngles = getAngle(rot);
                if (rot.z < 0.0f) {
                    rotationAngles *= -1.0f;
                }

                new_scale = (a->effectorTransform.scale.asVec2) + rotate((motion * invScaleUnitSize) * vec2(1.0f, -1.0f), getAngle(a->effectorTransform.rotation));
            }

            SetGizmoPropInNode(a, effector, TrackType::TransformScaleX, new_scale.x);
            SetGizmoPropInNode(a, effector, TrackType::TransformScaleY, new_scale.y);
        }
    }
    else if (a->effector == ActiveTool::Rotate) {
        if (a->effectorNode != 0 && Imgui::PointerReleased()) {
            Node2D* effector = a->effectorNode;

            float angle = getAngle(a->effectorTransform.rotation);
            if (a->effectorTransform.rotation.z < 0.0f) {
                angle *= -1.0f;
            }
            angle = angle + a->effectorAxis.y;

            SetGizmoPropInNode(a, effector, TrackType::TransformRotation, angle* MATH_RAD2DEG);
        }
    }

    // Release pos & rot & scl
    if (Imgui::PointerReleased()) {
        a->effector = ActiveTool::None;
        a->effectorNode = 0;
    }

    Draw2D::PopClip();

    // Draw side toolbar
    Draw2D::DrawRect(toolBar.x, toolBar.y, toolBar.w, toolBar.h, s->toolBarBg.r, s->toolBarBg.g, s->toolBarBg.b, 1.0f);
    Draw2D::DrawRect(toolBar.x + toolBar.w - 1, toolBar.y, 1, toolBar.h, s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b, 1.0f);

    {
        Imgui::Rect toolbarButton = toolBar;
#define TOOLBAR_BTN_PADDING 12.0f
        toolbarButton.w = toolBar.w - TOOLBAR_BTN_PADDING;
        toolbarButton.x += TOOLBAR_BTN_PADDING / 2.0f;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;
        toolbarButton.h = toolbarButton.w;

        toolbarButton.y += TOOLBAR_BTN_PADDING * 2.0f + 1;

        if (Imgui::SidebarButton(toolbarButton, IMGUI_ICON_MOVE, "Translate", a->activeTool == ActiveTool::Move)) {
            a->SelectTool(ActiveTool::Move);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;

        if (Imgui::SidebarButton(toolbarButton, IMGUI_ICON_ROTATE, "Rotate", a->activeTool == ActiveTool::Rotate)) {
            a->SelectTool(ActiveTool::Rotate);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;

        if (Imgui::SidebarButton(toolbarButton, IMGUI_ICON_SCALE, "Scale", a->activeTool == ActiveTool::Scale)) {
            a->SelectTool(ActiveTool::Scale);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;

        /*if (Imgui::SidebarButton(toolbarButton, IMGUI_ICON_PIVOT, "Pivot", a->activeTool == ActiveTool::Pivot)) {
            a->SelectTool(ActiveTool::Pivot);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;*/

        Draw2D::DrawRect(toolbarButton.x - 2, toolbarButton.y, toolbarButton.w + 4, 2, s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b);
        Draw2D::DrawRect(toolbarButton.x - 2, toolbarButton.y, toolbarButton.w + 4, 1, s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;

        if (Imgui::SidebarButton(toolbarButton, IMGUI_ICON_MOVETOOL, "Pan", a->activeTool == ActiveTool::Pan)) {
            a->SelectTool(ActiveTool::Pan);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;

        if (Imgui::SidebarButton(toolbarButton, a->zoomIn ? IMGUI_ICON_ZOOM_IN : IMGUI_ICON_ZOOM_OUT, "Zoom", a->activeTool == ActiveTool::Zoom)) {
            a->SelectTool(ActiveTool::Zoom);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;

        if (Imgui::SidebarButton(toolbarButton, IMGUI_ICON_GRID, "Display options", a->activeTool == ActiveTool::Grid)) {
            a->SelectTool(ActiveTool::Grid);
        }
        toolbarButton.y += toolbarButton.h;
        toolbarButton.y += TOOLBAR_BTN_PADDING / 2.0f;
    }

    // Draw header toolbar
    Draw2D::DrawRect(header.x, header.y, header.w, header.h, s->toolBarBg.r, s->toolBarBg.g, s->toolBarBg.b);
    Draw2D::DrawRect(header.x, header.y + header.h - 1, header.w, 1, s->headerBgColor.r, s->headerBgColor.g, s->headerBgColor.b);

    {
        Imgui::Rect _toolbarButton(header.x + 3, header.y + 3, 30, header.h - 6);
        vec3 icon(3, 2, header.h - 10);
        if (Imgui::SidebarButton(_toolbarButton, IMGUI_ICON_APPICON, "Keyframe Studio Github", false, &icon)) {
            WindowOpenURL("https://github.com/gszauer/KeyframeStudio");
        }

        header.x += 38; // Just some padding

        Draw2D::DrawRect(toolBar.w - 2, header.y, 2, header.h, s->toolBarBg.r, s->toolBarBg.g, s->toolBarBg.b, 1.0f);
        Draw2D::DrawRect(toolBar.w - 1, header.y, 1, header.h, s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b, 1.0f);

        char textBuffer[32] = { 0 };
        if (a->activeTool == ActiveTool::Move || a->activeTool == ActiveTool::Rotate || a->activeTool == ActiveTool::Scale || a->activeTool == ActiveTool::Pivot) {
            Imgui::Rect control(header.x, header.y, header.w, header.h);
            control.y += 5;
            control.h -= 14;
            control.x += 5;
            control.w = 67;

            if (a->activeTool == ActiveTool::Move || a->activeTool == ActiveTool::Pivot) {
                Imgui::Label(control, "Tool space", false, false);
                control.x += 69;
                control.x += 5;
            }

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;

            if (a->activeTool == ActiveTool::Move) {
                i32 selected = a->moveWorld ? 1 : 0;
                a->moveWorld = Imgui::BeginComboBox(control, a->moveWorld ? "World" : "Local", 2, selected, 0, false, false);
            }
            if (a->activeTool == ActiveTool::Pivot) {
                i32 selected = a->pivotWorld ? 1 : 0;
                a->pivotWorld = Imgui::BeginComboBox(control, a->pivotWorld ? "World" : "Local", 2, selected, 0, false, false);
            }


            if (a->activeTool == ActiveTool::Move || a->activeTool == ActiveTool::Pivot) {
                Imgui::PushComboBoxItem("Local");
                Imgui::PushComboBoxItem("World");
                Imgui::EndComboBox();

                control.x += control.w;
                control.x += 15;
            }

            control.y = header.y + 5;
            control.h = header.h - 14;

            Imgui::Label(control, "Snap", false, false);
            control.x += 31;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;
            control.w = control.h;

            control.y += 1;
            if (a->activeTool == ActiveTool::Move) {
                a->moveSnap = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->moveSnap, "Snap");
            }
            else if (a->activeTool == ActiveTool::Rotate) {
                a->rotateSnap = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->rotateSnap, "Snap");
            }
            else if (a->activeTool == ActiveTool::Scale) {
                a->scaleSnap = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->scaleSnap, "Snap");
            }
            else if (a->activeTool == ActiveTool::Pivot) {
                a->pivotSnap = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->pivotSnap, "Snap");
            }
            control.y -= 1;

            control.w = 65;

            control.x += 25;

            if (a->activeTool == ActiveTool::Move) {
                stbsp_snprintf(textBuffer, 32, "%.2f", a->moveSnapVal);

                const char* text = Imgui::TextArea(control, textBuffer, 0, !a->moveSnap, true, false, false, false, false);
                if (text != textBuffer) {
                    f32 val = MathAToF(text);
                    a->moveSnapVal = MathAbsF(val);
                }
            }
            else if (a->activeTool == ActiveTool::Rotate) {
                stbsp_snprintf(textBuffer, 32, "%.2f", a->rotateSnapVal);

                const char* text = Imgui::TextArea(control, textBuffer, 0, !a->rotateSnap, true, false, false, false, false);
                if (text != textBuffer) {
                    f32 val = MathAToF(text);
                    a->rotateSnapVal = MathAbsF(val);
                }
            }
            else if (a->activeTool == ActiveTool::Scale) {
                stbsp_snprintf(textBuffer, 32, "%.2f", a->scaleSnapVal);

                const char* text = Imgui::TextArea(control, textBuffer, 0, !a->scaleSnap, true, false, false, false, false);
                if (text != textBuffer) {
                    f32 val = MathAToF(text);
                    a->scaleSnapVal = MathAbsF(val);
                }
            }
            else if (a->activeTool == ActiveTool::Pivot) {
                stbsp_snprintf(textBuffer, 32, "%.2f", a->pivotSnapVal);

                const char* text = Imgui::TextArea(control, textBuffer, 0, !a->pivotSnap, true, false, false, false, false);
                if (text != textBuffer) {
                    f32 val = MathAToF(text);
                    a->pivotSnapVal = MathAbsF(val);
                }
            }
            control.x += control.w;
            control.x += 15;

            control.y = header.y + 5;
            control.h = header.h - 14;

            Imgui::Label(control, "Auto select layer", false, false);
            control.x += 105;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;
            control.w = control.h;

            control.y += 1;
            if (a->activeTool == ActiveTool::Move) {
                a->moveSelect = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->moveSelect, "Auto Select");
            }
            else if (a->activeTool == ActiveTool::Rotate) {
                a->rotateSelect = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->rotateSelect, "Auto Select");
            }
            else if (a->activeTool == ActiveTool::Scale) {
                a->scaleSelect = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->scaleSelect, "Auto Select");
            }
            else if (a->activeTool == ActiveTool::Pivot) {
                a->pivotSelect = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->pivotSelect, "Auto Select");
            }
            control.y -= 1;
        }
        else if (a->activeTool == ActiveTool::Pan) {
            Imgui::Rect control(header.x, header.y, header.w, header.h);
            control.y += 5;
            control.h -= 14;
            control.x += 5;
            control.w = 67;

            Imgui::Label(control, "Viewport x", false);

            control.x += 72;
            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;
            stbsp_snprintf(textBuffer, 32, "%.2f", a->view.x);
            const char* text = Imgui::TextArea(control, textBuffer, 0, false, true, false, false, false, false);
            if (text != textBuffer) {
                float val = MathAToF(text);
                a->view.x = val;
            }
            control.y = header.y + 5;
            control.h = header.h - 14;

            control.x += control.w;
            control.x += 5;

            Imgui::Label(control, "y", false, true);

            control.x += 6;
            control.x += 5;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;

            stbsp_snprintf(textBuffer, 32, "%.2f", a->view.y);
            text = Imgui::TextArea(control, textBuffer, 0, false, true, false, false, false, false);
            if (text != textBuffer) {
                float val = MathAToF(text);
                a->view.y = val;
            }

            control.x += control.w;
            control.x += 15;

            control.y = header.y + 5;
            control.h = header.h - 14;

            Imgui::Label(control, "Zoom", false, false);
            control.x += 36;
            control.x += 5;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;

            stbsp_snprintf(textBuffer, 32, "%.2f", a->view.z);
            text = Imgui::TextArea(control, textBuffer, 0, false, true, false, false, false, false);
            if (text != textBuffer) {
                a->view.z = MathAToF(text);
            }
            control.x += control.w;
            control.x += 5;

            control.w = 200.0f;
            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;

            float val = (a->view.z + 500.0f) / 1000.0f;
            val = Imgui::FloatSlider(control, val);
            a->view.z = (val * 1000.0f) - 500.0f;

            if (a->view.z < -500.0f) {
                a->view.z = -500.0f;
            }
            if (a->view.z > 500.0f) {
                a->view.z = 500.0f;
            }
        }
        else if (a->activeTool == ActiveTool::Zoom) {
            Imgui::Rect control(header.x, header.y, header.w, header.h);
            control.y += 5;
            control.h -= 14;
            control.x += 5;
            control.w = 67;

            Imgui::Label(control, "Tool mode", false, false);
            control.x += 67;
            control.x += 5;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;
            control.w = 85;

            a->zoomIn = Imgui::BeginComboBox(control, a->zoomIn ? "Zoom in" : "Zoom out", 2, a->zoomIn ? 0 : 1, 0, false, false) ? false : true;
            Imgui::PushComboBoxItem("Zoom in");
            Imgui::PushComboBoxItem("Zoom out");
            Imgui::EndComboBox();

            control.x += control.w;
            control.x += 15;

            control.y = header.y + 5;
            control.h = header.h - 14;

            Imgui::Label(control, "Zoom", false, false);
            control.x += 36;
            control.x += 5;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;

            stbsp_snprintf(textBuffer, 32, "%.2f", a->view.z);
            const char* text = Imgui::TextArea(control, textBuffer, 0, false, true, false, false, false, false);
            if (text != textBuffer) {
                a->view.z = MathAToF(text);
            }
            control.x += control.w;
            control.x += 5;

            control.w = 200.0f;
            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;

            float val = (a->view.z + 500.0f) / 1000.0f;
            val = Imgui::FloatSlider(control, val);
            a->view.z = (val * 1000.0f) - 500.0f;

            if (a->view.z < -500.0f) {
                a->view.z = -500.0f;
            }
            if (a->view.z > 500.0f) {
                a->view.z = 500.0f;
            }
        }
        else if (a->activeTool == ActiveTool::Grid) {
            Imgui::Rect control(header.x, header.y, header.w, header.h);
            control.y += 5;
            control.h -= 14;
            control.x += 5;
            control.w = 67;

            control.y = header.y + 5;
            control.h = header.h - 14;
            Imgui::Label(control, "Show Grid", false, false);
            control.x += 65;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;
            control.w = control.h;
            control.y += 1;
            a->gridShow = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->gridShow, "Show Grid");
            control.y -= 1;
            control.x += control.h + 10;


            control.y = header.y + 5;
            control.h = header.h - 14;
            Imgui::Label(control, "Grid Size", false, false);
            control.x += 60;

            control.y -= 2;
            control.w = 65;
            control.h = header.h - 6.0f;
            stbsp_snprintf(textBuffer, 32, "%.2f", a->gridSize);
            const char* text = Imgui::TextArea(control, textBuffer, 0, !a->gridShow, true, false, false, false, false);
            if (text != textBuffer) {
                f32 val = MathAToF(text);
                a->gridSize = MathAbsF(val);
            }
            control.y += 2;
            control.x += 65 + 10;


            control.y = header.y + 5;
            control.h = header.h - 14;
            Imgui::Label(control, "Outline Selected Node", false, false);
            control.x += 140;

            control.y = header.y + 3.0f;
            control.h = header.h - 6.0f;
            control.w = control.h;
            control.y += 1;
            a->gridHighlight = Imgui::ToggleIcon(control, IMGUI_ICON_CHECKED, IMGUI_ICON_UNCHECKED, a->gridHighlight, "Outline Selected Node");
            control.y -= 1;
        }
    }
}

Node2D* ConvertNode(AnimLoader::Node* source) {
    Node2D* result = (Node2D*)MemAlloc(sizeof(Node2D));
    MemSet(result, 0, sizeof(Node2D));

    result->uid = source->nodeUid;

    char* src = (char*)source->name;
    char* iter = (char*)source->name;
    while (iter != 0 && *iter != 0) {
        ++iter;
    }
    if (result->uid != 0) {
        result->nameLength = iter - src;
        result->name = (char*)MemAlloc(result->nameLength + 1);
        for (u32 i = 0; i < result->nameLength; ++i) {
            result->name[i] = src[i];
        }
        result->name[result->nameLength] = 0;
        result->nameCapacity = result->nameLength;
    }
    else {
        result->nameLength = result->nameCapacity = 0;
        result->name = 0;
    }

    result->parent = (Node2D*)source->parentUid;
    result->firstChild = (Node2D*)source->firstChildUid;
    result->next = (Node2D*)source->nextSiblingUid;
    result->depth = 0;

    result->refCount = 1; // Document will hold onto it

    result->sortIndex = source->sortIndex;
    result->sortedPrev = 0;
    result->sortedNext = 0;

    result->position = vec2(source->positionX, source->positionY);
    result->rotationAngles = source->rotationAngles;
    result->scale = vec2(source->scaleX, source->scaleY);

    result->sprite.sourceX = source->sourceX;
    result->sprite.sourceY = source->sourceY;
    result->sprite.sourceW = source->sourceW;
    result->sprite.sourceH = source->sourceH;
    result->sprite.pivotX = source->pivotX;
    result->sprite.pivotY = source->pivotY;
    result->sprite.tintR = source->tintR;
    result->sprite.tintG = source->tintG;
    result->sprite.tintB = source->tintB;
    result->sprite.tintA = source->tintA;
    result->sprite.visible = source->visible;
    result->sprite.resourceUID = source->resourceUid;

    result->uiVisible = source->visible;
    result->uiExpanded = true;

    return result;
}

void OpenFile(Application* app, void* data, u32 bytes) {
    if (data != 0 && bytes != 0) {
        AnimLoader::Header* kfs = AnimLoader::Parse(data, bytes);

        app->document->nodeUidGenerator = kfs->nodeUidGenerator;
        app->document->animationUidGenerator = kfs->animationUidGenerator;
        app->document->resourceUidGenerator = kfs->resourceUidGenerator;

        app->document->nodeUidGenerator = kfs->nodeUidGenerator;
        app->document->animationUidGenerator = kfs->animationUidGenerator;
        app->document->resourceUidGenerator = kfs->resourceUidGenerator;
        app->document->numNodes = kfs->nodeCount - 1;

        { // Create all nodes
            AnimLoader::Node* loadedRoot = kfs->FirstNode();
            PlatformAssert(app->document->rootNode != 0, __LOCATION__);
            PlatformAssert(app->document->rootNode->firstChild == 0, __LOCATION__);
            PlatformAssert(app->document->rootNode->next == 0, __LOCATION__);
            PlatformAssert(app->document->rootNode->name == 0, __LOCATION__);

            MemRelease(app->document->rootNode);
            Node2D* newRootNode = ConvertNode(loadedRoot);
            app->document->rootNode = newRootNode;

            loadedRoot = loadedRoot->Next();
            for (int i = 1; i < kfs->nodeCount; ++i) {
                Node2D* newNode = ConvertNode(loadedRoot);
                newRootNode->sortedNext = newNode;
                newRootNode = newNode;
                loadedRoot = loadedRoot->Next();
            }
        }

        { // Re-link all nodes
            Node2D* iter = app->document->rootNode;
            for (int i = 0; i < kfs->nodeCount; ++i) {
                // Parent
                if (iter->uid != 0) {
                    u32 parentUid = (u32)iter->parent;
                    iter->parent = 0;

                    Node2D* search = app->document->rootNode;
                    for (int j = 0; j < kfs->nodeCount; ++j) {
                        if (search->uid == parentUid) {
                            iter->parent = search;
                            break;
                        }
                        search = search->sortedNext;
                    }

                }
                else {
                    iter->parent = 0; // Root never has a parent
                }

                // First child
                u32 firstChildUid = (u32)iter->firstChild;
                if (firstChildUid != 0) {
                    Node2D* search = app->document->rootNode;
                    for (int j = 0; j < kfs->nodeCount; ++j) {
                        if (search->uid == firstChildUid) {
                            iter->firstChild = search;
                            break;
                        }
                        search = search->sortedNext;
                    }
                }

                // Next sibling
                u32 nextSiblingUid = (u32)iter->next;
                if (nextSiblingUid != 0) {
                    Node2D* search = app->document->rootNode;
                    for (int j = 0; j < kfs->nodeCount; ++j) {
                        if (search->uid == nextSiblingUid) {
                            iter->next = search;
                            break;
                        }
                        search = search->sortedNext;
                    }
                }

                iter = iter->sortedNext;
            }

            // Clear all sorted next
            Node2D* clearIter = app->document->rootNode;
            for (int i = 0; i < kfs->nodeCount; ++i) {
                Node2D* thisIter = clearIter;
                clearIter = clearIter->sortedNext;
                thisIter->sortedNext = 0;

                thisIter->depth = 0;
                Node2D* parentIter = thisIter;
                while (parentIter != 0) {
                    thisIter->depth += 1;
                    parentIter = parentIter->parent;
                }
            }
        }
        app->document->selectedNode = 0;

        app->document->numResources = kfs->resourceCount;
        { // Populate resources
            AnimLoader::Resource* loadedResource = kfs->FirstResource();
            for (u32 i = 0; i < kfs->resourceCount; ++i) {
                Resource* newResource = (Resource*)MemAlloc(sizeof(Resource));
                MemSet(newResource, 0, sizeof(Resource));

                newResource->nameLen = 0;
                for (char* iter = (char*)loadedResource->name; *iter != 0; ++iter, newResource->nameLen++) {}
                if (newResource->nameLen != 0) {
                    newResource->name = (char*)MemAlloc(newResource->nameLen + 1);
                    char* oldName = (char*)loadedResource->name;
                    for (u32 i = 0; i < newResource->nameLen; ++i) {
                        newResource->name[i] = oldName[i];
                    }
                    newResource->name[newResource->nameLen] = 0;
                }
                else {
                    newResource->name = 0;
                }

                newResource->uid = loadedResource->resourceUid;
                newResource->size = loadedResource->bytes;
                newResource->data = MemAlloc(newResource->size);
                void* src = loadedResource->Data();

                MemCopy(newResource->data, src, newResource->size);
                newResource->filter = Draw2D::Interpolation::Linear;
                newResource->width = loadedResource->width;
                newResource->height = loadedResource->height;
                newResource->image = Draw2D::LoadImage(newResource->data, newResource->size, newResource->filter);

                if (app->document->resources == 0) {
                    app->document->resources = newResource;
                }
                else {
                    Resource* insert = app->document->resources;
                    while (insert->next != 0) {
                        insert = insert->next;
                    }
                    newResource->prev = insert;
                    insert->next = newResource;
                }
                loadedResource = loadedResource->Next();
            }
        }
        app->document->selectedResource = 0;

        app->document->numAnimations = kfs->animationCount;
        PlatformAssert(app->document->allAnimations == 0, __LOCATION__);
        Animation* lastAnim = 0;
        AnimLoader::Animation* loadedAnim = kfs->FirstAnimation();
        for (u32 i = 0; i < kfs->animationCount; ++i) {
            Animation* anim = (Animation*)MemAlloc(sizeof(Animation));
            MemClear(anim, sizeof(Animation));

            anim->uid = loadedAnim->animUid;
            anim->refCount = 1;
            anim->frameCount = loadedAnim->frameCount;
            anim->frameRate = loadedAnim->frameRate;
            anim->loop = loadedAnim->looping ? AnimationLoopMode::Looping : AnimationLoopMode::None;

            anim->name = 0;
            if (loadedAnim->name != 0) {
                char* start = (char*)loadedAnim->name;
                char* end = (char*)loadedAnim->name;
                while (*end != 0) {
                    ++end;
                }
                u32 len = end - start;
                anim->name = (char*)MemAlloc(len + 1);

                for (u32 j = 0; j < len; ++j) {
                    anim->name[j] = start[j];
                }

                anim->name[len] = 0;
            }

            anim->tracks = 0;
            anim->numTracks = loadedAnim->numTracks;
            AnimLoader::Track* loadedTrack = loadedAnim->FirstTrack();
            Track* lastTrack = 0;
            for (u32 j = 0; j < loadedAnim->numTracks; ++j) {
                Track* newTrack = (Track*)MemAlloc(sizeof(Track));
                MemClear(newTrack, sizeof(Track));

                newTrack->avtive = true;
                newTrack->targetNode = loadedTrack->targetNodeUid;
                newTrack->targetProp = (TrackType)loadedTrack->targetProp;

                newTrack->frameCount = loadedAnim->frameCount;
                newTrack->frameCapacity = loadedAnim->frameCount;
                newTrack->numKeyFrames = loadedTrack->numFrames;

                newTrack->frames = (Frame*)MemAlloc(sizeof(Frame) * newTrack->frameCount);
                MemClear(newTrack->frames, sizeof(Frame) * newTrack->frameCount);

                AnimLoader::Frame* loadedFrame = loadedTrack->FirstFrame();
                for (u32 k = 0; k < loadedTrack->numFrames; ++k) {
                    Frame* thisFrame = &newTrack->frames[loadedFrame->index];
                    thisFrame->key = true;
                    thisFrame->interp = (InterpolationType)loadedFrame->interp;
                    thisFrame->uValue = loadedFrame->uValue;

                    loadedFrame = loadedFrame->Next();
                }

                if (anim->tracks == 0) {
                    anim->tracks = newTrack;
                }

                if (lastTrack == 0) {
                    lastTrack = newTrack;
                }
                else {
                    newTrack->prev = lastTrack;
                    lastTrack->next = newTrack;
                    lastTrack = newTrack;
                }
                loadedTrack = loadedTrack->NextInAnimation();
            }

            if (app->document->allAnimations == 0) {
                app->document->allAnimations = anim;
            }
            if (lastAnim == 0) {
                lastAnim = anim;
            }
            else {
                lastAnim->next = anim;
                anim->prev = lastAnim;
                lastAnim = anim;
            }
            loadedAnim = loadedAnim->Next();
        }
        app->document->selectedAnimation = 0;

        app->document->timelineAnimation = 0;
        app->document->selectedTrack = 0;
        app->document->timelineSelectedFrame = -1;
        app->document->timelineLastSelectedFrame = -1;

        if (app->document->allAnimations != 0) {
            app->document->SelectAnimation(app->document->allAnimations);
            app->document->SelectTimeline(app->document->allAnimations);
        }

        MemRelease(data);
    }
}

export void Render(unsigned int x, unsigned int y, unsigned int __w, unsigned int __h, float dpi, void* userData) {
    Application* app = (Application*)userData;
#if _DEBUG
    GfxClearColor(0, 0, 0.5f, 0.6, 0.7f);
#else
    GfxClearColor(0, 0, 0.0f, 0.0, 0.0f);
#endif

    float w = (float)__w / dpi;
    float h = (float)__h / dpi;
    StyleSheet* s = &app->style;

    if (app->numFilesLoading != 0) {
        Draw2D::Begin(1920, 1080, dpi, __w, __h);

        Draw2D::EnableAlphaBlending();
        Draw2D::DrawRect(w / 2 - 200, h / 2 - 100, 400, 100, s->menuBarBg.r, s->menuBarBg.g, s->menuBarBg.b, 1.0f);
        Draw2D::DrawString(app->interfaceFont, 35, w / 2 - 200 + 25, h / 2 - 50 + 10, "Loading, please wait");
        Draw2D::DisableAlphaBlending();

        Draw2D::End();
        return;
    }

    if (app->gridShow) {
        vec4 offset(app->view.x, app->view.y, (app->view.z + 500.0f) / 1000.0f, MathMaxF(app->gridSize, 0.1f));
        vec4 color1(s->gridA.r, s->gridA.g, s->gridA.b, w);
        vec4 color2(s->gridB.r, s->gridB.g, s->gridB.b, h);
        GfxSetUniform(app->gridShader, app->gridOffsetUniform, &offset.x, GfxUniformTypeFloat4, 1);
        GfxSetUniform(app->gridShader, app->gridColor1Uniform, &color1.x, GfxUniformTypeFloat4, 1);
        GfxSetUniform(app->gridShader, app->gridColor2Uniform, &color2.x, GfxUniformTypeFloat4, 1);
        GfxDraw(0, 0, app->gridVao, GfxDrawModeTriangles, 0, 6, 1);
    }

    Draw2D::Begin(1920, 1080, dpi, __w, __h);
    Imgui::BeginFrame(dpi, app->style);
    Draw2D::EnableAlphaBlending();

    Imgui::Rect inspectorArea; // The entire inspector (game + hierarchy + anim + inspector)
    Imgui::Rect inspectorLeft; // Left side only (game / anim)
    Imgui::Rect inspectorRight; // Right side only (inspector / hierarchy)
    Imgui::Rect animationArea; // Split up / down. Game view & anim view
    Imgui::Rect animationTop; // Game view only (top)
    Imgui::Rect animationBottom; // Anim view only (bottom)
    Imgui::Rect menuBar;
    Imgui::Rect hierarchySchrollArea;
    Imgui::Rect hierarchyScrollBar;
    Imgui::Rect hierarchyFooter;

    u32 visibleNodeCount = app->document->GetVisibleNodeCount();
    Imgui::Point mousePos = Imgui::GetPointer();
    const char* title[4] = { 0 };

    { // Menu bar
        menuBar.w = w;
        menuBar.h = s->menuBarHeight;
        //Draw2D::DrawRect(menuBar.x, menuBar.y, menuBar.w, menuBar.h, s->menuBarBg.r, s->menuBarBg.g, s->menuBarBg.b, 1.0f);
        const char* fileMenu[] = {
            "File",
            "Edit",
            "Animation",
            "View",
            "Help"
        };
        Imgui::Rect activator[5] = { 
            Imgui::Rect(10, 5, 28.5, 20),
            Imgui::Rect(48.5, 5, 31, 20),
            Imgui::Rect(89.5, 5, 72, 20),
            Imgui::Rect(171.5, 5, 38, 20),
            Imgui::Rect(219.5, 5, 36, 20),
         };

       // At the end?

        const char* fileOptions[] = {
            "New",
            "Open",
            "Save As",
            "Import Asset",
            "Open Sample Project",
        };

        //if (app->fileMenuOpen) {
            Imgui::Rect popupArea(activator[0].x, menuBar.y, 150, menuBar.h);
            i32 modal = Imgui::BeginModalPopup(popupArea, activator[0], 5);
            for (u32 i = 0; i < 5; ++i) {
                Imgui::PushModalPopupItem(fileOptions[i]);
            }
            if (!Imgui::EndModalPopup()) {
            }

            if (modal == 0) {
                app->NewDocument();
            }
            else if (modal == 1) {
                app->NewDocument();

                RequestFileAsynch([](const char* path, void* data, unsigned int bytes, void* userData) {
                    Application* app = (Application*)userData;
                    OpenFile(app, data, bytes);
                }, app);
            }
            else if (modal == 2) {
                u32 saveBytes = app->document->GetSaveSizeBytes();
                void* saveData = MemAlloc(saveBytes);
                MemSet(saveData, 0, saveBytes);
                saveBytes = app->document->SaveInto(saveData);
                PresentFile(saveData, saveBytes);
                MemRelease(saveData);
            }
            else if (modal == 3) {
                app->document->RequestResource([](const char* path, const Resource& resource, void* userData) {
                    // Nothing to really do here. If the function suceeds, then the resource
                    // is already registered. If the function fails, then nothing.
                    }, app);
            }
            else if (modal == 4) {
                app->numFilesLoading += 1; // Increase so none of the load functions end
                void* loadArena = MemAlloc(1024 * 1024);
                LoadFileAsynch("assets/sample.kfs", loadArena, 1024 * 1024, [](const char* path, void* data, unsigned int bytes, void* userData) {
                    Application* app = (Application*)userData;
                    app->document->NewDocument();
                    OpenFile(app, data, bytes); // Calls MEmRelease(data)
                    app->numFilesLoading -= 1; // Increase so none of the load functions end
                }, app);
            }
        //}

        const char* editOptions[] = {
            "Undo",
            "Redo",
            "Clear Undo History",
            "Create New Node",
            "Delete Selected Node",
        };

        popupArea = Imgui::Rect(activator[1].x, menuBar.y, 160, menuBar.h);
        modal = Imgui::BeginModalPopup(popupArea, activator[1], 5);
        for (u32 i = 0; i < 5; ++i) {
            Imgui::PushModalPopupItem(editOptions[i]);
        }
        if (!Imgui::EndModalPopup()) {
        }

        if (modal == 0) {
            app->document->Undo();
        }
        else if (modal == 1) {
            app->document->Redo();
        }
        else if (modal == 2) {
            app->document->ClearUndoHistory();
        }
        else if (modal == 3) {
            app->document->CreateNode(app->document->rootNode);
        }
        else if (modal == 4) {
            Node2D* n = app->document->GetSelectedNode();
            if (n != 0) {
                app->document->DeleteNode(n);
            }
        }

        const char* animationOptions[] = {
            "Create New Animation",
            "Delete Selected Animation",
            "Set Keyframe",
            "Clear Keyframe",
            "Auto Key",
        };

        //if (app->editMenuOpen) {
        popupArea = Imgui::Rect(activator[2].x, menuBar.y, 190, menuBar.h);
        modal = Imgui::BeginModalPopup(popupArea, activator[2], 5);
        for (u32 i = 0; i < 5; ++i) {
            Imgui::PushModalPopupItem(animationOptions[i]);
        }
        if (!Imgui::EndModalPopup()) {
        }
        //}

        if (modal == 0) {
            app->document->CreateAnimation();
        }
        else if (modal == 1) {
            if (app->document->GetSelectedAnimation() != 0) {
                Animation* oldSelected = app ->document->GetSelectedAnimation();
                if (app->document->GetTimelineAnimation() == oldSelected) {
                    app->document->SelectTimeline(0);
                }
                app->document->SelectAnimation(0);
                app->autoKey = false;
                app->document->DeleteAnimation(oldSelected);
            }
        }
        else if (modal == 2) {
            Track* selectedTrack = app->document->GetSelectedTrack();
            i32 selectedFrame = app->document->GetSelectedFrame();
            if (selectedTrack != 0 && selectedFrame >= 0) {
                app->document->SetFrame(selectedTrack, selectedFrame, app->document->GetSelectedInterpolationType());
            }
        }
        else if (modal == 3) {
            Track* selectedTrack = app->document->GetSelectedTrack();
            i32 selectedFrame = app->document->GetSelectedFrame();
            if (selectedTrack != 0 && selectedFrame >= 0) {
                app->document->ClearFrame(selectedTrack, selectedFrame);
            }
        }
        else if (modal == 4) {
            app->autoKey = !app->autoKey;
        }

        const char* viewOptions[] = {
            "Show Sidebar",
            "Show Footer",
        };

        popupArea = Imgui::Rect(activator[3].x, menuBar.y, 105, menuBar.h);
        modal = Imgui::BeginModalPopup(popupArea, activator[3], 2);
        for (u32 i = 0; i < 2; ++i) {
            Imgui::PushModalPopupItem(viewOptions[i]);
        }
        if (!Imgui::EndModalPopup()) {
        }

        if (modal == 0) {
            app->showSide = !app->showSide;
        }
        else if (modal == 1) {
            app->showBottom = !app->showBottom;
        }

        const char* helpOptions[] = {
            "Github"
        };

        popupArea = Imgui::Rect(activator[4].x, menuBar.y, 100, menuBar.h);
        modal = Imgui::BeginModalPopup(popupArea, activator[4], 1);
        for (u32 i = 0; i < 1; ++i) {
            Imgui::PushModalPopupItem(helpOptions[i]);
        }
        if (!Imgui::EndModalPopup()) {
        }

        if (modal == 0) {
            WindowOpenURL("https://github.com/gszauer/KeyframeStudio");
        }

        app->selectedMenu = Imgui::FileMenu(menuBar, fileMenu, 5, app->selectedMenu, activator);

    }

    { // Inspector Splitter (Game / anim on left, inspector / hierarchy right)
        inspectorArea.x = 0;
        inspectorArea.y = s->menuBarHeight;
        inspectorArea.w = w;
        inspectorArea.h = h - s->menuBarHeight - s->footerHeight;
        
        if (app->showSide) {
            app->inspectorSplit = Imgui::VSplit(inspectorArea, s->inspectorMinWidth, app->inspectorSplit);
            inspectorLeft = Imgui::VSplitFirstArea(inspectorArea, app->inspectorSplit);
            inspectorRight = Imgui::VSplitSecondArea(inspectorArea, app->inspectorSplit);
        }
        else {
            app->inspectorSplit = 1.0f;
            inspectorLeft = inspectorArea;
        }
        //Draw2D::DrawRect(inspectorArea.x, inspectorArea.y, inspectorArea.w, inspectorArea.h, 0, 0, 1);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
        //Draw2D::DrawRect(inspectorLeft.x, inspectorLeft.y, inspectorLeft.w, inspectorLeft.h, 1, 0, 0);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
        //Draw2D::DrawRect(inspectorRight.x, inspectorRight.y, inspectorRight.w, inspectorRight.h, 0, 1, 0);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);

        {
            { // Animation Splitter
                animationArea = inspectorLeft;
                //Draw2D::DrawRect(animationArea.x, animationArea.y + 30, animationArea.w, animationArea.h, 0, 1, 0);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
                if (app->showBottom) {
                    app->animationSplit = Imgui::HSplit(animationArea, s->animatorMinHeight, app->animationSplit);
                    animationTop = Imgui::HSplitFirstArea(animationArea, app->animationSplit);
                    animationBottom = Imgui::HSplitSecondArea(animationArea, app->animationSplit);
                }
                else {
                    app->animationSplit = 1.0f;
                    animationTop = animationArea;
                }

                //Draw2D::DrawRect(animationArea.x, animationArea.y, animationArea.w, animationArea.h, 0, 0, 1);// s->documentBGColor.r, s->documentBGColor.g, s->documentBGColor.b, 1.0f);
                if (app->sceneTab != 0 || !app->gridShow) {
                    Draw2D::DrawRect(animationTop.x, animationTop.y, animationTop.w, animationTop.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b, 1.0f);
                }
                if (app->showBottom) {
                    Draw2D::DrawRect(animationBottom.x, animationBottom.y, animationBottom.w, animationBottom.h, s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b, 1.0f);
                }

                {
                    Imgui::Rect gameHeader = animationTop;
                    gameHeader.h = s->headerHeight;
                    Imgui::Rect gameArea = animationTop;
                    gameArea.y += gameHeader.h;
                    gameArea.h -= gameHeader.h;

                    // Draw header
                    title[0] = "Scene";
                    title[1] = 0;// "Exporter";
                    title[2] = "Memory";

                    if (!app->memVisible) {
                        title[2] = 0;
                    }

                    u32 oldTab = app->sceneTab;
                    u32 newTab = app->sceneTab = Imgui::Header(gameHeader, title, 3, oldTab);

                    if (newTab == 2 && !app->memVisible) {
                        newTab = app->sceneTab = 0;
                    }

                    if (oldTab != newTab) {
                        if (newTab == 2) {
                            app->memoryScroll = 0.0f;
                        }
                    }

                    if (newTab == 0) {
                        ImguiSceneView(gameArea, app);
                    }
                    else if (newTab == 2) {
                        ImguiMemoryViewer(gameArea, app);
                    }
                }

                if (app->showBottom) { // Animation
                    Imgui::Rect animationHeader = animationBottom;
                    animationHeader.h = s->headerHeight;

                    Imgui::Rect animRect = animationBottom;
                    animRect.y += s->headerHeight;
                    animRect.h -= s->headerHeight;

                    if (app->timelineTab == 0) {
                        ImguiAnimationInspector(animRect, app);
                    }
                    else {
                        ImguiKeyboard(animRect, app);
                    }

                    // Draw header
                    title[0] = "Timeline";
                    title[1] = "Keyboard";

                    app->timelineTab = Imgui::Header(animationHeader, title, 2, app->timelineTab);
                }
            }
        }

        if (app->showSide) { // InspectorPanel
            f32 inspectorStack = inspectorArea.y;

            Imgui::Rect detailHeader = inspectorRight;
            detailHeader.h = s->headerHeight;
            detailHeader.y = inspectorStack;

            if (app->hierarchyTab == 0) { // Transform
                inspectorStack += detailHeader.h;

                Imgui::Rect transformEditor = detailHeader;
                transformEditor.y = detailHeader.y + detailHeader.h;

                if (app->transformTab == 0) {
                    transformEditor.h = IMGUI_TRANSFORM_HEIGHT;
                    inspectorStack += IMGUI_TRANSFORM_HEIGHT;
                    ImguiTransformEditor(transformEditor, app);
                }
                else {
                    transformEditor.h = IMGUI_SPRITE_HEIGHT;
                    inspectorStack += IMGUI_SPRITE_HEIGHT;
                    ImguiSpriteEditor(transformEditor, app);
                }

                Node2D* selectedNode = app->document->GetSelectedNode();
                title[0] = "Transform"; // Header
                title[1] = "Sprite"; // Header

                app->transformTab = Imgui::Header(detailHeader, title, 2, app->transformTab);
            }
            else if (app->hierarchyTab == 1) { // Assets
                inspectorStack += detailHeader.h;

                Imgui::Rect animEditor = detailHeader;
                animEditor.y = animEditor.y + animEditor.h;

                animEditor.h = IMGUI_PREVIEW_HEIGHT;
                inspectorStack += IMGUI_PREVIEW_HEIGHT;

                StyleSheet* s = &app->style;
                Draw2D::DrawRect(animEditor.x, animEditor.y, animEditor.w, animEditor.h,
                    s->panelBgColor.r, s->panelBgColor.g, s->panelBgColor.b);

                animEditor.w = IMGUI_PREVIEW_HEIGHT;
                Resource* r = app->document->GetSelectedResource();

                Imgui::Rect imageArea(animEditor.x + 5, animEditor.y + 5, animEditor.w - 10, animEditor.h - 10);
                Imgui::Rect sourceRect;
                if (r != 0) {
                    sourceRect.w = r->width;
                    sourceRect.h = r->height;
                    Imgui::ImageBlock(imageArea, 0, r->image, sourceRect, r == 0);
                }
                else {
                    Imgui::ImageBlock(imageArea, 0, 0, sourceRect, r == 0);
                }

                animEditor.x += IMGUI_PREVIEW_HEIGHT;
                animEditor.x += 2;
                animEditor.y += 7;
                animEditor.h = 14;

                if (r != 0) {
                    char printBuff[64] = { 0 };
                    stbsp_snprintf(printBuff, 64, "Name: %s", r->name);
                    Imgui::Label(animEditor, printBuff, false);
                    animEditor.y += 20;
                    stbsp_snprintf(printBuff, 64, "Width: %d", r->width);
                    Imgui::Label(animEditor, printBuff, false);
                    animEditor.y += 20;
                    stbsp_snprintf(printBuff, 64, "Height: %d", r->height);
                    Imgui::Label(animEditor, printBuff, false);
                    animEditor.y += 20;
                    stbsp_snprintf(printBuff, 64, "Bytes: %d", r->size);
                    Imgui::Label(animEditor, printBuff, false);
                    animEditor.y += 20;
                    stbsp_snprintf(printBuff, 64, "References: %d", app->document->GetReferenceCount(r));
                    Imgui::Label(animEditor, printBuff, false);
                }
                else {
                    Imgui::Label(animEditor, "Select image\nto preview", false);
                }

                title[0] = "Preview";
                Imgui::Header(detailHeader, title, 1, 0);
            }
            else if (app->hierarchyTab == 2) { // Anims
                inspectorStack += detailHeader.h;

                Imgui::Rect animEditor = detailHeader;
                animEditor.y = animEditor.y + animEditor.h;

                animEditor.h = IMGUI_ANIMDETAIL_HEIGHT;
                inspectorStack += IMGUI_ANIMDETAIL_HEIGHT;

                ImguiAnimationEditor(animEditor, app);
                title[0] = "Animation";
                Imgui::Header(detailHeader, title, 1, 0);
            }

            { // Hierarchy and undo stack
                Imgui::Rect hierarchyHeader = inspectorRight;
                hierarchyHeader.h = s->headerHeight;
                hierarchyHeader.y = inspectorStack;

                f32 hiearchyHeight = inspectorRight.h - s->footerHeight - s->headerHeight - (inspectorStack - inspectorArea.y) - 2;
                if (hiearchyHeight < 0.0f) {
                    hiearchyHeight = 0.0f;
                }

                hierarchySchrollArea = hierarchyHeader;
                hierarchySchrollArea.y = hierarchyHeader.y + hierarchyHeader.h;
                hierarchySchrollArea.w = inspectorRight.w - s->scrollBarSize;
                hierarchySchrollArea.h = hiearchyHeight;

                if (hierarchySchrollArea.w < 0.0f) {
                    hierarchySchrollArea = 0.0f;
                }

                if (app->hierarchyTab == 0) {
                    ImguiHierarchy(hierarchySchrollArea, app);
                }
                else if (app->hierarchyTab == 1) {
                    ImguiResources(hierarchySchrollArea, app);
                }
                else if (app->hierarchyTab == 2) {
                    ImguiAnimations(hierarchySchrollArea, app);
                }
                else if (app->hierarchyTab == 3) {
                    ImguiUndoStack(hierarchySchrollArea, app);
                }

                // Draw header
                title[0] = "Nodes";
                title[1] = "Images";
                title[2] = "Animations";
                title[3] = "Undo";

                u32 oldTab = app->hierarchyTab;
                app->hierarchyTab = Imgui::Header(hierarchyHeader, title, 4, app->hierarchyTab);
                if (app->hierarchyTab != oldTab) {
                    app->hierarchyScroll = 0.0f;
                    Imgui::ClearActiveTextAreas();
                }
            }
        }
    }


    { // Draw the footer
        Draw2D::DrawRect(0, h - s->footerHeight, w, s->footerHeight, s->footerBg.r, s->footerBg.g, s->footerBg.b, 1.0f);

        float colorA[3] = {
                s->dividerAColor.r, s->dividerAColor.g, s->dividerAColor.b
        };
        float colorB[3] = {
            s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b
        };

        Draw2D::DrawRect(0, h - s->footerHeight, w, 2, colorA[0], colorA[1], colorA[2], 1.0f);
        Draw2D::DrawRect(0, h - s->footerHeight, w, 1, colorB[0], colorB[1], colorB[2], 1.0f);


#define FPS_LABEL_WIDTH 45.0f
#define MEM_LABEL_WIDTH_2 61.0f
#define MEM_LABEL_WIDTH_1 54.0f
#define FOOTERBAR_PADDING 5.0f
#define FPS_GRAPH_WIDTH 31.0f
#define MEM_BTN_WIDTH 50.0f


        vec2 footerKarrat(w, h - s->footerHeight + 1);
        footerKarrat.x -= FOOTERBAR_PADDING / 2;

        footerKarrat.x -= FOOTERBAR_PADDING;
        footerKarrat.x -= s->footerHeight - 6;
        Imgui::Rect mem_rect(footerKarrat.x, footerKarrat.y + 3, s->footerHeight - 6, s->footerHeight - 6);

        u32 activgePages = MemGetCurrentNumPages();
        u32 numPages = MemGetHeapSize() / MemGetPageSize();
        float t = (float)activgePages / (float)numPages + 0.005f;
        u32 p = 100.0f * t;
#define ANIMATOR_MEM_BUFFER_SIZE 32
        char mem_buffer[ANIMATOR_MEM_BUFFER_SIZE] = { 0 };

        u32 activeBytes = activgePages * 4049;
        int written = 0;
        if (activeBytes > 1024 * 1024 * 1024) {
            float gib = (float)activeBytes / 1024.0f / 1024.0f / 1024.0f;
            written = stbsp_snprintf(mem_buffer, ANIMATOR_MEM_BUFFER_SIZE, "%.1fGB / ", gib);
        }
        else if (activeBytes > 1024 * 1024) {
            float mib = (float)activeBytes / 1024.0f / 1024.0f;
            written = stbsp_snprintf(mem_buffer, ANIMATOR_MEM_BUFFER_SIZE, "%.1fMB / ", mib);
        }
        else if (activeBytes > 1024) {
            float kib = (float)activeBytes / 1024.0f;
            written = stbsp_snprintf(mem_buffer, ANIMATOR_MEM_BUFFER_SIZE, "%.1fKB / ", kib);
        }
        else {
            written = stbsp_snprintf(mem_buffer, ANIMATOR_MEM_BUFFER_SIZE, "%dB / ", activeBytes);
        }

        char* iter = mem_buffer + written;

        u32 totalBytes = MemGetHeapSize();
        if (totalBytes > 1024 * 1024 * 1024) {
            float gib = (float)totalBytes / 1024.0f / 1024.0f / 1024.0f;
            stbsp_snprintf(iter, ANIMATOR_MEM_BUFFER_SIZE - written, "%.1fGB", gib);
        }
        else if (totalBytes > 1024 * 1024) {
            float mib = (float)totalBytes / 1024.0f / 1024.0f;
            stbsp_snprintf(iter, ANIMATOR_MEM_BUFFER_SIZE - written, "%.1fMB", mib);
        }
        else if (totalBytes > 1024) {
            float kib = (float)totalBytes / 1024.0f;
            stbsp_snprintf(iter, ANIMATOR_MEM_BUFFER_SIZE - written, "%.1fKB", kib);
        }
        else {
            stbsp_snprintf(iter, ANIMATOR_MEM_BUFFER_SIZE - written, "%dB", totalBytes);
        }

        StyleColor tint = { 0 };
        app->memVisible = Imgui::ToggleButton(mem_rect, '0' + (p / 10), '0' + (p / 10), app->memVisible, false, mem_buffer, tint);

        footerKarrat.x -= FOOTERBAR_PADDING / 3;
        if (p < 10) {
            footerKarrat.x -= MEM_LABEL_WIDTH_1;
        }
        else {
            footerKarrat.x -= MEM_LABEL_WIDTH_2;
        }

#define ANIMATOR_MEM_BUFFER2_SIZE 32
        char memBuffer[ANIMATOR_MEM_BUFFER2_SIZE] = { 0 };
        stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER2_SIZE, "MEM: %d%c", p, '%');

        Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
            s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);

        footerKarrat.x -= FOOTERBAR_PADDING;
        footerKarrat.x -= FOOTERBAR_PADDING;
        footerKarrat.x -= FPS_GRAPH_WIDTH;
        float graph_height = s->footerHeight - 10;
        Draw2D::DrawRect(footerKarrat.x, footerKarrat.y + 5, FPS_GRAPH_WIDTH, graph_height,
            s->textAreaOutline_Normal.r, s->textAreaOutline_Normal.g, s->textAreaOutline_Normal.b);
        Draw2D::DrawRect(footerKarrat.x + 1, footerKarrat.y + 5 + 1, FPS_GRAPH_WIDTH - 2, graph_height - 2,
            s->textAreaBg_Normal.r, s->textAreaBg_Normal.g, s->textAreaBg_Normal.b);

        double avgDt = 0.0;
        for (i32 i = 0; i < 60; ++i) {
            avgDt += app->msPerFrame[i];
        }
        if (avgDt > 0.0) {
            avgDt /= 60.0;
        }

        if (app->msIter == 0) {
            app->fpsDisplay = (i32)(1000.0 / (avgDt * 1000.0));
        }

        float lastSample = 1000.0f / (app->msPerFrame[0] * 1000.0f);
        float lastSampleNormalized = Math01(lastSample / 60.0f);

        StyleColor color = s->fps1;
        if (app->fpsDisplay < 28) {
            color = s->fps2;
        }
        else if (app->fpsDisplay < 18) {
            color = s->fps3;
        }

        f32 points[30 * 2] = { 0 };
        for (int i = 0; i < 30; ++i) {
            float thisSample = 1000.0f / (app->msPerFrame[i * 2] * 1000.0f);
            float thisNormalized = Math01(thisSample / 60.0f);
            points[i * 2 + 0] = footerKarrat.x + 1 + i;
            points[i * 2 + 1] = footerKarrat.y + 1 + 5 + 4 + (1.0f - thisNormalized) * (graph_height - 8);
        }
        Draw2D::DrawLine(points, 30, 1, color.r, color.g, color.b);


        footerKarrat.x -= FOOTERBAR_PADDING;
        footerKarrat.x -= FPS_LABEL_WIDTH;

        stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER2_SIZE, "FPS: %d", app->fpsDisplay);
        Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
            s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);


        stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER2_SIZE, "Index count: %d", GfxStatsIndexCount());
        Draw2D::Size size = Draw2D::MeasureString(app->interfaceFont, 12, memBuffer);
        footerKarrat.x -= FOOTERBAR_PADDING;
        footerKarrat.x -= size.w + 2;
        Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
            s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);

        stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER2_SIZE, "Draw calls: %d", GfxStatsDrawCount());
        size = Draw2D::MeasureString(app->interfaceFont, 12, memBuffer);
        footerKarrat.x -= FOOTERBAR_PADDING;
        footerKarrat.x -= size.w + 2;
        Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
            s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);

        footerKarrat.x = FOOTERBAR_PADDING;

        Animation* timelineAnimation = app->document->GetTimelineAnimation();
        if (timelineAnimation == 0) {
            stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER_SIZE, "Duration: 0");
            Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
                s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);
            Draw2D::Size screenSize = Draw2D::MeasureString(app->interfaceFont, 12, memBuffer);
            footerKarrat.x += screenSize.w;
            footerKarrat.x += FOOTERBAR_PADDING;
            footerKarrat.x += FOOTERBAR_PADDING;

            stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER_SIZE, "Frame: 0/0");
            Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
                s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);
        }
        else {
            float seconds = (float)timelineAnimation->frameCount / (float)timelineAnimation->frameRate;
            stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER_SIZE, "Duration: %.1fs", seconds);
            Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
                s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);
            Draw2D::Size screenSize = Draw2D::MeasureString(app->interfaceFont, 12, memBuffer);
            footerKarrat.x += screenSize.w;

            footerKarrat.x += FOOTERBAR_PADDING;
            footerKarrat.x += FOOTERBAR_PADDING;
            {
                stbsp_snprintf(memBuffer, ANIMATOR_MEM_BUFFER_SIZE, "Frame: %d/%d", (app->document->GetSelectedFrame() + 1), (timelineAnimation->frameCount));

            }
            Draw2D::DrawString(app->interfaceFont, 12, footerKarrat.x, footerKarrat.y + 12 + 6, memBuffer,
                s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);
        }
    }

    app->kbdGen = 0;
    if (Imgui::PointerReleased()) {
        app->kbdActive = 0;
    }
    app->kbdHot = 0;
    app->kbdHandled = false;

    bool processedKbd = Imgui::EndFrame(__w, __h);
    Draw2D::DisableAlphaBlending();
    Draw2D::End();

    if (!processedKbd) {
        if (KeyboardPressed(KeyboardCodeQ)) {
            app->activeTool = ActiveTool::None;
        }
        else if (KeyboardPressed(KeyboardCodeW)) {
            app->activeTool = ActiveTool::Move;
        }
        else if (KeyboardPressed(KeyboardCodeE)) {
            app->activeTool = ActiveTool::Rotate;
        }
        else if (KeyboardPressed(KeyboardCodeR)) {
            app->activeTool = ActiveTool::Scale;
        }
        else if (KeyboardPressed(KeyboardCodeS)) {
            if (KeyboardDown(KeyboardCodeControl)) {
                u32 saveBytes = app->document->GetSaveSizeBytes();
                void* saveData = MemAlloc(saveBytes);
                MemSet(saveData, 0, saveBytes);
                saveBytes = app->document->SaveInto(saveData);
                PresentFile(saveData, saveBytes);
                MemRelease(saveData);
            }
            else {
                Animation* timeline = app->document->GetTimelineAnimation();
                if (timeline != 0) {
                    Track* track = app->document->GetSelectedTrack();
                    if (track != 0) {
                        i32 frame = app->document->GetSelectedFrame();
                        if (frame > 0) {
                            app->document->SetFrame(track, frame, app->document->GetSelectedInterpolationType());
                        }
                    }
                }
            }
        }
        else if (KeyboardPressed(KeyboardCodeZ)) {
            if (KeyboardDown(KeyboardCodeControl)) {
                app->document->Undo();
            }
        }
        else if (KeyboardPressed(KeyboardCodeY)) {
            if (KeyboardDown(KeyboardCodeControl)) {
                app->document->Redo();
            }
        }
        else if (KeyboardPressed(KeyboardCodeSpace)) {
            app->playSelectedAnimation = !app->playSelectedAnimation;
            // a->playingAnimationFrame
            //a->playingAnimationTimer
        }    
    }        
    app->document->EndOfFrame(app->autoKey); // Updates node animations
}

export void Shutdown(void* userData) {
    Application* app = (Application*)userData;
    app->document->~Document();

    GfxDestroyShaderVertexLayout(app->gridVao);
    GfxDestroyBuffer(app->gridVbo);
    GfxDestroyBuffer(app->gridIbo);
    GfxDestroyShader(app->gridShader);

    Imgui::Shutdown();
    Draw2D::Shutdown();

    MemRelease(app->document);
    MemRelease(app);
}