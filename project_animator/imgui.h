#ifndef _H_IMGUI_
#define _H_IMGUI_

#include "../platform/memory.h"
#include "../framework/vec3.h"
#include "color.h"
#include "Node2D.h"
#include "../framework/draw2d.h"

#define IMGUI_ICON_CODEPOINT_RESIZE_HORIZONTAL 'A'
#define IMGUI_ICON_CODEPOINT_RESIZE_VERTICAL 'B'
#define IMGUI_ICON_CODEPOINT_TRASHCAN 'C'
#define IMGUI_ICON_CODEPOINT_NEWLAYER 'D'
#define IMGUI_ICON_CODEPOINT_NEWCHAIN 'E'
#define IMGUI_ICON_CODEPOINT_NEWSPRITE 'F'
#define IMGUI_ICON_CODEPOINT_REMOVECHAIN 'G'
#define IMGUI_ICON_CODEPOINT_REMOVESPRITE 'H'
#define IMGUI_ICON_CODEPOINT_CHAIN 'I'
#define IMGUI_ICON_CODEPOINT_KEY 'J'
#define IMGUI_ICON_CODEPOINT_NOKEY 'K'
#define IMGUI_ICON_CODEPOINT_DESELECT 'L'
#define IMGUI_ICON_CODEPOINT_UNDO 'M'
#define IMGUI_ICON_CODEPOINT_REDO 'N'
#define IMGUI_ICON_CODEPOINT_CLEAR_UNDO 'O'
#define IMGUI_ICON_CODEPOINT_ARROW_DOWN 'P'
#define IMGUI_ICON_CODEPOINT_ARROW_UP 'Q'
#define IMGUI_ICON_CODEPOINT_ARROW_LEFT 'R'
#define IMGUI_ICON_CODEPOINT_ARROW_RIGHT 'S'
#define IMGUI_ICON_CODEPOINT_VISIBLE 'T'
#define IMGUI_ICON_CODEPOINT_INVISIBLE 'U'
#define IMGUI_ICON_CODEPOINT_DEBUGOFF 'V'
#define IMGUI_ICON_CODEPOINT_DEBUGON 'W'
#define IMGUI_ICON_CODEPOINT_COLLAPSED 'X'
#define IMGUI_ICON_CODEPOINT_EXPANDED 'Y'
#define IMGUI_ICON_CODEPOINT_KEYBOARD_OUTLINED 'Z'
#define IMGUI_ICON_CODEPOINT_KEYBOARD_FILLED 'a'
#define IMGUI_ICON_CODEPOINT_LOCK_CLOSED 'b'
#define IMGUI_ICON_CODEPOINT_LOCK_OPEN 'c'
#define IMGUI_ICON_CODEPOINT_ADD_FILE 'd'
#define IMGUI_ICON_APPICON 'e'
#define IMGUI_ICON_ADDTRACK 'f'
#define IMGUI_ICON_PLAY 'g'
#define IMGUI_ICON_PAUSE 'h'
#define IMGUI_ICON_RECORD 'i'
#define IMGUI_ICON_SYNC_ENABLED 'j'
#define IMGUI_ICON_SYNC_DISABLED 'k'
#define IMGUI_ICON_DIAMOND 'l'
#define IMGUI_ICON_TIMELINEKEY 'm'
#define IMGUI_ICON_CLEARKEY 'n'
#define IMGUI_ICON_STOP 'p'
#define IMGUI_ICON_MOVETOOL 'q'
#define IMGUI_ICON_ZOOM_IN 'r'
#define IMGUI_ICON_ZOOM_OUT 's'
#define IMGUI_ICON_ROTATE 'o'
#define IMGUI_ICON_SCALE 't'
#define IMGUI_ICON_MOVE 'u'
#define IMGUI_ICON_PIVOT 'v'
#define IMGUI_ICON_UNCHECKED 'w'
#define IMGUI_ICON_CHECKED 'x'
#define IMGUI_ICON_GRID 'y'
#define IMGUI_ICON_Left '!'
#define IMGUI_ICON_Right '"'
#define IMGUI_ICON_Up '#'
#define IMGUI_ICON_Down '$'

#define IMGUI_TOOLTIP_SIZE 18
#define IMGUI_SPLITTER_GRABBABLE_WIDTH 4.0f
#define IMGUI_PULSE_MS 250
#define IMGUI_LONG_PULSE_MS 500
#define IMGUI_BLINK_MS 800
#define IMGUI_TOOLTIP_MIN 1.5f
#define IMGUI_TOOLTIP_MAX 12.0f
#define IMGUI_TRANSFORM_HEIGHT 110.0f
#define IMGUI_PREVIEW_HEIGHT 110.0f
#define IMGUI_ANIMDETAIL_HEIGHT 75.0f
#define IMGUI_SPRITE_HEIGHT 150.0f
#define IMGUI_DOUBLECLICK_SECOND 0.4f

struct StyleColor {
	union {
		struct {
			float r;
			float g;
			float b;
			float a;
		};
		float rgba[4];
	};

	inline StyleColor(float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 1.0f) :
		r(_r), g(_g), b(_b), a(_a) {}
};

inline StyleColor ColorRGB8(u8 r, u8 g, u8 b) {
	float _r = (float)r / 255.0f;
	float _g = (float)g / 255.0f;
	float _b = (float)b / 255.0f;
	return StyleColor(_r, _g, _b, 1.0f);
}

inline StyleColor ColorRGB8(u8 c) {
	float _r = (float)c / 255.0f;
	float _g = (float)c / 255.0f;
	float _b = (float)c / 255.0f;
	return StyleColor(_r, _g, _b, 1.0f);
}

struct StyleSheet {
	float animationFrameWidth;
	float animationFrameHeight;
	StyleColor frameEven;
	StyleColor frameOdd;

	StyleColor gizmoR;
	StyleColor gizmoG;
	StyleColor gizmoB;
	StyleColor gizmoY;
	StyleColor gizmoR_Hover;
	StyleColor gizmoG_Hover;
	StyleColor gizmoB_Hover;
	StyleColor gizmoY_Hover;

	StyleColor menuBarBg;
	float menuBarHeight;

	StyleColor toolBarBg;
	float toolbarWidth;

	StyleColor footerBg;
	float footerHeight;

	float inspectorMinWidth;
	float timelineMinWidth;
	float animatorMinHeight;

	u32 menuTextLineHeight;

	StyleColor tooltipIconBGColor;
	StyleColor tooltipIconFGColor;
	StyleColor tooltipTextColor;
	StyleColor tooltipTextBGColor;

	StyleColor documentBGColor;
	StyleColor panelBgColor;
	StyleColor keyframeDiamond;

	StyleColor dividerAColor;
	StyleColor dividerBColor;
	float dividerHotTint;
	float dividerActiveTint;

	float headerHeight;
	u32 headerFontSize;
	StyleColor headerBgColor;
	StyleColor headerFontColor;
	StyleColor HeaderBGHotColor;
	StyleColor HeaderBGActiveColor;

	StyleColor imageBlockOutlineColor;
	StyleColor imageBlockBackgroundColor;
	StyleColor imageBlockLabelColor;

	StyleColor gridA;
	StyleColor gridB;

	StyleColor hierarchyFooterBg;
	float hierarchyFooterHeight;
	float hierarchyFooterButtonSize;
	StyleColor hierarchyFooterButtonBG_Hot;
	StyleColor hierarchyFooterButtonBG_Active;
	StyleColor hierarchyFooterButtonBorder_Hot;
	StyleColor hierarchyFooterButtonBorder_Active;
	StyleColor hierarchyFooterButtonIcon;
	StyleColor hierarchyFooterRemoveIcon;
	StyleColor hierarchyFooterDisabledIconColor;
	StyleColor hierarchyLabelDisabled;
	StyleColor hierarchyItemBGDisabled;

	float listBoxItemHeight;
	u32 hierarchyLabelFontSize;
	StyleColor hierarchyItemBG_A;
	StyleColor hierarchyItemBG_B;
	StyleColor hierarchyItemBG_Selected;
	StyleColor hierarchyItemBG_Movable;
	StyleColor hierarchyLabel;
	StyleColor hierarchyToggleNormal;
	StyleColor hierarchyToggleHot;

	StyleColor scrollBarTrackBG;
	StyleColor scrollBarIconNormal;
	StyleColor scrollBarIconHot;
	StyleColor scrollBarHotButtonBg;
	StyleColor scrollGrabberNormal;
	StyleColor scrollGrabberHot;
	float scrollBarSize;
	float scrollIconSize;
	float scrollBarHandleSize;

	StyleColor fps1;
	StyleColor fps2;
	StyleColor fps3;

	StyleColor toggleButtonBorder;
	StyleColor toggleButtonDisabledBorder;
	StyleColor toggleButtonNormal;
	StyleColor toggleButtonHot;
	StyleColor toggleButtonActive;
	StyleColor toggleButtonDisabled;

	StyleColor textAreaBg_Normal;
	StyleColor textAreaBg_Hot;
	StyleColor textAreaBg_Active;
	StyleColor textAreaBg_Disabled;
	StyleColor textAreaBg_AnimatedTint;
	StyleColor textAreaBg_InterpolatedTint;
	StyleColor textAreaBg_DirtyTint;

	StyleColor textAreaFont_Normal;
	StyleColor textAreaFont_Hot;
	StyleColor textAreaFont_Active;
	StyleColor textAreaFont_Disabled;
	StyleColor textAreaFont_AnimatedTint;
	StyleColor textAreaFont_InterpolatedTint;
	StyleColor textAreaFont_DirtyTint;

	StyleColor textAreaOutline_Normal;
	StyleColor textAreaOutline_Hot;
	StyleColor textAreaOutline_Active;
	StyleColor textAreaOutline_Disabled;
	StyleColor textAreaOutline_AnimatedTint;
	StyleColor textAreaOutline_InterpolatedTint;
	StyleColor textAreaOutline_DirtyTint;

	StyleColor memoryPageInUse;
	StyleColor memoryFreePage;

	StyleColor textAreaLabel_Normal;
	StyleColor textAreaLabel_Hot;
	StyleColor textAreaLabel_Active;
	StyleColor textAreaLabel_Disabled;

	StyleColor labelFontColorNormal;
	StyleColor labelFontColorDisabled;

	StyleColor textAreaSelectionColor;
	StyleColor karratColor;

	f32 txtAreaHeight;
	u32 textAreaFontSize;
	u32 textAreaLabelSize;
};

namespace Imgui {
	struct Point {
		float x;
		float y;

		inline Point(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) { }
	};

	struct Rect {
		f32 x;
		f32 y;
		f32 w;
		f32 h;

		inline Rect(f32 _x = 0.0f, f32 _y = 0.0f, f32 _w = 0.0f, f32 _h = 0.0f) :
			x(_x), y(_y), w(_w), h(_h) {}

		inline bool Contains(const Point& point) {
			if (point.x >= x && point.x <= x + w) {
				if (point.y >= y && point.y <= y + h) {
					return true;
				}
			}
			return false;
		}
	};

	inline bool Contains(const Rect& rect, const Point& point) {
		if (point.x >= rect.x && point.x <= rect.x + rect.w) {
			if (point.y >= rect.y && point.y <= rect.y + rect.h) {
				return true;
			}
		}
		return false;
	}

	inline bool Intersects(const Rect& r1, const Rect& r2) {
		return !(r1.x + r1.w < r2.x || r1.y + r1.h < r2.y || r1.x > r2.x + r2.w || r1.y > r2.y + r2.h);
	}

	void Initialize(u32 interfaceFont, u32 widgetFont, u32 labelFont);
	void Shutdown();
	void BeginFrame(float dpi, const struct StyleSheet& style);
	bool EndFrame(u32 screenWidth, u32 screenHeight); // Returns true if processed kbd this frame
	void TickFrame(float deltaTime);

	void VirtualKeyWasPressed();
	void SetFakeShiftToggle(bool val);
	bool GetFakeShiftToggle();
	void SetFakeCapsToggle(bool val);
	bool GetFakeCapsToggle();
	void SetFakeControlToggle(bool val);
	bool GetFakeControlToggle();

	void SetFakeLeftPressed(bool val);
	bool GetFakeLeftPressed();
	void SetFakeRightPressed(bool val);
	bool GetFakeRightPressed();

	float Split(const Rect& screenPos, bool horizontal, float minSplitterHeight, float normalizedDivider);
	Rect SplitFirstArea(const Rect& screenPos, bool horizontal, float splitter);
	Rect SplitSecondArea(const Rect& screenPos, bool horizontal, float splitter);
	
	inline float HSplit(const Rect& screenPos, float minSplitterHeight, float normalizedDivider) {
		return Split(screenPos, true, minSplitterHeight, normalizedDivider);
	}
	inline Rect HSplitFirstArea(const Rect& screenPos, float splitter) {
		return SplitFirstArea(screenPos, true, splitter);
	}
	inline Rect HSplitSecondArea(const Rect& screenPos, float splitter) {
		return SplitSecondArea(screenPos, true, splitter);
	}

	inline float VSplit(const Rect& screenPos, float minSplitterWidth, float normalizedDivider) {
		return Split(screenPos, false, minSplitterWidth, normalizedDivider);
	}
	inline Rect VSplitFirstArea(const Rect& screenPos, float splitter) {
		return SplitFirstArea(screenPos, false, splitter);
	}
	inline Rect VSplitSecondArea(const Rect& screenPos, float splitter) {
		return SplitSecondArea(screenPos, false, splitter);
	}

	struct HierarchyListItemResult { // Note: should be a bitmask
		bool activated;
		bool dragging;
		bool expanded;
	};

	struct DeletableListItemResult {
		bool activated;
		bool deleted;
	};

	HierarchyListItemResult HierarchyListItem(const Rect& screenPos, const Rect& listARea, const char* name, f32 indent, bool evenOrOdd, bool selected, bool expanded, bool leaf);
	DeletableListItemResult DeletableListItem(const Rect& screenPos, const Rect& scrollArea, const char* name, bool evenOrOdd, bool disabled, bool top);
	bool UndoListItem(const Rect& screenPos, const Rect& scrollArea, const char* name, bool evenOrOdd, bool disabled, bool top);
	u32 Header(const Rect& screenPos, const char** options, u32 numOptions, u32 selectedOption);
	
	i32 FileMenu(const Rect& screenPos, const char** options, u32 numOptions, i32 selection, Imgui::Rect* outRect);

	// Will show an overlay next to the pointer. Drawn in EndFrame.
	void SetTooltipIcon(int codePoint);

	bool ToggleIcon(const Rect& screenPos, u32 trueIcon, u32 falseIcon, bool state, const char* hoverLabel);

	bool ToggleButton(const Rect& screenPos, u32 trueIcon, u32 falseIcon, bool state, bool disabled, const char* hoverLabel, const StyleColor& tint, StyleColor* iconTint = 0);
	bool FooterButton(const Rect& screenPos, u32 codePoint, const char* caption, const StyleColor& color);
	bool ClickArea(const Rect& screenPos);
	
	struct HoldAreaDetails {
		bool wasActive;
		bool active;
		bool activated;
		vec2 mouseDown;
		float rememberF32OnClick;
		vec2 rememberVec2OnClick;
	};
	bool HoldArea(const Rect& screenPos, HoldAreaDetails* optDetails = 0, float* rememberOnClick = 0, const vec2* rememberVecOnClick = 0);
	bool SidebarButton(const Rect& screenPos, u32 codePoint, const char* caption, bool isActive, vec3* iconOverride = 0);

	void Icon(const Rect& screenPos, u32 iconSize, u32 codePoint, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
	i32 Dummy();

	float FloatSlider(const Rect& screeenPos, float value);

	float ScrollBar(const Rect& screeenPos, float value, bool horizontal, float grabberHeight = 15.0f, bool handleScroll = false);
	bool HandleScroll(const Rect& scrollArea);

	inline float Slider(const Rect& screenPos, float value) {
		return ScrollBar(screenPos, value, true, 3.0f, false);
	}

	inline float HScroll(const Rect& screenPos, float value, float grabberHeight = 15.0f, bool handleScroll = false) {
		return ScrollBar(screenPos, value, true, grabberHeight, handleScroll);
	}
	inline float VScroll(const Rect& screenPos, float value, float grabberHeight = 15.0f, bool handleScroll = false) {
		return ScrollBar(screenPos, value, false, grabberHeight, handleScroll);
	}

	Draw2D::Size Label(const Rect& screenPos, const char* text, bool disabled, bool measure = false, bool small = false, StyleColor* c = 0);
	Draw2D::Size MeasureLabel(const Rect& screenPos, const char* text, bool small = false);

	vec3 ImageBlock(const Rect& screenPos, const char* caption, u32 image, const Rect& sourceRectPixels, bool disabled);

	void ClearActiveTextAreas();

	i32 BeginComboBox(const Rect& screenPos, const char* name, u32 numOptions, i32 selectedOption, const char* label, bool reversed_like_backwards, bool disabled);
	void PushComboBoxItem(const char* itemName);
	void EndComboBox();

	i32 Timeline(const Rect& screenPos, u32 numFrames, i32 selectedFrame, float scroll);

	i32 BeginModalPopup(const Rect& screenPos, const Rect& activationArea, u32 numItems, bool reversed = false);
	void PushModalPopupItem(const char* item);
	bool EndModalPopup();

	// hsv ColorPicker(i32 id, const Rect& screenPos, const hsv& inColor); // This is private
	hsv ColorPicker(const Rect& screenPos, const hsv& inColor); // Not modal
	hsv ColorPickerButton(const Rect& screenPos, const hsv& inColor, bool disabled, const StyleColor& tint); // Picker is modal

	Point GetPointer();
	Point GetPrevPointer();
	Point GetPointerDelta();
	bool GetPulse();
	bool PointerReleased();
	bool PointerPressed();

	// Returns the same pointer as the second argument (inputText) when the control is inactive or the text is being edited
	// returns a pointer to a new string when the edited text is commited. The returned string is valid until Imgui::EndFrame
	const char* TextArea(const Rect& screenPos, const char* inputText, const char* label, 
		bool disabled, bool fraction, bool integer, bool animated, bool interpolated, bool dirty);

}

#endif