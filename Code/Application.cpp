#include "Application.h"
#include "Libraries/imgui.h"
#include "Libraries/imgui_internal.h"
#include "Libraries/imgui_stdlib.h"
#include "Libraries/imgui_neo_sequencer.h"
#include "Libraries/gabor_json.h"
#include "Libraries/base64.h"
#include "Libraries/sokol_imgui.h"
#include "UndoManager.h"
#include "Embedded.h"
#include "Platform.h"

#include <iostream>
#include <algorithm>

Application* Application::instance = 0;

#define FOOTER_BUTTON_PADDING 4.0f
#define FOOTER_BUTTON_WIDTH 20.0f
#define FOOTER_BUTTON_SIZE ImVec2(20, 20)

Application* Application::GetInstance() {
    if (Application::instance == 0) {
        Application::instance = new Application();
    }
    return Application::instance;
}

void Application::DestroyInstance() {
    AssetManager::DestroyAll();
    SceneNode::DestroyAll();
    if (Application::instance != 0) {
        delete Application::instance;
    }
    Application::instance = 0;
}

Application::Application() {
    argon = icon = xenon = 0;
    newThingName = "";
    newAnimFrameRate = 30;
    newAnimFrameCount = 60;
    newAnimLooping = false;
    deleteAtTheEndOfFrame = 0;
    resetDockSpace = true;
	showHierarchy = true;
	showAssets = true;
	showInspector = true;
	showSequencer = true;
    showHistory = true;
    isPlaying = false;
    focusNode = true;
    activeTool = 0;
    lastActiveTool = 0;
    rootNode = 0;
	dragAndDrop.Reset();
	selectedNode = 0;
    selectedAsset = 0;
	selectedAnimation = 0;
	selectedFrame = 0;

    leftClicked = false;
    middleClicked = false;
    rightClicked = false;

    playbackTime = 0.0f;

    createNewSceneAfterFrame = false;
    saveSceneAs = false;
    openScene = false;

    guiLoadAtlasFromFile = false;
    guiAtlasFileData = 0;
    guiAtlasFileSize = 0;

    rotationDisplay = RotationDisplayType::RADIAN_SLIDER;
    
    // Default DPI is one. For High DPI support, call initialize with dpi
    //float dpi = GetDpiForWindow(GetActiveWindow());
    windowDevicePixelRatio = 1.0f;// dpi / 96.f; // 96 is the "standard dpi"
}

Application::~Application() {
    // No need to clean up root node. All nodes are static free'd when the app exits.
}

void Application::SetSelectedNode(SceneNode* node, bool focus) {
    selectedNode = node;
    focusNode = focus;
}

void Application::SetSelectedAsset(Asset* node, bool focus, AnimationAsset* selectedAnim, int frame) {
    selectedAsset = node;
    focusNode = focus;
    selectedAnimation = selectedAnim;
    selectedFrame = frame;
}

void Application::FillWithDebugData() {
    //JsonValue* parsed = JsonParseString("{\"obj\":{\"foo\":7,\"bar\":true}}", 0);

    (new SceneNode("Scene Node"))->SetParent(rootNode);
    
    TransformNode* xFormNod = new TransformNode("Transform Node");
    xFormNod->SetParent(rootNode);
    xFormNod->SetRotation(2.375);

    SpriteNode* sprtNode = new SpriteNode("Sprite Node");
    sprtNode->SetParent(xFormNod);
    sprtNode->SetRotation(1.571f);

    selectedNode = new SpriteNode("Sprite Node");
    selectedNode->SetParent(rootNode);

    SceneNode* foo = new SceneNode("Foo");
    foo->SetParent(rootNode);

    SceneNode* bar = new SceneNode("Bar");
    bar->SetParent(rootNode);

    {
        (new SceneNode("Man"))->SetParent(bar);
        (new SceneNode("Woman"))->SetParent(bar);
        (new SceneNode("TV"))->SetParent(bar);

        (new SceneNode("Apple"))->SetParent(foo);
        (new SceneNode("Pine Apple"))->SetParent(foo);
        (new SceneNode("Appl"))->SetParent(foo);
    }


    AnimationAsset* newAnim = AssetManager::GetInstance()->NewAnimation("Running", 24, 60, true);
    if (false) {
        Track* t = newAnim->AddTrack(selectedNode, "Position.x", TrackType::TRACK_FLOAT);
        t->AddFrameF(0.5f, 2);
        t->AddFrameF(0.5f, 4);
        t->AddFrameF(0.5f, 10);

        t = newAnim->AddTrack(selectedNode, "Position.x", TrackType::TRACK_FLOAT);
        t->AddFrameF(0.5f, 12);
        t->AddFrameF(0.5f, 14);
        t->AddFrameF(0.5f, 20);

        t = newAnim->AddTrack(selectedNode, "Position.y", TrackType::TRACK_FLOAT);
        t->AddFrameI(5, 10);
        t->AddFrameI(5, 20);
        t->AddFrameI(5, 30);

        t = newAnim->AddTrack(selectedNode, "Rotation", TrackType::TRACK_FLOAT);
        t->AddFrameB(false, 30);
        t->AddFrameB(false, 40);
        t->AddFrameB(false, 50);
        t->AddFrameB(false, 60);

        t = newAnim->AddTrack(foo, "Position.x", TrackType::TRACK_FLOAT);
        t->AddFrameF(0.5f, 2);
        t->AddFrameF(0.5f, 4);
        t->AddFrameF(0.5f, 10);

        t = newAnim->AddTrack(foo, "Position.x", TrackType::TRACK_FLOAT);
        t->AddFrameF(0.5f, 12);
        t->AddFrameF(0.5f, 14);
        t->AddFrameF(0.5f, 20);

        t = newAnim->AddTrack(foo, "Position.y", TrackType::TRACK_FLOAT);
        t->AddFrameI(5, 10);
        t->AddFrameI(5, 20);
        t->AddFrameI(5, 30);

        t = newAnim->AddTrack(foo, "Rotation", TrackType::TRACK_FLOAT);
        t->AddFrameB(true, 30);
        t->AddFrameB(true, 40);
        t->AddFrameB(true, 50);
        t->AddFrameB(true, 60);

        t = newAnim->AddTrack(bar, "Position.x", TrackType::TRACK_FLOAT);
        t->AddFrameF(0.5f, 2);
        t->AddFrameF(0.5f, 4);
        t->AddFrameF(0.5f, 10);

        t = newAnim->AddTrack(bar, "Position.x", TrackType::TRACK_FLOAT);
        t->AddFrameF(0.5f, 12);
        t->AddFrameF(0.5f, 14);
        t->AddFrameF(0.5f, 20);

        t = newAnim->AddTrack(bar, "Position.y", TrackType::TRACK_FLOAT);
        t->AddFrameI(5, 10);
        t->AddFrameI(5, 20);
        t->AddFrameI(5, 30);

        t = newAnim->AddTrack(bar, "Rotation", TrackType::TRACK_FLOAT);
        t->AddFrameB(false, 30);
        t->AddFrameB(false, 40);
        t->AddFrameB(false, 50);
        t->AddFrameB(false, 60);
    }

    selectedAnimation = 0;
    selectedNode = 0;
}

void Application::SkinImGui(float dpi) {
    // Discord (Dark) style by BttrDrgn from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();
    //style.PopupBorderSize = IM_COL32(50, 50, 200, 255);
    
    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6000000238418579f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
   
    style.TabBarOverlineSize = 0.0f;
    //style.TabBorderSize = 32.0f;
    //style.TabBarBorderSize = 32.0f;

    defaultFramePadding = style.FramePadding;
    defaultWindowPadding = style.WindowPadding;
    style.FramePadding = ImVec2(3, 6);

    style.Colors[ImGuiCol_TabSelectedOverline] = ImVec4(35.0f / 255.0f, 65.0f / 255.0f, 120.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2117647081613541f, 0.2235294133424759f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(35.0f / 255.0f, 37.0f / 255.0f, 43.0f / 255.0f, 1.0f); //ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f); ;// ImVec4(15.0f / 255.0f, 25.0f / 255.0f, 35.0f / 255.0f, 1.0f);// ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] =  ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
    style.Colors[ImGuiCol_ScrollbarGrab] =        ImVec4( 65.0f / 255.0f,  65.0f / 255.0f,  75.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 95.0f / 255.0f,  95.0f / 255.0f, 105.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] =  ImVec4(120.0f / 255.0f, 120.0f / 255.0f, 130.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(210.0f / 255.0f, 211.0f / 255.0f, 215.0f / 255.0f, 1.0f);//ImVec4(60.0f / 255.0f, 106.0f / 255.0f, 197.0f / 255.0f, 1.0f);// ImVec4(0.2313725501298904f, 0.6470588445663452f, 0.364705890417099f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(120.0f / 255.0f, 120.0f / 255.0f, 130.0f / 255.0f, 1.0f);// ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(150.0f / 255.0f, 151.0f / 255.0f, 160.0f / 255.0f, 1.0f);// ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.407843142747879f, 0.4274509847164154f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.407843142747879f, 0.4274509847164154f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.407843142747879f, 0.4274509847164154f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
    
    style.Colors[ImGuiCol_Tab] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2705882489681244f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2588235437870026f, 0.2745098173618317f, 0.3019607961177826f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2588235437870026f, 0.2745098173618317f, 0.3019607961177826f, 1.0f); // ImVec4(1, 0, 0, 1);//ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] =  ImVec4(48.0f / 255.0f, 51.0f / 255.0f, 58.0f / 255.0f, 1.0f);//ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);

    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.3607843220233917f, 0.4000000059604645f, 0.4274509847164154f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.05098039284348488f, 0.4196078479290009f, 0.8588235378265381f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);

    //style.Colors[ImGuiCol_DockingPreview] = ImVec4(1, 0, 0, 1); // Docking preview color
    //style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0, 1, 0, 1); // Empty docking space color

    ImGuiNeoSequencerStyle& nStyle = ImGui::GetNeoSequencerStyle();
    nStyle.Colors[ImGuiNeoSequencerCol_Bg] = style.Colors[ImGuiCol_WindowBg];
    //nStyle.Colors[ImGuiNeoSequencerCol_TimelinesBg] = ImVec4(0, 1, 1, 1);

    nStyle.Colors[ImGuiNeoSequencerCol_ZoomBarSlider] = style.Colors[ImGuiCol_ScrollbarGrab];
    nStyle.Colors[ImGuiNeoSequencerCol_ZoomBarSliderHovered] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
    nStyle.Colors[ImGuiNeoSequencerCol_ZoomBarSliderEnds] = style.Colors[ImGuiCol_ScrollbarGrab];
    nStyle.Colors[ImGuiNeoSequencerCol_ZoomBarSliderEndsHovered] = style.Colors[ImGuiCol_ScrollbarGrabHovered];
    nStyle.Colors[ImGuiNeoSequencerCol_ZoomBarBg] = style.Colors[ImGuiCol_ScrollbarBg];
    nStyle.Colors[ImGuiNeoSequencerCol_TopBarBg] = style.Colors[ImGuiCol_FrameBg];
    nStyle.Colors[ImGuiNeoSequencerCol_FramePointer] = ImVec4(125.0f / 255.0f, 125.0f / 255.0f, 205.0f / 255.0f, 1.0f);
    nStyle.Colors[ImGuiNeoSequencerCol_FramePointerHovered] = ImVec4(145.0f / 255.0f, 145.0f / 255.0f, 225.0f / 255.0f, 1.0f);
    nStyle.Colors[ImGuiNeoSequencerCol_FramePointerPressed] = ImVec4(105.0f / 255.0f, 105.0f / 255.0f, 185.0f / 255.0f, 1.0f);
    nStyle.Colors[ImGuiNeoSequencerCol_Keyframe] = nStyle.Colors[ImGuiNeoSequencerCol_TopBarBg];
    nStyle.Colors[ImGuiNeoSequencerCol_KeyframeSelected] = nStyle.Colors[ImGuiNeoSequencerCol_FramePointer];
    nStyle.Colors[ImGuiNeoSequencerCol_KeyframeHovered] = nStyle.Colors[ImGuiNeoSequencerCol_FramePointerHovered];
    nStyle.Colors[ImGuiNeoSequencerCol_KeyframePressed] = nStyle.Colors[ImGuiNeoSequencerCol_FramePointerPressed];
    //nStyle.Colors[ImGuiNeoSequencerCol_Selection] = nStyle.Colors[ImGuiNeoSequencerCol_TopBarBg];
    nStyle.Colors[ImGuiNeoSequencerCol_SelectedTimeline] = style.Colors[ImGuiNeoSequencerCol_ZoomBarBg];
    
    
    nStyle.Colors[ImGuiNeoSequencerCol_TimelinesBg] = nStyle.Colors[ImGuiNeoSequencerCol_TopBarBg];

    //ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 60, 160, 255));

    // Need to call ImGui_ImplWin32_EnableDpiAwareness(); at startup
   // https://pthom.github.io/imgui_bundle/faq.html#_high_dpi_with_dear_imgui
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false; // Prevent Dear ImGui from freeing the font data.


    xenon = io.Fonts->AddFontFromMemoryTTF((void*)xenon_data, xenon_size, 16.0f * windowDevicePixelRatio, &fontConfig);
    // AddFontFromFileTTF("xenon.ttf", 16.0f * windowDevicePixelRatio);
    argon = io.Fonts->AddFontFromMemoryTTF((void*)argon_data, argon_size, 14.0f * windowDevicePixelRatio, &fontConfig);
    //AddFontFromFileTTF("argon.ttf", 14.0f * windowDevicePixelRatio);
    icon = io.Fonts->AddFontFromMemoryTTF((void*)icon_data, icon_size, 22.0f * windowDevicePixelRatio, &fontConfig);
    //AddFontFromFileTTF("icon.ttf", 22.0f * windowDevicePixelRatio);
    
    //io.Fonts->Build();
    simgui_font_tex_desc_t font_texture_desc = { };
    font_texture_desc.min_filter = SG_FILTER_LINEAR;
    font_texture_desc.mag_filter = SG_FILTER_LINEAR;
    simgui_create_fonts_texture(&font_texture_desc);

    nStyle.ZoomHeightScale = 1.3f;
    nStyle.CurrentFramePointerSize *= windowDevicePixelRatio;
    nStyle.TopBarHeight *= windowDevicePixelRatio;
    nStyle.ItemSpacing = nStyle.ItemSpacing * windowDevicePixelRatio;
    nStyle.DepthItemSpacing = 25 * windowDevicePixelRatio;
    nStyle.TopBarSpacing *= windowDevicePixelRatio;
    nStyle.TimelineBorderSize *= windowDevicePixelRatio;
    nStyle.CurrentFrameLineWidth *= windowDevicePixelRatio;
    nStyle.CollidedKeyframeOffset *= windowDevicePixelRatio;
    nStyle.MaxSizePerTick *= windowDevicePixelRatio;
    style.ScaleAllSizes(windowDevicePixelRatio);
}

void Application::ResetDockSpace() {
    resetDockSpace = false;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiID dockspace_id = ImGui::GetID("MainWindowImguiDockSpace");

    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    auto dock_split_1 = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.20f, nullptr, &dockspace_id);
    auto dock_split_0 = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);
    auto dock_split_2 = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
    auto dock_split_3 = ImGui::DockBuilderSplitNode(dock_split_1, ImGuiDir_Down, 0.5f, nullptr, &dock_split_1);

    ImGui::DockBuilderDockWindow("Scene", dockspace_id);
    ImGui::DockBuilderDockWindow("Sequencer", dock_split_0);
    //ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_split_0);
    ImGui::DockBuilderDockWindow("Hierarchy", dock_split_1);
    ImGui::DockBuilderDockWindow("Inspector", dock_split_2);
    ImGui::DockBuilderDockWindow("Assets", dock_split_3);
    ImGui::DockBuilderDockWindow("History", dock_split_1);

    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
    node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

    ImGui::DockBuilderFinish(dockspace_id);
}

void Application::ImguiHierarchy() {
    if (!showHierarchy) {
        return;
    }

    static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Hierarchy");// , 0, ImGuiWindowFlags_NoCollapse);
    ImGui::PushFont(argon);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);

    if (rootNode != 0) {
        ImVec2 windowRegion = ImGui::GetContentRegionAvail();
        windowRegion.y -= (28.0f * windowDevicePixelRatio);

        { // Actual scroll area
            bool dragDropActive = DoingDragDrop();

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None | ImGuiWindowFlags_AlwaysVerticalScrollbar;

            ImGui::BeginChild("HierarchyScrollArea", windowRegion, ImGuiChildFlags_None, window_flags);
           
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f * windowDevicePixelRatio, 2.0f * windowDevicePixelRatio));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4.0f * windowDevicePixelRatio));
            
            ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0, 1.0f * windowDevicePixelRatio));
            if (dragDropActive) {
                ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(93, 95, 103, 255));   // Active (clicked) color
                ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 4.0f * windowDevicePixelRatio);
            }
            else {
                ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 1.0f * windowDevicePixelRatio);
                ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(73, 75, 83, 255));   // Active (clicked) color
            }

            bool oldFocusNode = focusNode;
            bool newFocusNode = oldFocusNode;
            SceneNode* oldSelectedNode = selectedNode;
            SceneNode* newSelectedNode = rootNode->EditorImguiHierarchy(selectedNode, &dragAndDrop, &filter, &newFocusNode, dragDropActive);

            if (oldFocusNode != newFocusNode || oldSelectedNode != newSelectedNode) {
                UndoManager::GetInstance()->SelectNode(oldSelectedNode, newSelectedNode, oldFocusNode, newFocusNode); // Will set: selectedNode, focusNode
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                SceneNode::clicked = 0;
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::EndChild();
        }

        float rightSide = 0.0f;
        float filterEnd;
        { //  Footer
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            float footerWidth = windowRegion.x;

            SceneNode* parent = rootNode;
            if (selectedNode != 0 && selectedNode->CanHaveChildren()) {
                parent = selectedNode;
            }
            
            rightSide = ImGui::GetCursorPosX() + footerWidth;// -FOOTER_BUTTON_PADDING * windowDevicePixelRatio;
            ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 4.0f * windowDevicePixelRatio);
            ImGui::PushFont(icon); 
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            if (ImGui::Button("D", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
                //newNode = new TransformNode("Transform Node");
                UndoManager::GetInstance()->CreateTransformNode(parent);
            }
            ImGui::PopStyleVar();
            ImGui::PopFont();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
            ImGui::SetItemTooltip("New Transform Node");
            ImGui::PopStyleVar();
            ImGui::SameLine();

            ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 3.0f * windowDevicePixelRatio);
            ImGui::PushFont(icon);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); 
            if (ImGui::Button("C", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
                //newNode = new SpriteNode("Sprite Node");
                UndoManager::GetInstance()->CreateSpriteNode(parent);
            }
            ImGui::PopStyleVar();
            ImGui::PopFont();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
            ImGui::SetItemTooltip("New Sprite Node");
            ImGui::PopStyleVar();

            {
                bool disabled = selectedNode == 0;
                if (disabled) {
                    ImGui::BeginDisabled();
                }
                ImGui::SameLine();

                ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 2.0f * windowDevicePixelRatio);
                ImGui::PushFont(icon);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); 
                if (ImGui::Button("B", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
                    //selectedNode = 0;
                    UndoManager::GetInstance()->SelectNode(selectedNode, 0, focusNode, false);
                }
                ImGui::PopStyleVar();
                ImGui::PopFont();
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
                ImGui::SetItemTooltip("Deselect Node");
                ImGui::PopStyleVar();

                ImGui::SameLine();
                ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 1.0f * windowDevicePixelRatio);
                ImGui::PushFont(icon);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (ImGui::Button("A", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
                    if (selectedNode != 0) {
                        DeleteAtEndOfFrame(selectedNode);
                    }
                    //selectedNode = 0;
                    UndoManager::GetInstance()->SelectNode(selectedNode, 0, focusNode, false);
                }
                ImGui::PopStyleVar();
                ImGui::PopFont();
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
                ImGui::SetItemTooltip("Delete Node");
                ImGui::PopStyleVar();

                if (disabled) {
                    ImGui::EndDisabled();
                }

                filterEnd = rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 4.0f * windowDevicePixelRatio;
                filterEnd -= FOOTER_BUTTON_PADDING * 2.0f * windowDevicePixelRatio;
            }

            ImGui::PopStyleVar();
        }

        { // Filter
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(FOOTER_BUTTON_PADDING * windowDevicePixelRatio);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f * windowDevicePixelRatio);
            ImGui::PushItemWidth(filterEnd);
            if (ImGui::InputTextWithHint("##HierarchyFilter", "Filter", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll)) {
                filter.Build();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar();
        }
    }
    else {
        dragAndDrop.Reset();
        ImGui::Text("No Hierarchy Provided to display");
    }

    if (dragAndDrop.source != 0 && dragAndDrop.target != 0) {
        IM_ASSERT(dragAndDrop.mode != SceneNode::DragDropInsertMode::UNSET);

        if (dragAndDrop.mode == SceneNode::DragDropInsertMode::CHILD) {
            //dragAndDrop.target->AddChild(dragAndDrop.source);
            // TODO: Last argument used to be null!
            UndoManager::GetInstance()->ReparentNode(dragAndDrop.source, dragAndDrop.target, dragAndDrop.target->GetFirstChild());
        }
        else if (dragAndDrop.mode == SceneNode::DragDropInsertMode::BEFORE) {
            //dragAndDrop.target->GetParent()->AddChildBefore(dragAndDrop.source, dragAndDrop.target);
            UndoManager::GetInstance()->ReparentNode(dragAndDrop.source, dragAndDrop.target->GetParent(), dragAndDrop.target);
        }
        else {
            //dragAndDrop.target->GetParent()->AddChildAfter(dragAndDrop.target, dragAndDrop.source);
            UndoManager::GetInstance()->ReparentNode(dragAndDrop.source, dragAndDrop.target->GetParent(), dragAndDrop.target->GetNextSibling());
        }

        dragAndDrop.Reset();
    }

    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();

    ImGui::PopStyleVar();
}

void Application::ImguiInspector() {
    if (!showInspector) {
        return;
    }
    

    ImGui::Begin("Inspector");
    ImGui::PushFont(argon);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, defaultWindowPadding);

    bool displayed = false;

    if (focusNode) {
        if (selectedNode != 0) {
            selectedNode->EditorImmediateInspector(selectedAnimation, selectedFrame);
        }
        else if (selectedAsset != 0) {
            selectedAsset->EditorImmediateInspector(&selectedFrame);
        }
        else {
            ImGui::Text("Nothing selected");
        }
    }
    else {
        if (selectedAsset != 0) {
            selectedAsset->EditorImmediateInspector(&selectedFrame);
        }
        else if (selectedNode != 0) {
            selectedNode->EditorImmediateInspector(selectedAnimation, selectedFrame);
        }
        else {
            ImGui::Text("Nothing selected");
        }
    }
   
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();
}

void Application::ImguiAssets() {
    if (!showAssets) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Assets");
    ImGui::PushFont(argon);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);

    ImVec2 windowRegion = ImGui::GetContentRegionAvail();
    windowRegion.y -= 28.0f * windowDevicePixelRatio;

    ImGui::BeginChild("AssetsScrollArea", windowRegion, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f * windowDevicePixelRatio, 10.0f * windowDevicePixelRatio));
    AssetManager* assetManager = AssetManager::GetInstance();
    for (auto iter = assetManager->Begin(), end = assetManager->End(); iter != end; iter++) {
        Asset* asset = iter->second;
        if (asset->deleted) {
            continue;
        }
        const std::string& fileName = iter->second->name;

        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
        if (iter->second->GetType() != AssetType::ASSET_ATLAS) {
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
        }
        else {
            AtlasAsset* atlas = (AtlasAsset*)iter->second;
            if (atlas->NumFrames() == 0) {
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
            }
        }

        if (Asset::clicked == iter->second || selectedAsset == iter->second) {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool is_open = ImGui::TreeNodeEx((void*)asset, node_flags, fileName.c_str());

        {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
                Asset::clicked = iter->second;
            }

            if (Asset::clicked == iter->second && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                if (ImGui::IsItemHovered()) {
                    //selectedAsset = iter->second;
                    //focusNode = false;

                    AnimationAsset* selAnim = 0;
                    if (iter->second->GetType() == AssetType::ASSET_ANIMATION) {
                        selAnim = (AnimationAsset*)iter->second;
                        //selectedFrame = 0;
                    }
                    UndoManager::GetInstance()->SelectAsset(selectedAsset, iter->second, focusNode, false, selectedAnimation, selAnim, selectedFrame, selAnim == 0 ? selectedFrame : 0);
                }
                Asset::clicked = 0;
            }
        }

        if (asset->GetType() == AssetType::ASSET_IMAGE) {
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("Assets:Image", (const void*)&asset, sizeof(Asset*));
                ImGui::Text(fileName.c_str());
                ImGui::EndDragDropSource();
            }
        }

        if (is_open && iter->second->GetType() == AssetType::ASSET_ATLAS) {
            AtlasAsset* atlas = (AtlasAsset*)iter->second;
            atlas->ImguiAssetsList(selectedAsset, focusNode, selectedAnimation, selectedFrame);
        }

        if (is_open) {
            ImGui::TreePop();
        }
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        Asset::clicked = 0;
    }

    //ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    float footerWidth = windowRegion.x;
    float rightSide = ImGui::GetCursorPosX() + footerWidth;// -FOOTER_BUTTON_PADDING * windowDevicePixelRatio;

    { // Footer
        ImGui::PushFont(icon);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 5.0f * windowDevicePixelRatio);
        if (ImGui::Button("C", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
            UndoManager::GetInstance()->NewImage();
        }
        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Import Image");
        ImGui::PopStyleVar();

        ImGui::SameLine();
       
        ImGui::PushFont(icon);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 4.0f * windowDevicePixelRatio);
        if (ImGui::Button("F", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
            newThingName = "";
            newAnimFrameRate = 30;
            newAnimFrameCount = 60;
            newAnimLooping = false;
            ImGui::OpenPopup("New Animation");
        }
        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("New Animation");
        ImGui::PopStyleVar();

        ImGui::SameLine();
        ImGui::PushFont(icon);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 3.0f * windowDevicePixelRatio);
        if (ImGui::Button("E", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
            newThingName = "";
            ImGui::OpenPopup("New Atlas");
        }
        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("New Atlas");
        ImGui::PopStyleVar();

        {
            bool disabled = selectedAsset == 0;
            if (disabled) {
                ImGui::BeginDisabled();
            }
            ImGui::SameLine();
            ImGui::PushFont(icon);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 2.0f * windowDevicePixelRatio);
            if (ImGui::Button("B", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
                UndoManager::GetInstance()->SelectAsset(
                    selectedAsset, 0,
                    focusNode, true,
                    selectedAnimation, 0,
                    selectedFrame, 0);
            }
            ImGui::PopFont();
            ImGui::PopStyleVar();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
            ImGui::SetItemTooltip("Deselect Asset");
            ImGui::PopStyleVar();

            ImGui::SameLine();
            ImGui::PushFont(icon);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::SetCursorPosX(rightSide - (FOOTER_BUTTON_WIDTH + FOOTER_BUTTON_PADDING) * 1.0f * windowDevicePixelRatio);
            if (ImGui::Button("A", FOOTER_BUTTON_SIZE * windowDevicePixelRatio)) {
                if (selectedAsset != 0) {
                    Asset* toDelete = selectedAsset;
                    UndoManager::GetInstance()->SelectAsset(selectedAsset, 0, focusNode, true, selectedAnimation,
                        (selectedAnimation == selectedAsset) ? 0 : selectedAnimation, selectedFrame,
                        (selectedAnimation == selectedAsset) ? 0 : selectedFrame);
                    UndoManager::GetInstance()->DeleteAsset(toDelete);
                }
            }
            ImGui::PopFont();
            ImGui::PopStyleVar();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
            ImGui::SetItemTooltip("Delete selected asset");
            ImGui::PopStyleVar();

            if (disabled) {
                ImGui::EndDisabled();
            }
        }
    }
    ImGui::PopStyleVar();


    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, defaultWindowPadding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);
    ImGui::SetNextWindowSize(ImVec2(250.0f * windowDevicePixelRatio, 135 * windowDevicePixelRatio));
    ImGui::PushFont(xenon);
    if (ImGui::BeginPopupModal("New Animation", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PopFont();
        ImGui::BeginTable("##newAnimationConfigAll", 2);
        {
            ImGui::TableSetupColumn("##newAnimationAllTheThingsAAA", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("##newAnimationAllTheThingsBBB", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputText("##newAnimationName", &newThingName);
            ImGui::PopItemWidth();

            ImGui::TableNextColumn();
            ImGui::Text("Frame Rate");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputInt("##newAnimationFPS", &newAnimFrameRate);
            ImGui::PopItemWidth();

            ImGui::TableNextColumn();
            ImGui::Text("Frame Count");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputInt("##newAnimationFPC", &newAnimFrameCount);
            ImGui::PopItemWidth();
            if (newAnimFrameCount < 2) {
                newAnimFrameCount = 2;
            }

            ImGui::TableNextColumn();
            ImGui::Text("Looping");
            ImGui::TableNextColumn();
            ImGui::Checkbox("##newAnimationIsLooop", &newAnimLooping);
        }
        ImGui::EndTable();

        if (newThingName.length() == 0) {
            ImGui::BeginDisabled();
        }
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 95.0f * windowDevicePixelRatio);
        if (ImGui::Button("Create", ImVec2(70 * windowDevicePixelRatio, 20.0f * windowDevicePixelRatio))) {
            if (newThingName.length() > 0) {
                //AssetManager::GetInstance()->NewAnimation(newThingName, newAnimFrameRate, newAnimFrameCount, newAnimLooping);
                UndoManager::GetInstance()->NewAnimation(newThingName, newAnimFrameRate, newAnimFrameCount, newAnimLooping);
            }
            ImGui::CloseCurrentPopup();
        }
        if (newThingName.length() == 0) {
            ImGui::EndDisabled();
        }
        ImGui::SameLine();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("Cancel", ImVec2(70 * windowDevicePixelRatio, 20.0f * windowDevicePixelRatio))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    else {
        ImGui::PopFont();
    }
    
    ImGui::PushFont(xenon);
    ImGui::SetNextWindowSize(ImVec2(250.0f * windowDevicePixelRatio, 90 * windowDevicePixelRatio));
    if (ImGui::BeginPopupModal("New Atlas", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PopFont();
        ImGui::Text("Name: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::InputText("##newAtlasName", &newThingName);
        ImGui::PopItemWidth();

        ImGui::Text("File (optional): ");
        ImGui::SameLine();
        if (ImGui::Button("Browse", ImVec2(87 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            _FileOperationStarted();
           
            PlatformSelectFile("json\0*.json\0All\0*.*\0\0",
                [](const char* _fileName_, unsigned char* _buffer_, unsigned int _size_) {
                    std::string& newThingName = Application::GetInstance()->newThingName;
                    std::string& guiAtlasTargetFileName = Application::GetInstance()->guiAtlasTargetFileName;
                    bool& guiLoadAtlasFromFile = Application::GetInstance()->guiLoadAtlasFromFile;
                    void*& guiAtlasFileData = Application::GetInstance()->guiAtlasFileData;
                    unsigned int& guiAtlasFileSize = Application::GetInstance()->guiAtlasFileSize;

                    newThingName = (_fileName_ == 0) ? "" : _fileName_;
                    guiAtlasTargetFileName = newThingName;

                    size_t lastIndex = newThingName.find_last_of('/');
                    if (lastIndex == std::string::npos) {
                        lastIndex = newThingName.find_last_of("\\");
                    }

                    if (lastIndex != std::string::npos) {
                        newThingName = newThingName.substr(lastIndex + 1);
                    }

                    guiLoadAtlasFromFile = _fileName_ != 0 && _fileName_[0] != 0 && _buffer_ != 0 && _size_ != 0;
                    if (guiLoadAtlasFromFile) {
                        guiAtlasFileData = malloc(_size_);
                        memcpy(guiAtlasFileData, _buffer_, _size_);
                        guiAtlasFileSize = _size_;
                    }

                    Application::GetInstance()->_FileOperationFinished();
                }
            );
        }

        if (newThingName.length() == 0) {
            ImGui::BeginDisabled();
        }

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 95.0f * windowDevicePixelRatio);
        if (ImGui::Button("Create", ImVec2(70 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            if (guiLoadAtlasFromFile) {
                //newAtlas = AssetManager::GetInstance()->LoadAtlasFromFile(newThingName, guiAtlasTargetFileName.c_str());
                //UndoManager::GetInstance()->NewAtlasFromFile(newThingName.c_str(), guiAtlasTargetFileName.c_str());
                UndoManager::GetInstance()->NewAtlasFromMemory(newThingName.c_str(), guiAtlasTargetFileName.c_str(), (unsigned char*)guiAtlasFileData, guiAtlasFileSize);
                free(guiAtlasFileData);
            }
            else {
                //newAtlas = AssetManager::GetInstance()->NewAtlas(newThingName);
                UndoManager::GetInstance()->NewAtlas(newThingName.c_str());
            }
            guiLoadAtlasFromFile = false;
            guiAtlasFileData = 0;
            guiAtlasFileSize = 0;
            
            ImGui::CloseCurrentPopup();
        }
        if (newThingName.length() == 0) {
            ImGui::EndDisabled();
        }
        ImGui::SameLine();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("Cancel", ImVec2(70 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    else {
        ImGui::PopFont();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();


    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
}

void Application::ImguiScene() {
    const float LEFT_TOOLBAR_SIZE = 28 * windowDevicePixelRatio;
    const float TOP_TOOLBAR_SIZE = 30 * windowDevicePixelRatio;
    const float GRID_STEP = panTool.gridSize * panTool.zoom;
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::Begin("Scene");
    ImGui::PushFont(argon);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);
    ImVec2 contentRegionSize = ImGui::GetContentRegionAvail();
    
    bool openPopup = false;
    ImGui::BeginChild("##sceneSettingsPanel", ImVec2(contentRegionSize.x, TOP_TOOLBAR_SIZE), ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        ImGui::PushFont(icon);
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        float oldY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(5 * windowDevicePixelRatio);
        if (ImGui::Button("H", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            openPopup = true;
        }
        ImGui::PopFont();

        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Menu");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        if (activeTool != 0) {
            ImGui::SetCursorPosY(8 * windowDevicePixelRatio);
            ImGui::SetCursorPosX(LEFT_TOOLBAR_SIZE + 5 * windowDevicePixelRatio);
            activeTool->ImGuiEditorBar();
        }
    }
    ImGui::EndChild();

    ImGui::PushFont(icon);
    ImGui::BeginChild("##sceneControlPanel", ImVec2(LEFT_TOOLBAR_SIZE, contentRegionSize.y - TOP_TOOLBAR_SIZE), ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        float padding = 1.0f;
        float seperator = 6.0f;

        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        if (selectedAnimation == 0) {
            ImGui::BeginDisabled();
        }
        if (isPlaying) {
            if (ImGui::Button("N", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
                isPlaying = false;
            }
            if (selectedAnimation != 0) {
                ImGui::PushFont(argon);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
                ImGui::SetItemTooltip("Pause");
                ImGui::PopStyleVar();
                ImGui::PopFont();
            }
        }
        else {
            if (ImGui::Button("O", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
                isPlaying = true;
                selectedFrame = 0;
                playbackTime = (float)selectedFrame / (float)selectedAnimation->GetFrameCount() * selectedAnimation->GetPlaybackTime();
            }
            if (selectedAnimation != 0) {
                ImGui::PushFont(argon);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
                ImGui::SetItemTooltip("Play");
                ImGui::PopStyleVar();
                ImGui::PopFont();
            }
        }
        if (selectedAnimation == 0) {
            ImGui::PushFont(argon);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
            ImGui::SetItemTooltip("Select Animation to Play");
            ImGui::PopStyleVar();
            ImGui::PopFont();

            ImGui::EndDisabled();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + seperator * windowDevicePixelRatio);

        bool isActiveTool = activeTool == &translateTool;
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        if (isActiveTool) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 60, 160, 255));
        }
        if (ImGui::Button("I", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            if (activeTool == &translateTool) {
                //activeTool = 0;
            }
            else {
                activeTool = &translateTool;
            }
        }
        if (isActiveTool) {
            ImGui::PopStyleColor();
        }

        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Translate");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding * windowDevicePixelRatio);


        isActiveTool = activeTool == &rotateTool;
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        if (isActiveTool) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 60, 160, 255));
        }
        if (ImGui::Button("J", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            if (activeTool == &rotateTool) {
                //activeTool = 0;
            }
            else {
                activeTool = &rotateTool;
            }
        }
        if (isActiveTool) {
            ImGui::PopStyleColor();
        }

        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Rotate");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding * windowDevicePixelRatio);

        isActiveTool = activeTool == &scaleTool;
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        if (isActiveTool) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 60, 160, 255));
        }
        if (ImGui::Button("K", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            if (activeTool == &scaleTool) {
                //activeTool = 0;
            }
            else {
                activeTool = &scaleTool;
            }
        }
        if (isActiveTool) {
            ImGui::PopStyleColor();
        }

        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Scale");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding * windowDevicePixelRatio);

        isActiveTool = activeTool == &pivotTool;
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        if (isActiveTool) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 60, 160, 255));
        }
        if (ImGui::Button("L", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            if (activeTool == &pivotTool) {
                //activeTool = 0;
            }
            else {
                activeTool = &pivotTool;
            }
        }
        if (isActiveTool) {
            ImGui::PopStyleColor();
        }

        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Adjust Pivot");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + seperator * windowDevicePixelRatio);

        isActiveTool = activeTool == &panTool;
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        if (isActiveTool) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 60, 160, 255));
        }
        if (ImGui::Button("M", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            if (activeTool == &panTool) {
                //activeTool = 0;
            }
            else {
                activeTool = &panTool;
            }
        }
        if (isActiveTool) {
            ImGui::PopStyleColor();
        }

        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Pan View");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        float skipHeight = ImGui::GetContentRegionAvail().y - (20 * windowDevicePixelRatio + padding * windowDevicePixelRatio) * 2 - (5 * windowDevicePixelRatio);
        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + skipHeight);
        bool can_undo = UndoManager::GetInstance()->CanUndo();
        if (!can_undo) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("P", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            UndoManager::GetInstance()->Undo();
        }
        if (!can_undo) {
            ImGui::EndDisabled();
        }
        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Undo");
        ImGui::PopStyleVar();
        ImGui::PopFont();

        ImGui::SetCursorPosX(4 * windowDevicePixelRatio);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (seperator * 0.5f * windowDevicePixelRatio));
        bool can_redo = UndoManager::GetInstance()->CanRedo();
        if (!can_redo) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Q", ImVec2(20 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
            UndoManager::GetInstance()->Redo();
        }
        if (!can_redo) {
            ImGui::EndDisabled();
        }
        ImGui::PushFont(argon);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
        ImGui::SetItemTooltip("Redo");
        ImGui::PopStyleVar();
        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::SameLine();

    if (activeTool != 0) {
        activeTool->target = selectedNode;
    }

    ImVec2 windowOffset = ImGui::GetCursorScreenPos();// 
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x - 1, canvas_p0.y + canvas_sz.y - 1);

    ImVec2 cameraPanView = panTool.pan + canvas_sz * 0.5f;

    ImDrawList* list = ImGui::GetWindowDrawList();
    { // Drag border & bg (unclipped)
        list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    }
    list->PushClipRect(canvas_p0, canvas_p1, true);
    ImU32 gridCol = IM_COL32(70, 70, 70, 255);
    { // Main content
        if (panTool.gridVisible) { // Regular grid
            for (float x = fmodf(cameraPanView.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP) {
                list->AddLine( // windowOffset is already applied to canvas_p0
                    ImVec2(canvas_p0.x + x, canvas_p0.y),
                    ImVec2(canvas_p0.x + x, canvas_p1.y),
                    gridCol, windowDevicePixelRatio);
            }
            for (float y = fmodf(cameraPanView.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP) {
                list->AddLine( // windowOffset is already applied to canvas_p0
                    ImVec2(canvas_p0.x, canvas_p0.y + y),
                    ImVec2(canvas_p1.x, canvas_p0.y + y),
                    gridCol, windowDevicePixelRatio);
            }
        }

        if (panTool.gridVisible) { // Decorative grid (ie thick center bars and circle
            ImVec2 canvas_p2 = ImVec2(canvas_p0.x - canvas_sz.x, canvas_p0.y - canvas_sz.y);
            list->AddLine(
                cameraPanView + ImVec2(canvas_p0.x, canvas_p2.y),
                cameraPanView + ImVec2(canvas_p0.x, canvas_p1.y),
                gridCol, 3 * windowDevicePixelRatio);
            list->AddLine( // windowOffset is already applied to canvas_p0
                cameraPanView + ImVec2(canvas_p2.x, canvas_p0.y),
                cameraPanView + ImVec2(canvas_p1.x, canvas_p0.y),
                gridCol, 3 * windowDevicePixelRatio);

            list->AddCircle(windowOffset + cameraPanView, GRID_STEP / 2, gridCol, 0, 3 * windowDevicePixelRatio);
        }

        visibleSplires.clear();
        for (SceneNode* iter = SceneNode::DepthFirst(0, *rootNode); iter != 0; iter = SceneNode::DepthFirst(iter, *rootNode)) {
            if (iter->rtti == NodeType::SPRITE_NODE) {
                SpriteNode* spriteNode = (SpriteNode*)iter;
                bool vis = spriteNode->visible;
                if (selectedAnimation != 0 && !selectedAnimation->deleted && selectedFrame >= 0) {
                    Track* visibleTrack = selectedAnimation->GetTrack(spriteNode, "visible");
                    if (visibleTrack != 0) {
                        vis = visibleTrack->InterpolateB(selectedFrame, selectedAnimation->looping);
                    }
                }

                if (vis) {
                    visibleSplires.push_back(spriteNode);
                }
            }
        }
        std::sort(visibleSplires.begin(), visibleSplires.end(),
            [](const SpriteNode* lhs, const SpriteNode* rhs)->bool {
                return lhs->sort < rhs->sort;
            }
        );

        for (int i = 0, size = (int)visibleSplires.size(); i < size; ++i) {
            SpriteNode* spriteNode = visibleSplires[i];
            ImageAsset* image = spriteNode->image;
            if (image == 0 || image->deleted) {
                continue;
            }
            ImU32 spriteTint = ImGui::ColorConvertFloat4ToU32(spriteNode->tint);
            if (selectedAnimation != 0 && !selectedAnimation->deleted && selectedFrame >= 0) {
                Track* tintTrack = selectedAnimation->GetTrack(spriteNode, "tint");
                if (tintTrack != 0) {
                    spriteTint = tintTrack->InterpolateC(selectedFrame, selectedAnimation->looping);
                }
            }

            if (image != 0 && image->id != 0 && spriteNode->visible) {
                ImVec2 pivot = spriteNode->GetPivot() * spriteNode->size;
               
                if (activeTool == &pivotTool && pivotTool.isActive && activeTool->target == spriteNode) {
                    pivot = pivotTool.objectCurrentPivot;

                    //pivot.x = 1.0f - pivot.x;
                    //pivot.y = 1.0f - pivot.y;

                    if (spriteNode->frame != 0 && spriteNode->frame->rotated) {
                        pivot = ImVec2(pivot.y * spriteNode->width, pivot.x * spriteNode->height);
                    }
                    else {
                        pivot = pivot * spriteNode->size;
                    }
                }

                ImVec2 uv1(0, 0);
                ImVec2 uv2(1, 0);
                ImVec2 uv3(1, 1);
                ImVec2 uv4(0, 1);

                if (spriteNode->frame != 0 && !spriteNode->frame->deleted) {
                    uv1 = spriteNode->frame->Uv0();
                    uv2 = spriteNode->frame->Uv1();
                    uv3 = spriteNode->frame->Uv2();
                    uv4 = spriteNode->frame->Uv3();
                } 
                else {
                    ImVec2 invSize(1, 1);
                    if (spriteNode->image != 0) {
                        ImVec2 srcSize = spriteNode->image->size;
                        if (srcSize.x < -0.0001f || srcSize.x > 0.0001f) {
                            invSize.x = 1.0f / srcSize.x;
                        }
                        if (srcSize.y < -0.0001f || srcSize.y > 0.0001f) {
                            invSize.y = 1.0f / srcSize.y;
                        }
                    }
                    uv1 = spriteNode->frameMin;
                    uv3 = spriteNode->frameMax;
                    uv2.x = uv3.x; uv2.y = uv1.y;
                    uv4.x = uv1.x; uv4.y = uv3.y;

                    uv1 = uv1 * invSize;
                    uv3 = uv3 * invSize;
                    uv2 = uv2 * invSize;
                    uv4 = uv4 * invSize;
                }

                ImVec2 spriteNodeSize = spriteNode->size;
                if (spriteNode->frame != 0 && !spriteNode->frame->deleted) {
                    if (spriteNode->frame->rotated) {
                        float t = pivot.x;
                        pivot.x = pivot.y;
                        pivot.y = t;
                        t = spriteNodeSize.x;
                        spriteNodeSize.x = spriteNodeSize.y;
                        spriteNodeSize.y = t;

                    }
                }
                ImVec2 p0 = ImVec2(0, 0) - pivot;
                ImVec2 p1 = ImVec2(spriteNodeSize.x, 0) - pivot;
                ImVec2 p2 = spriteNodeSize - pivot;
                ImVec2 p3 = ImVec2(0, spriteNodeSize.y) - pivot;

                ImMat3 worldMat = spriteNode->GetWorldMatrix(activeTool, selectedAnimation, selectedFrame);

                p0 = windowOffset + cameraPanView + (worldMat * ImVec3(p0, 1)).Vec2() * panTool.zoom;
                p1 = windowOffset + cameraPanView + (worldMat * ImVec3(p1, 1)).Vec2() * panTool.zoom;
                p2 = windowOffset + cameraPanView + (worldMat * ImVec3(p2, 1)).Vec2() * panTool.zoom;
                p3 = windowOffset + cameraPanView + (worldMat * ImVec3(p3, 1)).Vec2() * panTool.zoom;

                list->AddImageQuad(image->id,   p0, p1, p2, p3,
                                                uv1, uv2, uv3, uv4, spriteTint);

                if (activeTool == &pivotTool && pivotTool.isActive) {
                    if (pivotTool.outline == spriteNode) {
                        ImU32 color = IM_COL32(255, 255, 0, 255);
                        list->AddQuad(p0, p1, p2, p3, color, pivotTool.thickness);
                    }
                }
            }
        }
    }


    // Drag Border
    list->AddRect(canvas_p0, canvas_p1, IM_COL32(100, 100, 100, 255), windowDevicePixelRatio);


    ImGui::InvisibleButton("##sceneCanvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
    const bool is_hovered = ImGui::IsItemHovered(); // Hovered
    const bool is_active = ImGui::IsItemActive();   // Held

    Tool::toolZoom = panTool.zoom;

    if (is_hovered) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (leftClicked) {
                IM_ASSERT(false && "This is an error! Previous click never finished!");
            }
            leftClicked = true;
            if (activeTool != 0) {
                activeTool->OnMouseDown(io.MousePos.x, io.MousePos.y, ToolMouseButton::LEFT);
            }
        }
        else if (leftClicked && ImGui::IsMouseDown(ImGuiMouseButton_Left) && activeTool != 0) {
            activeTool->OnMouseDrag(io.MousePos.x, io.MousePos.y, io.MouseDelta.x, io.MouseDelta.y, ToolMouseButton::LEFT);
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            if (rightClicked) {
                IM_ASSERT(false && "This is an error! Previous click never finished!");
            }
            rightClicked = true;
            if (activeTool != 0) {
                activeTool->OnMouseDown(io.MousePos.x, io.MousePos.y, ToolMouseButton::RIGHT);
            }
        }
        else if (rightClicked && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            if (activeTool != 0) {
                activeTool->OnMouseDrag(io.MousePos.x, io.MousePos.y, io.MouseDelta.x, io.MouseDelta.y, ToolMouseButton::RIGHT);
            }
            if (activeTool != &panTool) {
                panTool.OnMouseDrag(io.MousePos.x, io.MousePos.y, io.MouseDelta.x, io.MouseDelta.y, ToolMouseButton::RIGHT);
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
            if (middleClicked) {
                IM_ASSERT(false && "This is an error! Previous click never finished!");
            }
            middleClicked = true;
            if (activeTool != 0) {
                activeTool->OnMouseDown(io.MousePos.x, io.MousePos.y, ToolMouseButton::MIDDLE);
            }
        }
        else if (middleClicked && ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            if (activeTool != 0) {
                activeTool->OnMouseDrag(io.MousePos.x, io.MousePos.y, io.MouseDelta.x, io.MouseDelta.y, ToolMouseButton::MIDDLE);
            }
            if (activeTool != &panTool) {
                panTool.OnMouseDrag(io.MousePos.x, io.MousePos.y, io.MouseDelta.x, io.MouseDelta.y, ToolMouseButton::MIDDLE);
            }
        }
    }
    if (leftClicked && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (activeTool != 0) {
            activeTool->OnMouseUp(ToolMouseButton::LEFT);
        }
        leftClicked = false;
    }
    if (rightClicked && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        if (activeTool != 0) {
            activeTool->OnMouseUp(ToolMouseButton::RIGHT);
        }
        rightClicked = false;
    }
    if (middleClicked && !ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        if (activeTool != 0) {
            activeTool->OnMouseUp(ToolMouseButton::MIDDLE);
        }
        middleClicked = false;
    }

    if (is_hovered && io.MouseWheel) {
        if (activeTool != 0) {
            activeTool->OnScroll(io.MouseWheel);
        }
        if (activeTool != &panTool) {
            panTool.OnScroll(io.MouseWheel);
        }
    }

    if (activeTool != 0) {
        activeTool->Draw(list, windowOffset + cameraPanView, panTool.zoom, selectedAnimation, selectedFrame);
    }
    { // Reset tools not in use
        if (activeTool != &translateTool) {
            translateTool.lastDrawnAsset = 0;
            translateTool.lastFrame = -1;
        }
        if (activeTool != &rotateTool) {
            rotateTool.lastDrawnAsset = 0;
            rotateTool.lastFrame = -1;
        }
        if (activeTool != &panTool) {
            panTool.lastDrawnAsset = 0;
            panTool.lastFrame = -1;
        }
    }

    list->PopClipRect();

    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    if (openPopup) {
        ImGui::OpenPopup("##SETTINGS");
    }
    if (ImGui::BeginPopup("##SETTINGS")) {
        ImGui::PushFont(xenon);
        ImGui::SeparatorText("File");
        ImGui::PopFont();
        
        ImGui::PushFont(argon);
        if (ImGui::Selectable("New")) {
            createNewSceneAfterFrame = true;
        }
        if (ImGui::Selectable("Open")) {
            openScene = true;
        }
        if (ImGui::Selectable("Save")) {
            saveSceneAs = true;
        }
        ImGui::PopFont();

        ImGui::PushFont(xenon);
        ImGui::SeparatorText("Edit");
        ImGui::PopFont();

        ImGui::PushFont(argon);
        bool can_undo = UndoManager::GetInstance()->CanUndo();
        bool can_redo = UndoManager::GetInstance()->CanRedo();
        if (!can_undo) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Selectable("Undo")) {
            UndoManager::GetInstance()->Undo();
        }
        if (!can_undo) {
            ImGui::EndDisabled();
        }
        if (!can_redo) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Selectable("Redo")) {
            UndoManager::GetInstance()->Redo();
        }
        if (!can_redo) {
            ImGui::EndDisabled();
        }
        if (!can_undo && !can_redo) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Selectable("Clear History")) {
            UndoManager::DestroyAll();
        }
        if (!can_undo && !can_redo) {
            ImGui::EndDisabled();
        }
        ImGui::PopFont();

        ImGui::PushFont(xenon);
        ImGui::SeparatorText("Window");
        ImGui::PopFont();

        ImGui::PushFont(argon);
        if (ImGui::MenuItem("Inspector", "", &showInspector)) {
        }
        if (ImGui::MenuItem("Hierarchy", "", &showHierarchy)) {
        }
        if (ImGui::MenuItem("Assets", "", &showAssets)) {
        }
        if (ImGui::MenuItem("Sequencer", "", &showSequencer)) {
        }
        if (ImGui::MenuItem("History", "", &showHistory)) {
        }
        ImGui::PopFont();

        ImGui::PushFont(xenon);
        ImGui::SeparatorText("About");
        ImGui::PopFont();

        ImGui::PushFont(argon);
        if (ImGui::Selectable("Key Frame Studio")) {
        }
        if (ImGui::Selectable("Github")) {
        }
        ImGui::PopFont();

        ImGui::EndPopup();
    }
}

void Application::ImguiHistory() {
    if (!showHistory) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("History");
    ImGui::PushFont(argon);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);

    UndoManager::GetInstance()->Imgui();

    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
}

void Application::ImguiSequencer() {
    if (!showSequencer) {
        return;
    }

    if (selectedAnimation != 0 && !selectedAnimation->deleted) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    }

    ImGui::Begin("Sequencer");
    ImGui::PushFont(argon);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, defaultFramePadding);
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, defaultWindowPadding);

    if (selectedAnimation == 0 || selectedAnimation->deleted) {
        ImGui::Text("No active animation");
    }
    else {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        selectedFrame = selectedAnimation->ImguiDrawSequencer(selectedFrame);
        ImGui::PopStyleVar();
    }

    //ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();
    if (selectedAnimation != 0 && !selectedAnimation->deleted) {
        ImGui::PopStyleVar();
    }
}

void Application::Initialize(float dpi) {
    ImGuiIO& io = ImGui::GetIO(); 
    io.IniFilename = 0;
    io.LogFilename = 0;

    AssetManager::GetInstance(); // Loads all assets
    rootNode = new SceneNode("Root Node");

    resetDockSpace = true;
    activeTool = &panTool;

#if 0
    {
        float _dpi = GetDpiForWindow(GetActiveWindow());
        windowDevicePixelRatio = _dpi / 96.f; // 96 is the "standard dpi"
    }
#endif

    windowDevicePixelRatio = dpi;
    SkinImGui(dpi);
}

void Application::Update() {

    if (resetDockSpace) {
        ResetDockSpace();
    }

    if (isPlaying && selectedAnimation != 0) {
        float length = selectedAnimation->GetPlaybackTime();

        playbackTime = playbackTime + ImGui::GetIO().DeltaTime;
        if (selectedAnimation->looping) {
            playbackTime = fmodf(playbackTime, length);
        }
        else if (playbackTime > length) {
            playbackTime = length;
            isPlaying = false;
        }
        selectedFrame = (int)((playbackTime / length) * (float)selectedAnimation->GetFrameCount());
    }

    ImguiHierarchy();
    ImguiInspector();
    ImguiAssets();
    ImguiSequencer();
    ImguiHistory();
    ImguiScene();

    lastActiveTool = activeTool;

    if (deleteAtTheEndOfFrame != 0) {
        //deleteAtTheEndOfFrame->isDeleted = true;
        //deleteAtTheEndOfFrame->ClearParent();
        UndoManager::GetInstance()->DeleteNode(deleteAtTheEndOfFrame);
        deleteAtTheEndOfFrame = 0;
    }
    
    if (saveSceneAs) {
        _SaveAs();
    }
    if (openScene) {
        _OpenScene();
    }
    if (createNewSceneAfterFrame) {
        _NewScene();
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        dragAndDrop.source = 0;
    }
}

void Application::Deserialize(const char* _json, int _len) {
    JsonValue* json = JsonParseString(_json, _len);

    if (json->type != JsonValueType::JSON_OBJECT) {
        IM_ASSERT(json->type == JsonValueType::JSON_OBJECT);
        return;
    }

    if (JsonObjectGet(json, "images")) {
        for (JsonIterator iter = JsonGetIterator(JsonObjectGet(json, "images")); iter.valid; JsonIteratorAdvance(&iter)) {
            JsonValue* image = iter.value;
            JsonValue* size = JsonObjectGet(image, "size");

            const char* name = JsonObjectGet(image, "name")->asString.chars;
            const char* guid = JsonObjectGet(image, "guid")->asString.chars;
            int w = (int)JsonObjectGet(size, "w")->asNumber;
            int h = (int)JsonObjectGet(size, "h")->asNumber;

            const char* data = JsonObjectGet(image, "data")->asString.chars;

            std::string decoded_data = base64_decode(data, false);
            AssetManager::GetInstance()->DeserializeImage(name, guid, decoded_data.c_str(), (int)decoded_data.size());
        }
    }

    if (JsonObjectGet(json, "atlases")) {
        for (JsonIterator iter = JsonGetIterator(JsonObjectGet(json, "atlases")); iter.valid; JsonIteratorAdvance(&iter)) {
            JsonValue* atlas = iter.value;
            JsonValue* size = JsonObjectGet(atlas, "size");
            JsonValue* frames = JsonObjectGet(atlas, "frames");

            const char* name = JsonObjectGet(atlas, "name")->asString.chars;
            const char* guid = JsonObjectGet(atlas, "guid")->asString.chars;
            int w = (int)JsonObjectGet(size, "w")->asNumber;
            int h = (int)JsonObjectGet(size, "h")->asNumber;

            AtlasAsset* at = AssetManager::GetInstance()->DeserializeAtlas(name, guid, w, h);

            for (JsonIterator frame_iter = JsonGetIterator(frames); frame_iter.valid; JsonIteratorAdvance(&frame_iter)) {
                JsonValue* frame = frame_iter.value;

                const char* frame_name = JsonObjectGet(frame, "name")->asString.chars;
                const char* frame_guid = JsonObjectGet(frame, "guid")->asString.chars;
                bool frame_rotated = JsonObjectGet(frame, "rotated")->asBool;

                JsonValue* point = JsonObjectGet(frame, "p0");
                ImVec2 p0((float)JsonObjectGet(point, "x")->asNumber, (float)JsonObjectGet(point, "y")->asNumber);

                point = JsonObjectGet(frame, "p1");
                ImVec2 p1((float)JsonObjectGet(point, "x")->asNumber, (float)JsonObjectGet(point, "y")->asNumber);

                point = JsonObjectGet(frame, "p2");
                ImVec2 p2((float)JsonObjectGet(point, "x")->asNumber, (float)JsonObjectGet(point, "y")->asNumber);

                point = JsonObjectGet(frame, "p3");
                ImVec2 p3((float)JsonObjectGet(point, "x")->asNumber, (float)JsonObjectGet(point, "y")->asNumber);

                AtlasFrame* fr = at->AddFrame(frame_name, p0, p1, p2, p3, frame_rotated);
                fr->SetGuid(frame_guid);
            }
        }
    }
    
    if (JsonObjectGet(json, "hierarchy")) {
        for (JsonIterator iter = JsonGetIterator(JsonObjectGet(json, "hierarchy")); iter.valid; JsonIteratorAdvance(&iter)) {
            JsonValue* node = iter.value;
            _DeserializeNode(rootNode, node);
        }
    }

    if (JsonObjectGet(json, "animations")) {
        for (JsonIterator iter = JsonGetIterator(JsonObjectGet(json, "animations")); iter.valid; JsonIteratorAdvance(&iter)) {
            JsonValue* node = iter.value;

            const char* name = JsonObjectGet(node, "name")->asString.chars;
            const char* guid = JsonObjectGet(node, "guid")->asString.chars;
            int frameRate = (int)JsonObjectGet(node, "frameRate")->asNumber;
            int frameCount = (int)JsonObjectGet(node, "frameCount")->asNumber;
            bool looping = (bool)JsonObjectGet(node, "looping")->asBool;

            AnimationAsset* anim = AssetManager::GetInstance()->DeserializeAnimation(name, guid, frameRate, frameCount, looping);

            JsonValue* tracks = JsonObjectGet(node, "tracks");
            if (tracks != 0) {
                for (JsonIterator tIter = JsonGetIterator(tracks); tIter.valid; JsonIteratorAdvance(&tIter)) {
                    JsonValue* track = tIter.value;

                    const char* tname = JsonObjectGet(track, "name")->asString.chars;
                    const char* ttarget = JsonObjectGet(track, "target")->asString.chars;
                    const char* _rtti = JsonObjectGet(track, "rtti")->asString.chars;
                    
                    const char* trtti = "TRACK_INVALID";
                    TrackType t = TrackType::TRACK_INVALID;
                    if (strcmp(_rtti, "TRACK_FLOAT") == 0) {
                        trtti = "TRACK_FLOAT";
                        t = TrackType::TRACK_FLOAT;
                    }
                    else if (strcmp(_rtti, "TRACK_INT") == 0) {
                        trtti = "TRACK_INT";
                        t = TrackType::TRACK_INT;
                    }
                    else if (strcmp(_rtti, "TRACK_BOOL") == 0) {
                        trtti = "TRACK_BOOL";
                        t = TrackType::TRACK_BOOL;
                    }
                    else if (strcmp(_rtti, "TRACK_COLOR") == 0) {
                        trtti = "TRACK_COLOR";
                        t = TrackType::TRACK_COLOR;
                    }
                    else {
                        IM_ASSERT(false);
                    }

                    SceneNode* found = rootNode->EditorFindRecursive(ttarget);
                    Track* _track = anim->AddTrack(found, tname, t);
                    JsonValue* frames = JsonObjectGet(track, "frames");
                    for (JsonIterator fIter = JsonGetIterator(frames); fIter.valid; JsonIteratorAdvance(&fIter)) {
                        JsonValue* frame = fIter.value;

                        int index = (int)JsonObjectGet(frame, "index")->asNumber;
                        if (t == TrackType::TRACK_FLOAT) {
                            float data = (float)JsonObjectGet(frame, "data")->asNumber;
                            _track->AddFrameF(data, index);
                        }
                        else if (t == TrackType::TRACK_INT) {
                            int data = (int)JsonObjectGet(frame, "data")->asNumber;
                            _track->AddFrameI(data, index);
                        }
                        else if (t == TrackType::TRACK_BOOL) {
                            bool data = (bool)JsonObjectGet(frame, "data")->asBool;
                            _track->AddFrameB(data, index);
                        }
                        else if (t == TrackType::TRACK_COLOR) {
                            ImU32 data = (ImU32)JsonObjectGet(frame, "data")->asNumber;
                            _track->AddFrameC(data, index);
                        }
                        else {
                            IM_ASSERT(false);
                        }
                    }
                }
            }
        }
    }
}

void Application::_DeserializeNode(SceneNode* parent, JsonValue* obj) {
    const char* rtti = JsonObjectGet(obj, "rtti")->asString.chars;

    bool isTransform = strcmp(rtti, "TRANSFORM_NODE") == 0;
    bool isSprite = strcmp(rtti, "SPRITE_NODE") == 0;
    IM_ASSERT(isTransform || isSprite);

    const char* name = JsonObjectGet(obj, "name")->asString.chars;
    const char* guid = JsonObjectGet(obj, "guid")->asString.chars;

    ImVec2 position;
    float rotation = 0.0f;
    ImVec2 scale(1, 1);

    if (isTransform || isSprite) {
        JsonValue* jPos = JsonObjectGet(obj, "position");
        JsonValue* jScl = JsonObjectGet(obj, "scale");

        position.x = (float)JsonObjectGet(jPos, "x")->asNumber;
        position.y = (float)JsonObjectGet(jPos, "y")->asNumber;
        rotation = (float)JsonObjectGet(obj, "rotation")->asNumber;
        scale.x = (float)JsonObjectGet(jScl, "x")->asNumber;
        scale.y = (float)JsonObjectGet(jScl, "y")->asNumber;
    }

    // Only used for sprites
    ImVec2 size, pivot, frameMin, frameMax;
    ImVec4 tint;
    int sort = 0;
    bool visible = true;
    ImageAsset* image = 0;
    AtlasFrame* frame = 0;

    if (isSprite) {
        JsonValue* jSize = JsonObjectGet(obj, "size");
        JsonValue* jPivot = JsonObjectGet(obj, "pivot");
        size.x = (float)JsonObjectGet(jSize, "w")->asNumber;
        size.y = (float)JsonObjectGet(jSize, "h")->asNumber;
        pivot.x = (float)JsonObjectGet(jPivot, "x")->asNumber;
        pivot.y = (float)JsonObjectGet(jPivot, "y")->asNumber;

        JsonValue* jMin = JsonObjectGet(obj, "frameMin");
        if (jMin != 0) {
            frameMin.x = (float)JsonObjectGet(jMin, "x")->asNumber;
            frameMin.y = (float)JsonObjectGet(jMin, "y")->asNumber;
        }
        JsonValue* jMax = JsonObjectGet(obj, "frameMax");
        if (jMax != 0) {
            frameMax.x = (float)JsonObjectGet(jMax, "x")->asNumber;
            frameMax.y = (float)JsonObjectGet(jMax, "y")->asNumber;
        }

        JsonValue* jTint = JsonObjectGet(obj, "tint");
        tint.x = (float)JsonObjectGet(jTint, "r")->asNumber;
        tint.y = (float)JsonObjectGet(jTint, "g")->asNumber;
        tint.z = (float)JsonObjectGet(jTint, "b")->asNumber;
        tint.w = (float)JsonObjectGet(jTint, "a")->asNumber;

        sort = (int)JsonObjectGet(obj, "sort")->asNumber;
        visible = JsonObjectGet(obj, "visible")->asBool;

        JsonValue* jImage = JsonObjectGet(obj, "image");
        if (jImage != 0 && jImage->type != JsonValueType::JSON_NULL) {
            image = (ImageAsset*)AssetManager::GetInstance()->Get(jImage->asString.chars);
            if (image != 0) {
                IM_ASSERT(image->GetType() == AssetType::ASSET_IMAGE);
            }
        }
        JsonValue* jFrame = JsonObjectGet(obj, "frame");
        if (jFrame != 0 != JsonValueType::JSON_NULL) {
            const char* frame_uuid = jFrame->asString.chars;
            for (auto iter = AssetManager::GetInstance()->Begin(), end = AssetManager::GetInstance()->End(); iter != end; iter++) {
                if (iter->second->GetType() == AssetType::ASSET_ATLAS) {
                    AtlasAsset* atlas = (AtlasAsset*)iter->second;
                    for (int i = 0, len = atlas->NumFrames(); i < len; ++i) {
                        AtlasFrame* f = atlas->GetFrameByIndex(i);
                        if (strcmp(f->GetGUID().c_str(), frame_uuid) == 0) {
                            frame = f;
                            break;
                        }
                    }
                }
                if (frame != 0) {
                    break;
                }
            }
        }
    }
    
    SceneNode* thisNode = 0;
    if (isTransform) {
        thisNode = SceneNode::DeserializeTransform(name, guid, position, rotation, scale);
    }
    else if (isSprite) {
        thisNode = SceneNode::DeserializeSprite(name, guid, position, rotation, scale, size, pivot, frameMin, frameMax, tint, sort, visible, image, frame);
    }
    thisNode->SetParent(parent);

    JsonValue* children = JsonObjectGet(obj, "children");
    for (JsonIterator iter = JsonGetIterator(children); iter.valid; JsonIteratorAdvance(&iter)) {
        _DeserializeNode(thisNode, iter.value);
    }
}

void Application::Serialize(std::stringstream& output) {
    output << "{\n";
    AssetManager* am = AssetManager::GetInstance();

    { // Hierarchy
        output << "\t\"hierarchy\": [\n";

        rootNode->Serialize(output, 0);

        output << "\t],\n";
    }
    { // Images
        output << "\t\"images\": [\n";

        bool cleanupTrailingComma = false;
        for (std::map<std::string, Asset*>::iterator iter = am->Begin(), end = am->End(); iter != end; iter++) {
            if (iter->second->GetType() == AssetType::ASSET_IMAGE && !iter->second->deleted) {
                iter->second->Serialize(output, 2);
                cleanupTrailingComma = true;
            }
        }
        if (cleanupTrailingComma) {
            output.seekp(-2, output.cur);
            output << " \n";
        }

        output << "\t],\n";
    }
    { // Atlases
        output << "\t\"atlases\": [\n";

        bool cleanupTrailingComma = false;
        for (std::map<std::string, Asset*>::iterator iter = am->Begin(), end = am->End(); iter != end; iter++) {
            if (iter->second->GetType() == AssetType::ASSET_ATLAS && !iter->second->deleted) {
                iter->second->Serialize(output, 2);
                cleanupTrailingComma = true;
            }
        }
        if (cleanupTrailingComma) {
            output.seekp(-2, output.cur);
            output << " \n";
        }

        output << "\t],\n";
    }
    { // Animations
        output << "\t\"animations\": [\n";
        bool cleanupTrailingComma = false;
        for (std::map<std::string, Asset*>::iterator iter = am->Begin(), end = am->End(); iter != end; iter++) {
            if (iter->second->GetType() == AssetType::ASSET_ANIMATION && !iter->second->deleted) {
                iter->second->Serialize(output, 2);
                cleanupTrailingComma = true;
            }
        }
        if (cleanupTrailingComma) {
            output.seekp(-2, output.cur); 
            output << " \n";
        }
        output << "\t]\n";
    }
    output << "}";
}

void Application::_SaveAs() {
    saveSceneAs = false;

    std::stringstream output;
    Serialize(output);
    std::string json = output.str();

    _FileOperationStarted();
    PlatformSaveAs((const unsigned char*)json.c_str(), (unsigned int)json.size(),
        [](bool success) {
            Application::GetInstance()->_FileOperationFinished();
        }
    );

#if 0
    static char path[1024] = { 0 };
    ZeroMemory(path, 1024);

    OPENFILENAMEA saveFileName;
    ZeroMemory(&saveFileName, sizeof(OPENFILENAMEA));
    saveFileName.lStructSize = sizeof(OPENFILENAMEA);
    saveFileName.hwndOwner = GetActiveWindow();// gHwnd;
    saveFileName.lpstrFile = path;
    saveFileName.lpstrFilter = "Key Frame Studio\0*.kfs\0All\0*.*\0\0";
    saveFileName.nMaxFile = 1024;
    saveFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (FALSE != GetSaveFileNameA(&saveFileName)) {
        DWORD written = 0;
        HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        WriteFile(hFile, json.c_str(), (DWORD)json.size(), &written, NULL);
        CloseHandle(hFile);
    }
#endif
}

void Application::_NewScene() {
    UndoManager::DestroyAll();

    createNewSceneAfterFrame = false;
    saveSceneAs = false;
    openScene = false;

    SceneNode::DestroyAll();
    AssetManager::DestroyAll();

    activeTool = &panTool;
    lastActiveTool = 0;
    selectedNode = 0;
    selectedAsset = 0;
    focusNode = true;
    isPlaying = false;
    playbackTime = 0.0f;
    selectedAnimation = 0;
    selectedFrame = 0;
    visibleSplires.clear();
    dragAndDrop.Reset();
    rootNode = new SceneNode("Root Node");
}

void Application::_OpenScene() {
    openScene = false;
    _FileOperationStarted();

    PlatformSelectFile("kfs\0*.kfs\0All\0*.*\0\0",
        [](const char* fileName, unsigned char* buffer, unsigned int size) {
            if (buffer != 0) {
                Application::GetInstance()->_NewScene();
                Application::GetInstance()->Deserialize((const char*)buffer, size);
            }
            Application::GetInstance()->_FileOperationFinished();
        }
    );

#if 0
    if (fileName != 0 && fileName[0] != 0) {
        HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        DWORD bytesInFile = GetFileSize(hFile, 0);
        DWORD bytesRead = 0;

        if (hFile == INVALID_HANDLE_VALUE) {
            return;
        }
        char* fileBuffer = new char[bytesInFile + 1];

        if (ReadFile(hFile, fileBuffer, bytesInFile, &bytesRead, NULL) != 0) {
            _NewScene();
            fileBuffer[bytesInFile] = 0; // Force json parser to stop
            Deserialize(fileBuffer, bytesInFile);
        }

        delete[] fileBuffer;
        CloseHandle(hFile);
    }
#endif
}
