#include "imgui.h"
#include "../platform/math.h"
#include "../platform/window.h"
#include "../framework/vector.h"
#include "../framework/draw2d.h"
#include "../debt/stb_sprintf.h"

#define IMGUI_ACTIVE_INPUT_NONE 0
#define IMGUI_ACTIVE_INPUT_MOUSE 1
#define IMGUI_ACTIVE_INPUT_TOUCH1 2
#define IMGUI_ACTIVE_INPUT_TOUCH2 3
#define IMGUI_ACTIVE_INPUT_TOUCH3 4
#define IMGUI_ACTIVE_INPUT_TOUCH4 5
#define IMGUI_ACTIVE_INPUT_TOUCH5 6

#define IMGUI_WIDGET_BLOCK_SIZE 256

namespace Imgui {
	void SetTooltipLabel(const char* label);
	namespace Internal {
#if PLATFORM_DEBUG
		struct RectBlock
		{
			Imgui::Rect rectangles[IMGUI_WIDGET_BLOCK_SIZE];
			RectBlock* next;
		};
#endif
		enum class TextAreaState {
			None = 0,
			Activated,
			Idle,
			Shutdown
		};

		struct TextAreaInstance {
			i32 widgetId;
			char* buffer;
			u32 length;
			u32 capacity;
			i32 karratIndex;
			i32 selection;
			TextAreaState state;

			inline void Reserve(u32 cap) {
				u32 newCap = capacity;
				if (capacity == 0) {
					newCap = 16;
				}
				if (newCap < cap) {
					newCap = cap;
				}

				if (capacity != newCap) {
					buffer = (char*)MemRealloc(buffer, sizeof(char) * newCap);
					capacity = newCap;
				}
			}

			inline void Set(const char* newText, u32 newTextLen) {
				if (newTextLen + 1 >= capacity) {
					Reserve(newTextLen + 1);
				}

				if (newTextLen == 0) {
					buffer[0] = 0;
				}
				else {
					MemCopy(buffer, newText, newTextLen); // This includes the null terminator (i think)
					buffer[newTextLen] = 0;
				}
				length = newTextLen;
			}
		};
		
		struct PopupMenuInfo {
			i32 id;
			Rect screenPos;
			Vector<const char*> items;
			u32 itemCount;
			i32 hot;
			bool reversed;
		};

		struct State {
			const StyleSheet* style;

			i32 hotControl;
			i32 activeControl;
			i32 modalControl;
			i32 lastBeingModalPopupContext;
			i32 lastBeingModalPopupActivation;

			u32 widgetFont;
			u32 interfaceFont;
			u32 labelFont;

			bool ateKeyboard;
			bool virtualKeyPressed;
			bool fakeShift;
			bool fakeCtrl;
			bool fakeCaps;
			bool fakeLeftPressed;
			bool fakeRightPressed;

			i32 positiveIdGenerator;
			i32 negativeIdGenerator;

			i32 grabKeyboardOnOrAfter;

			f32 dpi;

			i32 lastHotControl;
			f32 hotControlTimer;

			i32 lastActiveControl;
			f32 activeControlTimer;
			f32 lastTextDoubleClickTimer;

			f32 pulseTimer;
			bool pulse;

			f32 longPulseTimer;
			bool longPulse;

			f32 blinkTimer;
			bool blink;

			i32 tooltipIcon;
			const char* tooltipLabel;
			i32 activeInputMethod;

			TextAreaInstance textAreaA;
			TextAreaInstance textAreaB;

			f32 timeSinceLastRelease;

			hsv lastSelectedColor;
			bool colorPickerActive;
			Rect colorPickerArea;

			bool debugOverlayEnabled;

			Vector<PopupMenuInfo> modalPopups;
			u32 numPopupMenus;
			u32 maxPopupMenus;
			i32 lastModalId;
#if PLATFORM_DEBUG
			RectBlock positiveWidgets;
			RectBlock negativeWidgets;
#endif
		private:
#if PLATFORM_DEBUG

			inline i32 _PrivateGetWidgetIndex(i32 widgetId) {
				i32 widgetIndex = MathAbsI(widgetId);
				u32 widgetBlock = widgetIndex / IMGUI_WIDGET_BLOCK_SIZE;
				widgetIndex -= widgetBlock * IMGUI_WIDGET_BLOCK_SIZE;
				PlatformAssert(widgetIndex >= 0, __LOCATION__);
				PlatformAssert(widgetIndex < IMGUI_WIDGET_BLOCK_SIZE, __LOCATION__);
				return widgetIndex;
			}

			RectBlock* _PrivateGetWidgetBlock(i32 widgetId) {
				RectBlock* block = &positiveWidgets;
				if (widgetId < 0) {
					block = &negativeWidgets;
				}

				i32 widgetIndex = MathAbsI(widgetId);
				u32 widgetBlock = widgetIndex / IMGUI_WIDGET_BLOCK_SIZE;
				widgetIndex -= widgetBlock * IMGUI_WIDGET_BLOCK_SIZE;
				PlatformAssert(widgetIndex >= 0, __LOCATION__);
				PlatformAssert(widgetIndex < IMGUI_WIDGET_BLOCK_SIZE, __LOCATION__);

				for (u32 i = 0; i < widgetBlock; ++i) {
					if (block->next == 0) {
						block->next = (RectBlock*)MemAlloc(sizeof(RectBlock));
					}
					block = block->next;
				}

				return block;
			}
#endif
			inline i32 _PrivateGenerateWidget(const Rect& widgetRect, i32 widgetId) {
#if PLATFORM_DEBUG
				RectBlock* block = _PrivateGetWidgetBlock(widgetId);
				i32 index = _PrivateGetWidgetIndex(widgetId);
				block->rectangles[index] = widgetRect;
#endif
				return widgetId;
			}
		public:
			inline Rect Debug_GetWidgetRect(i32 widgetId) {
#if PLATFORM_DEBUG
				RectBlock* block = _PrivateGetWidgetBlock(widgetId);
				i32 index = _PrivateGetWidgetIndex(widgetId);
				return block->rectangles[index];
#else
				return Rect(0, 0, 50, 50);
#endif
			}

			inline i32 NegativeId(const Rect& widgetRect) {
				return _PrivateGenerateWidget(widgetRect, --negativeIdGenerator);
			}

			inline i32 PositiveId(const Rect& widgetRect) {
				return _PrivateGenerateWidget(widgetRect, ++positiveIdGenerator);
			}

			
			// A "pointer" in this context is the mouse or a touch. Only one thing can be
			// the active pointer at once. IE, if the active pointer is the mouse, then
			// touch events wont be dispatched. But if the active pointer is a touch,
			// then mouse events wont be dispatched. Only one active touch at a time.
			inline f32 GetPointerX() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchGetX(touchIndex);

				}
				return MouseGetX();
			}

			inline f32 GetPrevPointerX() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchGetPrevX(touchIndex);

				}
				return MousePrevX();
			}

			inline f32 GetPointerY() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchGetY(touchIndex);

				}
				return MouseGetY();
			}

			inline f32 GetPrevPointerY() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchGetPrevY(touchIndex);

				}
				return MousePrevY();
			}

			inline f32 GetPointerDeltaX() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchDeltaX(touchIndex);

				}
				return MouseDeltaX();
			}

			inline f32 GetPointerDeltaY() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchDeltaY(touchIndex);

				}
				return MouseDeltaY();
			}

			inline bool GetPointerPressed() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchPressed(touchIndex);

				}
				return MousePressed(MouseButtonLeft);
			}

			inline bool GetPointerReleased() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return TouchReleased(touchIndex);
				}
				bool debug = MouseReleased(MouseButtonLeft);
				return debug;
			}

			inline bool GetPointerUp() {
				if (activeInputMethod != IMGUI_ACTIVE_INPUT_NONE && activeInputMethod != IMGUI_ACTIVE_INPUT_MOUSE) {
					int touchIndex = activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
					PlatformAssert(touchIndex >= 0 && touchIndex <= 4, __LOCATION__);
					return !TouchIsActive(touchIndex);

				}
				return MouseUp(MouseButtonLeft);
			}
		};

		State* gState;

		inline u32 StrLen(const char* str) {
			if (str == 0) {
				return 0;
			}
			u32 len = 0;
			for (const char* s = str; *s != 0; ++s, ++len);
			return len;
		}


		// Returns the number of elements shifted
		inline i32 RemoveRangeFromString(char* stringToModify, u32 stringLen, i32 startIndex, i32 endIndex) {
			if (startIndex > endIndex) {
				i32 tmp = startIndex;
				startIndex = endIndex;
				endIndex = tmp;
			}

			int n = endIndex - startIndex;
			int p = startIndex;
			
			if (p + n >= stringLen + 1) { // +1 to clear the string (it's cool, i own the memory for sure)
				return 0;
			}

			int cut = 0;
			for (n += p; stringToModify[n] != 0; cut++) {
				stringToModify[p++] = stringToModify[n++];
			}
			stringToModify[p] = 0;
			
			return cut;
		}

		inline bool MakeHotOrActive(i32 widgetId, const Rect& screenPos, const vec2& mouse, const char* tooltip) {
			// Make hot
			if (mouse.x >= screenPos.x && mouse.x <= screenPos.x + screenPos.w) {
				if (mouse.y >= screenPos.y && mouse.y <= screenPos.y + screenPos.h) {
					if (Internal::gState->activeControl == widgetId || Internal::gState->activeControl == 0) {
						Internal::gState->hotControl = widgetId;

					}
				}
			}

			// Make active
			if (Internal::gState->hotControl == widgetId && Internal::gState->activeControl == 0) {
				if (Internal::gState->GetPointerPressed()) {
					Internal::gState->activeControl = widgetId;
					Internal::gState->hotControlTimer = 0.0f;
				}
				else {
					if (Internal::gState->hotControlTimer > IMGUI_TOOLTIP_MIN && Internal::gState->hotControlTimer < IMGUI_TOOLTIP_MAX) {
						if (tooltip != 0) {
							Imgui::SetTooltipLabel(tooltip);
						}
					}
				}
			}

			// Activate return
			if (Internal::gState->activeControl == widgetId && Internal::gState->hotControl == widgetId) {
				if (Internal::gState->GetPointerReleased()) {
					return true;
				}
			}

			return false;
		}
	
		inline i32 GetStringIndexUnderMouse(const char* string, i32 inputTextLength, const Rect& screenPos, const Draw2D::Size& inputTextSize, const vec2& mouse) {
			int index = -1;
			float string_x = screenPos.x + screenPos.w - 5 - inputTextSize.w;
			float string_y = screenPos.y + 2;
			vec2 relativePoint(mouse.x - string_x, 0); // ignore y


			if (relativePoint.y >= 0 && relativePoint.y <= inputTextSize.h) {
				if (relativePoint.x < 0) {
					index = 0;
				}
				else if (relativePoint.x > inputTextSize.w) {
					index = inputTextLength;
				}
				else {
					index = Draw2D::FindStringIndex(Internal::gState->interfaceFont, Internal::gState->style->textAreaFontSize, string, relativePoint);
				}
			}

			if (index < 0) {
				index = 0;
			}
			else if (index >= inputTextLength) {
				index = (i32)inputTextLength;
			}

			return index;
		}
	
		// You are responsible for calling MemFree here
		inline char* GetSubString(const char* string, i32 startIndex, i32 endIndex) {
			if (endIndex < startIndex) {
				i32 tmp = endIndex;
				endIndex = startIndex;
				startIndex = tmp;
			}

			i32 len = endIndex - startIndex;
			if (len <= 0) {
				return 0;
			}

			if (string == 0) {
				return 0;
			}

			if (startIndex < 0 || endIndex < 0) {
				return 0;
			}

			char* buffer = (char*)MemAlloc(len + 1);
			MemClear(buffer, len + 1);
			for (i32 i = 0; i < len; ++i) {
				buffer[i] = string[startIndex + i];
			}
			buffer[len] = '\0';

			return buffer;
		}
	}

	void VirtualKeyWasPressed() {
		Internal::gState->virtualKeyPressed = true;
	}

	void SetFakeShiftToggle(bool val) {
		Internal::gState->fakeShift = val;
	}

	bool GetFakeShiftToggle() {
		return Internal::gState->fakeShift;
	}

	void SetFakeLeftPressed(bool val) {
		Internal::gState->fakeLeftPressed = val;
	}
	bool GetFakeLeftPressed() {
		return Internal::gState->fakeLeftPressed;
	}
	void SetFakeRightPressed(bool val) {
		Internal::gState->fakeRightPressed = val;
	}
	bool GetFakeRightPressed() {
		return Internal::gState->fakeRightPressed;
	}

	void SetFakeCapsToggle(bool val) {
		Internal::gState->fakeCaps = val;
	}

	bool GetFakeCapsToggle() {
		return Internal::gState->fakeCaps;
	}

	void SetFakeControlToggle(bool val) {
		Internal::gState->fakeCtrl = val;
	}

	bool GetFakeControlToggle() {
		return Internal::gState->fakeCtrl;
	}
	
	bool UndoListItem(const Rect& screenPos, const Rect& scrollArea, const char* name, bool evenOrOdd, bool disabled, bool top) {
		const StyleSheet* s = Internal::gState->style;
		i32 widgetId = Internal::gState->NegativeId(screenPos);

		f32 dpi = Internal::gState->dpi;
		f32 mouseX = Internal::gState->GetPointerX() / dpi;
		f32 mouseY = Internal::gState->GetPointerY() / dpi;
		bool activated = false;

		bool mouseInScrollArea = mouseX >= scrollArea.x && mouseX <= scrollArea.x + scrollArea.w && mouseY >= scrollArea.y && mouseY <= scrollArea.y + scrollArea.h;
		if (mouseInScrollArea) {
			bool mouseInControlArea = mouseX >= screenPos.x && mouseX <= screenPos.x + screenPos.w && mouseY >= screenPos.y && mouseY <= screenPos.y + screenPos.h;
			if (mouseInControlArea) {
				if (Internal::MakeHotOrActive(widgetId, screenPos, vec2(mouseX, mouseY), 0)) {
					activated = true;
				}
			}
		}

		StyleColor bg = s->hierarchyItemBG_A;
		StyleColor txt = s->hierarchyLabel;
		if (evenOrOdd) {
			bg = s->hierarchyItemBG_B;
		}

		if (top) {
			bg = s->hierarchyItemBG_Selected;
		}
		else if (disabled) {
			txt = s->hierarchyLabelDisabled;
			bg = s->hierarchyItemBGDisabled;
		}

		if (Intersects(screenPos, scrollArea)) {
			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, bg.r, bg.g, bg.b);

			Draw2D::DrawString(Internal::gState->interfaceFont, s->hierarchyLabelFontSize,
				screenPos.x + 5, screenPos.y + s->hierarchyLabelFontSize, name, txt.r, txt.g, txt.b);
		}

		return activated;
	}

	DeletableListItemResult DeletableListItem(const Rect& screenPos, const Rect& scrollArea, const char* name, bool evenOrOdd, bool disabled, bool top) {
		const StyleSheet* s = Internal::gState->style;

		Rect listItemArea = screenPos;
		listItemArea.x += listItemArea.h;
		listItemArea.w -= listItemArea.h;
		i32 widgetId = Internal::gState->NegativeId(listItemArea);

		Rect deletedIconArea = screenPos;
		deletedIconArea.w = listItemArea.h;
		Rect deletedHitArea = deletedIconArea;
		deletedHitArea.x += 3;
		deletedHitArea.y += 2;
		deletedHitArea.h -= 6;
		deletedHitArea.w -= 7;
		i32 deleteIconID = Internal::gState->NegativeId(deletedIconArea);

		f32 dpi = Internal::gState->dpi;
		f32 mouseX = Internal::gState->GetPointerX() / dpi;
		f32 mouseY = Internal::gState->GetPointerY() / dpi;
		DeletableListItemResult result = { 0 };;

		bool mouseInScrollArea = mouseX >= scrollArea.x && mouseX <= scrollArea.x + scrollArea.w && mouseY >= scrollArea.y && mouseY <= scrollArea.y + scrollArea.h;
		if (mouseInScrollArea) {
			bool mouseInControlArea = mouseX >= screenPos.x && mouseX <= screenPos.x + screenPos.w && mouseY >= screenPos.y && mouseY <= screenPos.y + screenPos.h;
			if (mouseInControlArea) {
				if (Internal::MakeHotOrActive(widgetId, listItemArea, vec2(mouseX, mouseY), 0)) {
					result.activated = true;
				}
				if (Internal::MakeHotOrActive(deleteIconID, deletedHitArea, vec2(mouseX, mouseY), 0)) {
					result.deleted = true;
				}
			}
		}

		StyleColor bg = s->hierarchyItemBG_A;
		StyleColor txt = s->hierarchyLabel;
		if (evenOrOdd) {
			bg = s->hierarchyItemBG_B;
		}

		if (top) {
			bg = s->hierarchyItemBG_Selected;
		}
		else if (disabled) {
			txt = s->hierarchyLabelDisabled;
			bg = s->hierarchyItemBGDisabled;
		}

		if (Intersects(screenPos, scrollArea)) {
			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, bg.r, bg.g, bg.b);

			StyleColor iconColor = s->textAreaFont_Normal;
			if (Internal::gState->activeControl == deleteIconID) {
				iconColor = s->textAreaFont_Active;
			}
			else if (Internal::gState->hotControl == deleteIconID) {
				iconColor = s->textAreaFont_Hot;
			}

			//Draw2D::DrawRect(deletedHitArea.x, deletedHitArea.y, deletedHitArea.w, deletedHitArea.h, 1, 0, 0);
			Draw2D::DrawCodePoint(Internal::gState->widgetFont, s->hierarchyLabelFontSize,
				deletedIconArea.x + 2, deletedIconArea.y + s->hierarchyLabelFontSize + 3, 
				IMGUI_ICON_CODEPOINT_TRASHCAN, iconColor.r, iconColor.g, iconColor.b);

			Draw2D::DrawString(Internal::gState->interfaceFont, s->hierarchyLabelFontSize,
				listItemArea.x, listItemArea.y + s->hierarchyLabelFontSize, name, txt.r, txt.g, txt.b);
		}

		return result;
	}

	HierarchyListItemResult HierarchyListItem(const Rect& screenPos, const Rect& listARea, const char* name, f32 indent, bool evenOrOdd, bool selected, bool expanded, bool leaf) {
		HierarchyListItemResult result;
		result.expanded = expanded;
		
		const StyleSheet* s = Internal::gState->style;
		i32 widgetId = Internal::gState->NegativeId(screenPos);

		f32 dpi = Internal::gState->dpi;
		f32 mouseX = Internal::gState->GetPointerX() / dpi; 
		f32 mouseY = Internal::gState->GetPointerY() / dpi; 
		
		bool activated = false;
		
		f32 toggleX = screenPos.x + indent;
		f32 toggleY = screenPos.y + s->hierarchyLabelFontSize + 3;
		Rect toggleArea(toggleX, screenPos.y, s->hierarchyLabelFontSize + 3, screenPos.h);
		i32 toggleId = Internal::gState->NegativeId(toggleArea);

		bool mouseInListArea = mouseX >= listARea.x && mouseX <= listARea.x + listARea.w && mouseY >= listARea.y && mouseY <= listARea.y + listARea.h;
		
		if (mouseInListArea) {
			bool mouseInControlArea = mouseX >= screenPos.x && mouseX <= screenPos.x + screenPos.w && mouseY >= screenPos.y && mouseY <= screenPos.y + screenPos.h;
			bool mouseInToggleArea = mouseX >= toggleArea.x && mouseX <= toggleArea.x + toggleArea.w && mouseY >= toggleArea.y && mouseY <= toggleArea.y + toggleArea.h;
			
			bool wasHotOrActive = Internal::gState->activeControl == widgetId || Internal::gState->hotControl == widgetId;

			if ((!wasHotOrActive) && mouseInToggleArea) {
				if (Internal::MakeHotOrActive(toggleId, toggleArea, vec2(mouseX, mouseY), 0)) {
					result.expanded = !result.expanded;
				}
			}
			else if (mouseInControlArea) {
				if (Internal::MakeHotOrActive(widgetId, screenPos, vec2(mouseX, mouseY), 0)) {
					activated = true;
				}
			}
		}

		StyleColor bg = s->hierarchyItemBG_A;
		if (evenOrOdd) {
			bg = s->hierarchyItemBG_B;
		}

		if (selected) {
			if (Internal::gState->activeControl == widgetId) {
				bg = s->hierarchyItemBG_Movable;
			}
			else {
				bg = s->hierarchyItemBG_Selected;
			}
		}
		else {
			if (Internal::gState->activeControl == widgetId) {
				bg = s->hierarchyItemBG_Movable;
			}
		}

		if (Intersects(screenPos, listARea)) {
			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, bg.r, bg.g, bg.b);
			
			//Draw2D::DrawRect(toggleArea.x, toggleArea.y, toggleArea.w, toggleArea.h, 1, 0, 0);

			if (!leaf) {
				StyleColor toggleColor = s->hierarchyToggleNormal;
				if (Internal::gState->hotControl == toggleId || Internal::gState->activeControl == toggleId) {
					toggleColor = s->hierarchyToggleHot;
				}

				Draw2D::DrawCodePoint(Internal::gState->widgetFont, s->hierarchyLabelFontSize - 1, toggleX, toggleY,
					expanded ? IMGUI_ICON_CODEPOINT_EXPANDED : IMGUI_ICON_CODEPOINT_COLLAPSED,
					toggleColor.r, toggleColor.g, toggleColor.b);
			}
			
			Draw2D::DrawString(Internal::gState->interfaceFont, s->hierarchyLabelFontSize,
				toggleX + toggleArea.w, toggleY - 3 + 1, name,
				s->hierarchyLabel.r, s->hierarchyLabel.g, s->hierarchyLabel.b);
		}

		result.activated = activated;
		result.dragging = Internal::gState->activeControl == widgetId;
		return result;
	}

	float Split(const Rect& screenPos, bool horizontal, float minSplitterSize, float normalizedDivider) {
		if (normalizedDivider < 0.0f) {
			normalizedDivider = 0.0f;
		}
		else if (normalizedDivider > 1.0f) {
			normalizedDivider = 1.0f;
		}

		f32 x = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 y = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouseVector(x / dpi, y / dpi);

		Rect grabberArea;
		if (!horizontal) {
			float divider = screenPos.w * normalizedDivider;
			grabberArea.x = (screenPos.x + divider) - (IMGUI_SPLITTER_GRABBABLE_WIDTH / 2.0f);
			grabberArea.w = IMGUI_SPLITTER_GRABBABLE_WIDTH;
			grabberArea.y = screenPos.y;
			grabberArea.h = screenPos.h;

		}
		else {
			float divider = screenPos.h * normalizedDivider;

			grabberArea.x = screenPos.x;
			grabberArea.w = screenPos.w;
			grabberArea.y = (screenPos.y + divider) - (IMGUI_SPLITTER_GRABBABLE_WIDTH / 2.0f);
			grabberArea.h = IMGUI_SPLITTER_GRABBABLE_WIDTH;
		}

		i32 widgetId = Internal::gState->NegativeId(grabberArea);
		Internal::MakeHotOrActive(widgetId, grabberArea, mouseVector, 0);

		if (!horizontal) {
			// Move the splitter to normalizedDivider
			float targetX = normalizedDivider * screenPos.w + screenPos.x;
			if (Internal::gState->activeControl == widgetId) {
				targetX = x / dpi; // Track the mouse
			}

			{ // Constrain splitter to control
				if (targetX < screenPos.x) {
					targetX = screenPos.x;
				}
				if (targetX > screenPos.x + screenPos.w) {
					targetX = screenPos.x + screenPos.w;
				}

				// Respect min sizes (adjust right first to keep left always visible?
				if (screenPos.w - (targetX - screenPos.x) < minSplitterSize) { // Right side is too small
					targetX = screenPos.x + screenPos.w - minSplitterSize;
				}
				if (targetX - screenPos.x < minSplitterSize) { // Left side is too small
					targetX = screenPos.x + minSplitterSize;
				}

				float normalizedTarget = (targetX - screenPos.x) / screenPos.w;
				normalizedDivider = MathMaxF(MathMinF(normalizedTarget, 1.0f), 0.0f);
			}
		}
		else {
			// Move the splitter to normalizedDivider
			float targetY = (normalizedDivider) * screenPos.h + screenPos.y;
			if (Internal::gState->activeControl == widgetId) {
				targetY = y / dpi; // Track the mouse
			}

			{ // Constrain splitter to control
				if (targetY < screenPos.y) {
					targetY = screenPos.y;
				}
				if (targetY > screenPos.y + screenPos.h) {
					targetY = screenPos.y + screenPos.h;
				}

				// Respect min sizes (adjust right first to keep left always visible?
				if (screenPos.h - (targetY - screenPos.y) < minSplitterSize) { // Top side is too small
					targetY = screenPos.y + screenPos.h - minSplitterSize;
				}
				if (targetY - screenPos.y < minSplitterSize) { // Bottom side is too small
					targetY = screenPos.y + minSplitterSize;
				}

				float normalizedTarget = (targetY - screenPos.y) / screenPos.h;
				normalizedDivider = MathMaxF(MathMinF(normalizedTarget, 1.0f), 0.0f);
			}
		}
		
		{ // Drawing
			float colorA[3] = {
				Internal::gState->style->dividerAColor.r, Internal::gState->style->dividerAColor.g, Internal::gState->style->dividerAColor.b
			};
			float colorB[3] = {
				Internal::gState->style->dividerBColor.r, Internal::gState->style->dividerBColor.g, Internal::gState->style->dividerBColor.b
			};
			if (Internal::gState->activeControl == widgetId) {
				colorA[0] *= Internal::gState->style->dividerActiveTint;
				colorA[1] *= Internal::gState->style->dividerActiveTint;
				colorA[2] *= Internal::gState->style->dividerActiveTint;
				colorB[0] *= Internal::gState->style->dividerActiveTint;
				colorB[1] *= Internal::gState->style->dividerActiveTint;
				colorB[2] *= Internal::gState->style->dividerActiveTint;
				SetTooltipIcon(horizontal ? IMGUI_ICON_CODEPOINT_RESIZE_VERTICAL : IMGUI_ICON_CODEPOINT_RESIZE_HORIZONTAL);
			}
			else if (Internal::gState->hotControl == widgetId) {
				colorA[0] *= Internal::gState->style->dividerHotTint;
				colorA[1] *= Internal::gState->style->dividerHotTint;
				colorA[2] *= Internal::gState->style->dividerHotTint;
				colorB[0] *= Internal::gState->style->dividerHotTint;
				colorB[1] *= Internal::gState->style->dividerHotTint;
				colorB[2] *= Internal::gState->style->dividerHotTint;
				if (Internal::gState->activeControl == 0) {
					SetTooltipIcon(horizontal ? IMGUI_ICON_CODEPOINT_RESIZE_VERTICAL : IMGUI_ICON_CODEPOINT_RESIZE_HORIZONTAL);
				}
			}

			Rect splitterArea = SplitFirstArea(screenPos, horizontal, normalizedDivider);

			Rect line(
				splitterArea.x + splitterArea.w, splitterArea.y,
				2, splitterArea.h);

			if (horizontal) {
				line.x = splitterArea.x;
				line.y = splitterArea.y + splitterArea.h;
				line.w = splitterArea.w;
				line.h = 2;
			}

			Draw2D::DrawRect(line.x, line.y, line.w, line.h, colorA[0], colorA[1], colorA[2]);

			if (horizontal) {
				line.y += 1;
				line.h -= 1;
			}
			else {
				line.x += 1;
				line.w -= 1;
			}

			Draw2D::DrawRect(line.x, line.y, line.w, line.h, colorB[0], colorB[1], colorB[2]);
		}

		return normalizedDivider;
	}

	Rect SplitFirstArea(const Rect& screenPos, bool horizontal, float splitter) {
		Rect result = screenPos;
		if (splitter < 0.0f) { splitter = 0.0f; }
		if (splitter > 1.0f) { splitter = 1.0f; }

		if (!horizontal) {
			result.x = MathFloor(screenPos.x);
			result.w = MathFloor(screenPos.w * splitter);
			result.w -= 1; // Padding for grabber
		}
		else {
			result.y = MathFloor(screenPos.y);
			result.h = MathFloor(screenPos.h * splitter);
			result.h -= 1; // Padding for for grabber
		}

		return result;
	}

	Rect SplitSecondArea(const Rect& screenPos, bool horizontal, float splitter) {
		Rect result = screenPos;
		if (splitter < 0.0f) { splitter = 0.0f; }
		if (splitter > 1.0f) { splitter = 1.0f; }
		Rect firstArea = SplitFirstArea(screenPos, horizontal, splitter);

		if (!horizontal) {
			result.x = firstArea.x + firstArea.w;
			result.w = MathMaxF(screenPos.w - firstArea.w, 0.0f);

			// For grabber
			result.x += 2;
			result.w -= 2;
		}
		else {
			result.y = firstArea.y + firstArea.h;
			result.h = MathMaxF(screenPos.h - firstArea.h, 0.0f);

			// For grabber
			result.y += 2;
			result.h -= 2;
		}

		return result;
	}

	Point GetPointerDelta() {
		return Point(Internal::gState->GetPointerDeltaX() / Internal::gState->dpi, Internal::gState->GetPointerDeltaY() / Internal::gState->dpi);
	}

	Point GetPointer() {
		return Point(Internal::gState->GetPointerX() / Internal::gState->dpi, Internal::gState->GetPointerY() / Internal::gState->dpi);
	}

	Point GetPrevPointer() {
		return Point(Internal::gState->GetPrevPointerX() / Internal::gState->dpi, Internal::gState->GetPrevPointerY() / Internal::gState->dpi);
	}

	bool PointerReleased() {
		return Internal::gState->GetPointerReleased();
	}

	bool PointerPressed() {
		return Internal::gState->GetPointerPressed();
	}

	bool GetPulse() {
		return Internal::gState->pulse;
	}

	vec3 ImageBlock(const Rect& screenPos, const char* caption, u32 image, const Rect& sourceRect, bool disabled) {
		const StyleSheet* s = Internal::gState->style;

		StyleColor outline = s->imageBlockOutlineColor;
		StyleColor background = s->imageBlockBackgroundColor;
		if (disabled) {
			outline = s->toggleButtonDisabledBorder;
			background = s->toggleButtonDisabled;
		}

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, outline.r, outline.g, outline.b);
		Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2, background.r, background.g, background.b);

		if (caption != 0) {
			Draw2D::DrawString(Internal::gState->labelFont, s->textAreaLabelSize, screenPos.x + 3, screenPos.y, caption, s->imageBlockLabelColor.r, s->imageBlockLabelColor.g, s->imageBlockLabelColor.b);
		}

		float scale = 0.0f;
		float x_offset = 0.0f;
		float y_offset = 0.0f;
		if (image != 0) {
			if (sourceRect.w > sourceRect.h) {
				scale = (screenPos.w - 4) / sourceRect.w;
				float h = (screenPos.h - 4) - (sourceRect.h * scale);
				y_offset += h / 2.0f;
			}
			else if (sourceRect.h > sourceRect.w) {
				scale = (screenPos.h - 4) / sourceRect.h;
				float w = (screenPos.w - 4) - (sourceRect.w * scale);
				x_offset += w / 2.0f;
			}
			else {
				scale = (screenPos.h - 4) / sourceRect.h;
			}

			float x = screenPos.x + 2;
			float y = screenPos.y + 2;
			float w = sourceRect.w * scale;
			float h = sourceRect.h * scale;

			Draw2D::DrawImage(image, x + x_offset, y + y_offset, w, h, sourceRect.x, sourceRect.y, sourceRect.w, sourceRect.h);
		}
		else {
			Draw2D::DrawString(Internal::gState->labelFont, s->textAreaLabelSize, 
				screenPos.x + (screenPos.w / 2.0f) - (48.0f / 2.0f), 
				screenPos.y + (screenPos.h / 2.0f) , 
				"No image", s->imageBlockLabelColor.r, s->imageBlockLabelColor.g, s->imageBlockLabelColor.b
			);
		}

		return vec3(x_offset, y_offset, scale);
	}

	Draw2D::Size MeasureLabel(const Rect& screenPos, const char* text, bool samll) {
		return Draw2D::MeasureString(samll ? Internal::gState->labelFont : Internal::gState->interfaceFont, screenPos.h, text);
	}

	Draw2D::Size Label(const Rect& screenPos, const char* text, bool disabled, bool measure, bool samll, StyleColor* _c) {
		Draw2D::Size result(0, 0);

		const StyleSheet* s = Internal::gState->style;
		i32 fontSize = screenPos.h;

		StyleColor c = s->labelFontColorNormal;
		if (disabled) {
			c = s->labelFontColorDisabled;
		}
		if (_c != 0) {
			c = *_c;
		}

		u32 pixelHeight = screenPos.h;
		
		Draw2D::DrawString(samll? Internal::gState->labelFont : Internal::gState->interfaceFont, pixelHeight, screenPos.x, screenPos.y + screenPos.h, text, c.r, c.g, c.b);
	
		if (measure) {
			result =  Draw2D::MeasureString(samll ? Internal::gState->labelFont : Internal::gState->interfaceFont, pixelHeight, text);
		}

		return result;
	}

	bool IsModal(i32 id) {
		return Internal::gState->modalControl;
	}

	i32 FileMenu(const Rect& screenPos, const char** options, u32 numOptions, i32 selection, Imgui::Rect* outRects) {
		const StyleSheet* s = Internal::gState->style;
		PlatformAssert(numOptions <= 8, __LOCATION__);
		i32 widgetIds[8] = { 0 };

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, s->menuBarBg.r, s->menuBarBg.g, s->menuBarBg.b);
		Draw2D::DrawRect(screenPos.x, i32(screenPos.y + screenPos.h) - 2, screenPos.w, 1, s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

		float mouseX = Internal::gState->GetPointerX() / Internal::gState->dpi;
		float mouseY = Internal::gState->GetPointerY() / Internal::gState->dpi;
		vec2 mouse(mouseX, mouseY);

		Rect Buttons[8] = { 0 };
		Rect karrat = screenPos;
		karrat.x += 10;
		karrat.y += 5;
		karrat.h -= 10;

		StyleColor fontColor = s->textAreaFont_Active;
		StyleColor hotBg = s->textAreaBg_Hot;
		u32 height = 14;

		int hotIndex = -1;

		for (u32 i = 0; i < numOptions; ++i) {
			karrat.w = Draw2D::MeasureString(Internal::gState->interfaceFont, height, options[i]).w + 5;
			widgetIds[i] = Internal::gState->NegativeId(karrat);
			if (outRects != 0) {
				outRects[i] = karrat;
			}

			Internal::MakeHotOrActive(widgetIds[i], karrat, mouse, 0);
			if (widgetIds[i] == Internal::gState->hotControl || widgetIds[i] == Internal::gState->activeControl || selection == i) {
				if (Imgui::PointerPressed() && karrat.Contains(Imgui::Point(mouseX, mouseY))) {
					if (widgetIds[i] == Internal::gState->hotControl || widgetIds[i] == Internal::gState->activeControl) {
						hotIndex = i;
					}
				}
				Draw2D::DrawRect(karrat.x, karrat.y, karrat.w, karrat.h, hotBg.r, hotBg.g, hotBg.b);
			}

			karrat.w += 10;
			Draw2D::DrawString(Internal::gState->interfaceFont, height, karrat.x + 3, karrat.y + height , options[i], fontColor.r, fontColor.g, fontColor.b);
			karrat.x += karrat.w;
		}

		return hotIndex;
	}

	u32 Header(const Rect& screenPos, const char** options, u32 numOptions, u32 selectedOption) {
		const StyleSheet* s = Internal::gState->style;

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, s->headerBgColor.r, s->headerBgColor.g, s->headerBgColor.b);
		Draw2D::DrawRect(screenPos.x, i32(screenPos.y + screenPos.h) - 2, screenPos.w, 1, s->dividerBColor.r, s->dividerBColor.g, s->dividerBColor.b);

		float carrat = screenPos.x;
		Imgui::Point mousePos(Internal::gState->GetPointerX() / Internal::gState->dpi,
			Internal::gState->GetPointerY() / Internal::gState->dpi);

		float colorA[3] = {
				Internal::gState->style->dividerAColor.r, Internal::gState->style->dividerAColor.g, Internal::gState->style->dividerAColor.b
		};
		float colorB[3] = {
			Internal::gState->style->dividerBColor.r, Internal::gState->style->dividerBColor.g, Internal::gState->style->dividerBColor.b
		};

		u32 result = selectedOption;

		for (u32 i = 0; i < numOptions; ++i) {
			if (options[i] == 0) {
				if (result == i) {
					result = 0;
				}
				continue;
			}

			Draw2D::Size stringSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->headerFontSize, options[i]);
			stringSize.w += 10;

			Rect clickArea(carrat, screenPos.y, stringSize.w, screenPos.h);
			i32 widgetId = Internal::gState->NegativeId(clickArea);

			if (Contains(clickArea, mousePos)) {
				if (Internal::MakeHotOrActive(widgetId, clickArea, vec2(mousePos.x, mousePos.y), 0)) {
					result = i;
				}
			}

			if (Internal::gState->activeControl == widgetId) {
				Draw2D::DrawRect(clickArea.x, clickArea.y, clickArea.w, clickArea.h, s->HeaderBGActiveColor.r, s->HeaderBGActiveColor.g, s->HeaderBGActiveColor.b);
			}
			else  if (Internal::gState->hotControl == widgetId) {
				Draw2D::DrawRect(clickArea.x, clickArea.y, clickArea.w, clickArea.h, s->HeaderBGHotColor.r, s->HeaderBGHotColor.g, s->HeaderBGHotColor.b);
			}
			else if (result == i) {
				Draw2D::DrawRect(clickArea.x, clickArea.y, clickArea.w, clickArea.h, s->HeaderBGActiveColor.r, s->HeaderBGActiveColor.g, s->HeaderBGActiveColor.b);
			}

			Draw2D::DrawString(Internal::gState->interfaceFont, s->headerFontSize,
				carrat + 5, screenPos.y + 3 + s->headerFontSize, options[i],
				s->headerFontColor.r, s->headerFontColor.g, s->headerFontColor.b);
			
			carrat += stringSize.w;

			if (i + 1 < numOptions) {
				Rect divider = screenPos;
				divider.x = carrat;
				divider.w = 2;
				Draw2D::DrawRect(divider.x, divider.y, divider.w, divider.h, colorA[0], colorA[1], colorA[2]);
				divider.x = carrat + 1;
				divider.w = 1;
				Draw2D::DrawRect(divider.x, divider.y, divider.w, divider.h, colorB[0], colorB[1], colorB[2]);
				carrat += 2;
			}
		}

		return result;
	}

	void SetTooltipIcon(int codePoint) {
		Internal::gState->tooltipIcon = codePoint;
	}

	void SetTooltipLabel(const char* label) {
		Internal::gState->tooltipLabel = label;
	}

	Point GetPointer(float dpi) {
		return Point(Internal::gState->GetPointerX() / dpi, Internal::gState->GetPointerY() / dpi);
	}

	void Icon(const Rect& screenPos, u32 iconSize, u32 codePoint, float r, float g, float b, float a) {
		Internal::gState->NegativeId(screenPos);
		Draw2D::DrawCodePoint(
			Internal::gState->widgetFont, iconSize,
			screenPos.x,
			screenPos.y + iconSize,
			codePoint,
			r, g, b, a
		);
	}

	bool HandleScroll(const Rect& scrollArea) {
		float dpi = Internal::gState->dpi;
		float mouseX = Internal::gState->GetPointerX() / dpi;
		float mouseY = Internal::gState->GetPointerY() / dpi;
		return Contains(scrollArea, Point(mouseX, mouseY));
	}

	float FloatSlider(const Rect& screeenPos, float value) {
		if (screeenPos.w <= 1.0f) {
			return 0.0f;
		}

		i32 scrollTrackId = Internal::gState->NegativeId(screeenPos);

		float dpi = Internal::gState->dpi;
		Point mousePos((float)Internal::gState->GetPointerX() / dpi, (float)Internal::gState->GetPointerY() / dpi);
		Point mouseDelta((float)Internal::gState->GetPointerDeltaX() / dpi, (float)Internal::gState->GetPointerDeltaY() / dpi);
		const StyleSheet* s = Internal::gState->style;

		if (mousePos.x < screeenPos.x || mousePos.x > screeenPos.x + screeenPos.w || mousePos.y < screeenPos.y || mousePos.y > screeenPos.y + screeenPos.h) {
				mouseDelta = Point(0, 0);
		}

		Draw2D::DrawRect(screeenPos.x, screeenPos.y, screeenPos.w, screeenPos.h,
			s->scrollBarTrackBG.r, s->scrollBarTrackBG.g, s->scrollBarTrackBG.b);

		float v = value;
		if (v < 0.0f) {
			v = 0.0f;
		}
		else if (v > 1.0f) {
			v = 1.0f;
		}

		Rect track = screeenPos;
		Rect grabHandle = screeenPos;
		
		bool horizontal = true;

		f32 grabberSize = 4.0f;
		if (horizontal) {
			if (grabberSize > track.w - track.w / 4.0f) {
				grabberSize = track.w - track.w / 4.0f;
			}

			// Adjust track area for grabber
			track.x += grabberSize / 2.0f;
			track.w -= grabberSize;

			if (grabberSize > 0.0f) {
				grabHandle.h -= 6;
				grabHandle.y += 3;
				grabHandle.w = grabberSize;

				float handlePos = track.w * v + track.x;
				grabHandle.x = handlePos - grabberSize / 2;
			}
		}
		else {
			if (grabberSize > track.h - track.h / 4.0f) {
				grabberSize = track.h - track.h / 4.0f;
			}

			// Adjust track area for grabber
			track.y += grabberSize / 2.0f;
			track.h -= grabberSize;

			if (grabberSize > 0.0f) {
				grabHandle.w -= 6;
				grabHandle.x += 3;
				grabHandle.h = grabberSize;

				float handlePos = track.h * v + track.y;
				grabHandle.y = handlePos - grabberSize / 2;
			}
		}

		i32 scrollHandleId = Internal::gState->NegativeId(grabHandle);

		Internal::MakeHotOrActive(scrollHandleId, grabHandle, vec2(mousePos.x, mousePos.y), 0);

		if (Internal::gState->activeControl == scrollHandleId) {
			float scroll_v = 0.0f;
			if (horizontal) {
				if (Internal::gState->activeControl == scrollHandleId) {
					scroll_v = mouseDelta.x / track.w;
				}
			}
			else {
				if (Internal::gState->activeControl == scrollHandleId) {
					scroll_v = mouseDelta.y / track.h;
				}
			}
			v += scroll_v;
		}

		if (v < 0.0f) {
			v = 0.0f;
		}
		else if (v > 1.0f) {
			v = 1.0f;
		}

		{ // Handle
			if ((Internal::gState->hotControl == scrollHandleId && Internal::gState->activeControl == 0) ||
				Internal::gState->activeControl == scrollHandleId) {
				Draw2D::DrawRect(grabHandle.x, grabHandle.y, grabHandle.w, grabHandle.h,
					s->scrollGrabberHot.r, s->scrollGrabberHot.g, s->scrollGrabberHot.b);
			}
			else {
				Draw2D::DrawRect(grabHandle.x, grabHandle.y, grabHandle.w, grabHandle.h,
					s->scrollGrabberNormal.r, s->scrollGrabberNormal.g, s->scrollGrabberNormal.b);
			}
		}

		return v;
	}

	float ScrollBar(const Rect& screenPos, float value, bool horizontal, float grabberSize, bool handleScroll) {
		i32 scrollTrackId = Internal::gState->NegativeId(screenPos);

		float dpi = Internal::gState->dpi;
		Point mousePos((float)Internal::gState->GetPointerX() / dpi, (float)Internal::gState->GetPointerY() / dpi);
		Point mouseDelta((float)Internal::gState->GetPointerDeltaX() / dpi, (float)Internal::gState->GetPointerDeltaY() / dpi);
		
		const StyleSheet* s = Internal::gState->style;

		if (grabberSize < 0.0f) {
			grabberSize = 0.0f;
		}

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h,
			s->scrollBarTrackBG.r, s->scrollBarTrackBG.g, s->scrollBarTrackBG.b);

		u32 arrowAIcon = 0;
		u32 arrowBIcon = 0;

		float v = value;
		if (v < 0.0f) {
			v = 0.0f;
		}
		else if (v > 1.0f) {
			v = 1.0f;
		}

		Rect arrowAArea = screenPos;
		Rect arrowBArea = screenPos;
		Rect track = screenPos;
		Rect grabHandle = screenPos;

		StyleColor iconAColor = s->scrollBarIconNormal;
		StyleColor iconBColor = s->scrollBarIconNormal;

		if (horizontal) {
			arrowAIcon = IMGUI_ICON_CODEPOINT_ARROW_LEFT;
			arrowBIcon = IMGUI_ICON_CODEPOINT_ARROW_RIGHT;
			arrowAArea.w = arrowAArea.h;
			arrowBArea.w = arrowBArea.h;
			arrowBArea.x = screenPos.x + screenPos.w - arrowBArea.w;

			// Set track between arrows
			track.x += arrowAArea.w;
			track.w -= arrowAArea.w * 2.0f;

			// Add 1px gab before / after arrows
			track.x += 1;
			track.w -= 2;

			if (grabberSize > track.w - track.w / 4.0f) {
				grabberSize = track.w - track.w / 4.0f;
			}

			// Adjust track area for grabber
			track.x += grabberSize / 2.0f;
			track.w -= grabberSize;

			if (grabberSize > 0.0f) {
				grabHandle.h -= 6;
				grabHandle.y += 3;
				grabHandle.w = grabberSize;

				float handlePos = track.w * v + track.x;
				grabHandle.x = handlePos - grabberSize / 2;
			}
		}
		else {
			arrowAIcon = IMGUI_ICON_CODEPOINT_ARROW_UP;
			arrowBIcon = IMGUI_ICON_CODEPOINT_ARROW_DOWN;
			arrowAArea.h = arrowAArea.w;
			arrowBArea.h = arrowBArea.w;
			arrowBArea.y = screenPos.y + screenPos.h - arrowBArea.h;

			// Set track between arrows
			track.y += arrowBArea.h;
			track.h -= arrowBArea.h * 2.0f;

			// Add 1px gab before / after arrows
			track.y += 1;
			track.h -= 2;

			if (grabberSize > track.h - track.h / 4.0f) {
				grabberSize = track.h - track.h / 4.0f;
			}

			// Adjust track area for grabber
			track.y += grabberSize / 2.0f;
			track.h -= grabberSize;

			if (grabberSize > 0.0f) {
				grabHandle.w -= 6;
				grabHandle.x += 3;
				grabHandle.h = grabberSize;

				float handlePos = track.h * v + track.y;
				grabHandle.y = handlePos - grabberSize / 2;
			}
		}

		i32 scrollUpId = Internal::gState->NegativeId(arrowAArea); 
		i32 scrollDownId = Internal::gState->NegativeId(arrowBArea);
		i32 scrollHandleId = Internal::gState->NegativeId(grabHandle);

		if (grabberSize > 0.0f) {
			Internal::MakeHotOrActive(scrollUpId, arrowAArea, vec2(mousePos.x, mousePos.y), 0);
			Internal::MakeHotOrActive(scrollDownId, arrowBArea, vec2(mousePos.x, mousePos.y), 0);
			Internal::MakeHotOrActive(scrollHandleId, grabHandle, vec2(mousePos.x, mousePos.y), 0);
		}

		if (Internal::gState->activeControl == scrollUpId) {
			if (Internal::gState->pulse) {
				v -= 0.1f;
			}
		}
		else if (Internal::gState->activeControl == scrollDownId) {
			if (Internal::gState->pulse) {
				v += 0.1f;
			}
		}
		else if (Internal::gState->activeControl == scrollHandleId) {
			float scroll_v = 0.0f;
			if (horizontal) {
				if (Internal::gState->activeControl == scrollHandleId) {
					scroll_v = mouseDelta.x / track.w;
				}
			}
			else {
				if (Internal::gState->activeControl == scrollHandleId) {
					scroll_v = mouseDelta.y / track.h;
				}
			}
			v += scroll_v;
		}
		else if (handleScroll) { // This is the scroll wheel here
			i32 scroll = MouseGetScroll();
			i32 prev = MousePrevScroll();
			if (scroll != prev) {
				if (scroll > 0) {
					v -= 0.1f;
				}
				else if (scroll < 0) {
					v += 0.1f;
				}
			}
		}

		if (v < 0.0f) {
			v = 0.0f;
		}
		else if (v > 1.0f) {
			v = 1.0f;
		}

		if ((Internal::gState->hotControl == scrollUpId && Internal::gState->activeControl == 0)|| Internal::gState->activeControl == scrollUpId) {
			iconAColor = s->scrollBarIconHot;
			Draw2D::DrawRect(arrowAArea.x, arrowAArea.y, arrowAArea.w, arrowAArea.h,
				s->scrollBarHotButtonBg.r, s->scrollBarHotButtonBg.g, s->scrollBarHotButtonBg.b);
		}
		else if ((Internal::gState->hotControl == scrollDownId && Internal::gState->activeControl == 0) || Internal::gState->activeControl == scrollDownId) {
			iconBColor = s->scrollBarIconHot;
			Draw2D::DrawRect(arrowBArea.x, arrowBArea.y, arrowBArea.w, arrowBArea.h,
				s->scrollBarHotButtonBg.r, s->scrollBarHotButtonBg.g, s->scrollBarHotButtonBg.b);
		}
		
		if (grabberSize > 0.0f) {
			Imgui::Icon(arrowAArea, s->scrollIconSize, arrowAIcon, iconAColor.r, iconAColor.g, iconAColor.b);
			Imgui::Icon(arrowBArea, s->scrollIconSize, arrowBIcon, iconBColor.r, iconBColor.g, iconBColor.b);
		}

		if (grabberSize > 0.0f) { // Handle
			if ((Internal::gState->hotControl == scrollHandleId && Internal::gState->activeControl == 0) || Internal::gState->activeControl == scrollHandleId) {
				Draw2D::DrawRect(grabHandle.x, grabHandle.y, grabHandle.w, grabHandle.h,
					s->scrollGrabberHot.r, s->scrollGrabberHot.g, s->scrollGrabberHot.b);
			}
			else {
				Draw2D::DrawRect(grabHandle.x, grabHandle.y, grabHandle.w, grabHandle.h,
					s->scrollGrabberNormal.r, s->scrollGrabberNormal.g, s->scrollGrabberNormal.b);
			}
		}

		if (grabberSize <= 0.0f) {
			return 0.0f;
		}

		return v;
	}

	bool HoldArea(const Rect& screenPos, HoldAreaDetails* details, float* roc, const vec2* rocV) {
		i32 widgetId = Internal::gState->NegativeId(screenPos);
		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		bool wasActive = Internal::gState->activeControl == widgetId;
		bool activated = Internal::MakeHotOrActive(widgetId, screenPos, mouse, 0);
		bool active = Internal::gState->activeControl == widgetId;

		if (details != 0) {
			details->wasActive = wasActive;
			details->activated = activated;
			details->active = active;
			if (!details->wasActive && details->active) {
				details->mouseDown = mouse;
				if (roc != 0) {
					details->rememberF32OnClick = *roc;
				}
				if (rocV != 0) {
					details->rememberVec2OnClick = *rocV;
				}
			}
		}

		return activated || active;
	}

	bool ClickArea(const Rect& screenPos) {
		i32 widgetId = Internal::gState->NegativeId(screenPos);
		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);
		return Internal::MakeHotOrActive(widgetId, screenPos, mouse, 0);
	}

	bool SidebarButton(const Rect& screenPos, u32 codePoint, const char* caption, bool isActive, vec3* iconOverride) {
		const StyleSheet* s = Internal::gState->style;
		i32 widgetId = Internal::gState->NegativeId(screenPos);

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		bool result = Internal::MakeHotOrActive(widgetId, screenPos, mouse, caption);

		StyleColor bgColor = s->toolBarBg;
		if (Internal::gState->hotControl == widgetId) {
			bgColor = s->dividerAColor;
		}
		if (Internal::gState->activeControl == widgetId || isActive) {
			bgColor = s->headerBgColor;
		}

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, bgColor.r, bgColor.g, bgColor.b);

		StyleColor color = s->labelFontColorNormal;
		f32 height = screenPos.h - 8.0f;

		float x_off = 0.0f;
		float y_off = 0.0f;
		if (iconOverride != 0) {
			x_off = iconOverride->x;
			y_off = iconOverride->y;
		}

		Draw2D::DrawCodePoint(Internal::gState->widgetFont, iconOverride != 0? iconOverride->z : height, screenPos.x + 3.0 + x_off, y_off + screenPos.y + height + 4.0f, codePoint,
			color.r, color.g, color.b);

		return result;
	}

	bool FooterButton(const Rect& screenPos, u32 codePoint, const char* caption, const StyleColor& color) {
		const StyleSheet* s = Internal::gState->style;
		u32 iconSize = u32(s->hierarchyFooterButtonSize) - 4;

		i32 widgetId = Internal::gState->NegativeId(screenPos);

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		bool result = Internal::MakeHotOrActive(widgetId, screenPos, mouse, caption);

		if (Internal::gState->activeControl == widgetId) {
			StyleColor border = s->hierarchyFooterButtonBorder_Active;
			StyleColor button = s->hierarchyFooterButtonBG_Active;

			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h,
				border.r, border.g, border.b);
			Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2,
				button.r, button.g, button.b);
		}
		else if (Internal::gState->hotControl == widgetId) {
			StyleColor border = s->hierarchyFooterButtonBorder_Hot;
			StyleColor button = s->hierarchyFooterButtonBG_Hot;

			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h,
				border.r, border.g, border.b);
			Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2,
				button.r, button.g, button.b);
		}
#if 0
		else { // Debugging for normal state
			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, 1, 0, 0);
		}
#endif

		Draw2D::DrawCodePoint(
			Internal::gState->widgetFont, iconSize,
			screenPos.x + 2,
			screenPos.y + iconSize + 3,
			codePoint,
			color.r, color.g, color.b, 1.0f
		);

		return result;
	}

	i32 Dummy() {
		Rect screenPos = { 0 };
		screenPos.w = screenPos.h = 5;
		screenPos.x = screenPos.y = -10;
		i32 widgetId = Internal::gState->NegativeId(screenPos);
		return widgetId;
	}

	bool ToggleIcon(const Rect& screenPos, u32 trueIcon, u32 falseIcon, bool state, const char* label) {
		const StyleSheet* s = Internal::gState->style;
		u32 iconSize = u32(screenPos.h) - 6;

		i32 widgetId = Internal::gState->NegativeId(screenPos);

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		bool result = state;
		if (Internal::MakeHotOrActive(widgetId, screenPos, mouse, label)) {
			result = !result;
		}

		StyleColor iconColor = s->textAreaFont_Normal;

		Draw2D::DrawCodePoint(Internal::gState->widgetFont, screenPos.h - 6, screenPos.x + 3, screenPos.y + 3 + (screenPos.h - 6),
			state ? trueIcon : falseIcon, iconColor.r, iconColor.g, iconColor.b);

		return result;
	}

	bool ToggleButton(const Rect& screenPos, u32 trueIcon, u32 falseIcon, bool state, bool disabled, const char* label, const StyleColor& tint, StyleColor* iconTint) {
		const StyleSheet* s = Internal::gState->style;
		u32 iconSize = u32(screenPos.h) - 6;

		i32 widgetId = Internal::gState->NegativeId(screenPos);

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		bool result = state;
		if (!disabled && Internal::MakeHotOrActive(widgetId, screenPos, mouse, label)) {
			result = !result;
		}

		StyleColor borderColor = s->toggleButtonBorder;
		StyleColor bgColor = s->toggleButtonNormal;
		
		if (disabled) {
			borderColor = s->toggleButtonDisabledBorder;
			bgColor = s->toggleButtonDisabled;
		}
		else if (state) {
			bgColor = s->toggleButtonActive;
			if (Internal::gState->hotControl == widgetId) {
				bgColor = s->toggleButtonHot;
			}
		}
		else {
			if (Internal::gState->activeControl == widgetId) {
				bgColor = s->toggleButtonActive;
			}
			else if (Internal::gState->hotControl == widgetId) {
				bgColor = s->toggleButtonHot;
			}
		}

		// Adjustments
		borderColor.r += tint.r;
		borderColor.g += tint.g;
		borderColor.b += tint.b;
		bgColor.r += tint.r;
		bgColor.g += tint.g;
		bgColor.b += tint.b;

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, borderColor.r, borderColor.g, borderColor.b);
		Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2, bgColor.r, bgColor.g, bgColor.b);

		StyleColor iconColor = s->textAreaFont_Normal;
		if (disabled) {
			iconColor = s->textAreaFont_Disabled;
		}

		if (iconTint != 0) {
			iconColor.r += iconTint->r;
			iconColor.g += iconTint->g;
			iconColor.b += iconTint->b;
		}

		Draw2D::DrawCodePoint(Internal::gState->widgetFont, screenPos.h - 6, screenPos.x + 3, screenPos.y + 3 + (screenPos.h - 6), 
			state ? trueIcon : falseIcon, iconColor.r, iconColor.g, iconColor.b);

		return result;
	}

	namespace Internal {
		i32 PopupMenuBegin(i32 id, const Point& screenPos, f32 width, u32 numOptions, bool reversed) {
			i32 result = -1;
			const StyleSheet* s = Internal::gState->style;
			f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
			f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
			f32 dpi = Internal::gState->dpi;
			vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

			Rect screenArea(screenPos.x, screenPos.y, width, numOptions * s->listBoxItemHeight);
			if (reversed) {
				screenArea.y -= numOptions * s->listBoxItemHeight;
				screenArea.y -= s->listBoxItemHeight;
			}

			if (MakeHotOrActive(id, screenArea, mouse, "Display Sprite")) {
				float relative_y = mouse.y - screenArea.y;
				if (relative_y >= 0.0f && relative_y <= screenArea.h) {
					i32 index = relative_y / s->listBoxItemHeight;
					if (index >= 0 && index < numOptions) {
						result = index;
					}
				}
			}

			i32 hot = -1;

			float relative_y = mouse.y - screenArea.y;
			if (mouse.x >= screenPos.x && mouse.x <= screenPos.x + width) {
				if (relative_y >= 0.0f && relative_y <= screenArea.h) {
					i32 index = relative_y / s->listBoxItemHeight;
					if (index >= 0 && index < numOptions) {
						hot = index;
					}
				}
			}

			if (Internal::gState->numPopupMenus >= Internal::gState->modalPopups.Count()) {
				Internal::gState->modalPopups.Resize(Internal::gState->numPopupMenus + 1);
				new (&Internal::gState->modalPopups[Internal::gState->numPopupMenus].items) Vector< Internal::PopupMenuInfo>();
				Internal::gState->maxPopupMenus += 1;
			}

			Internal::PopupMenuInfo& reuse = Internal::gState->modalPopups[Internal::gState->numPopupMenus];
			reuse.id = id;
			reuse.reversed = reversed;
			reuse.screenPos = screenArea;
			reuse.hot = hot;
			reuse.itemCount = 0;
			reuse.items.Resize(numOptions);
			
			Internal::gState->lastModalId = id;
			Internal::gState->numPopupMenus += 1;

			return result;
		}

		void PopupMenuRender(PopupMenuInfo& info) {
			const StyleSheet* s = Internal::gState->style;
			StyleColor bg = s->textAreaBg_Normal;
			StyleColor outline = s->textAreaOutline_Normal;
			StyleColor txtN = s->textAreaFont_Normal;
			StyleColor txtH = s->textAreaFont_Hot;
			StyleColor hover = s->hierarchyItemBG_Selected;

			Draw2D::DrawRect(info.screenPos.x, info.screenPos.y, info.screenPos.w, info.screenPos.h, outline.r, outline.g, outline.b);
			Draw2D::DrawRect(info.screenPos.x + 1, info.screenPos.y + 1, info.screenPos.w - 2, info.screenPos.h - 2, bg.r, bg.g, bg.b);

			Draw2D::PushClip(info.screenPos.x + 1, info.screenPos.y + 1, info.screenPos.w - 2, info.screenPos.h - 2);
			f32 item_x = info.screenPos.x + 3;
			f32 item_y = info.screenPos.y + 2;

			for (i32 i = 0; i < info.itemCount; ++i) {
				const char* item = info.items[i];
				PlatformAssert(item != 0, __LOCATION__);

				if (i == info.hot) {
					Draw2D::DrawRect(info.screenPos.x + 1,
						info.screenPos.y + 1 + (i * s->listBoxItemHeight),
						info.screenPos.w - 2, s->listBoxItemHeight, hover.r, hover.g, hover.b);
				}

				Draw2D::DrawString(Internal::gState->interfaceFont, s->textAreaFontSize, item_x, item_y + s->textAreaFontSize, item, txtN.r, txtN.g, txtN.b);
				item_y += s->listBoxItemHeight;
			}
			Draw2D::PopClip();
		}

		void PopupMenuEnd() {
			Internal::gState->lastModalId = 0;
		}
	}

	// If endModalPopup returned the i32 instead, then we could
	// infer width. And the height is already given. This function
	// would become (Point screen, u32 count)
	i32 BeginModalPopup(const Rect& screenPos, const Rect& activation, u32 numOptions, bool reversed) {
		i32 activationId = Internal::gState->NegativeId(screenPos);
		i32 contextId = Internal::gState->NegativeId(screenPos);

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		if (Internal::MakeHotOrActive(activationId, activation, mouse, 0)) {
			Internal::gState->modalControl = contextId;
		}

		Internal::gState->lastBeingModalPopupContext = contextId;
		Internal::gState->lastBeingModalPopupActivation = activationId;

		if (Internal::gState->activeControl == contextId) {
			return Internal::PopupMenuBegin(contextId, Point(screenPos.x, screenPos.y + screenPos.h), screenPos.w, numOptions, reversed);
		}

		return -1;
	}

	void PushModalPopupItem(const char* item) {
		if (Internal::gState->lastModalId != 0 && Internal::gState->numPopupMenus > 0) {
			Internal::PopupMenuInfo* menu = 0;
			for (u32 i = 0; i < Internal::gState->numPopupMenus; ++i) {
				if (Internal::gState->modalPopups[i].id == Internal::gState->lastModalId) {
					menu = &Internal::gState->modalPopups[i];
					PlatformAssert(menu->itemCount <= menu->items.Count(), __LOCATION__);
					break;
				}
			}

			PlatformAssert(menu != 0, __LOCATION__);
			menu->items[menu->itemCount++] = item;
		}
	}

	bool EndModalPopup() {
		Internal::PopupMenuEnd();
		return Internal::gState->activeControl == Internal::gState->lastBeingModalPopupContext ||
			Internal::gState->modalControl == Internal::gState->lastBeingModalPopupContext ||
			Internal::gState->modalControl == Internal::gState->lastBeingModalPopupActivation ||
			//Internal::gState->lastActiveControl == Internal::gState->lastBeingModalPopupContext ||
			//Internal::gState->lastActiveControl == Internal::gState->lastBeingModalPopupActivation ||
			Internal::gState->activeControl ==  Internal::gState->lastBeingModalPopupActivation;
	}

	i32 Timeline(const Rect& screenPos, u32 numFrames, i32 selectedFrame, float scroll) {
		const StyleSheet* s = Internal::gState->style;
		StyleColor highlight = s->hierarchyItemBG_Selected;

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		float scrollableWidth = numFrames * s->animationFrameWidth;

		float x_offset = 0.0f;
		if (screenPos.w < scrollableWidth) {
			float delta = scrollableWidth - screenPos.w;
			x_offset = scroll * delta;
		}

		i32 widgetId = Internal::gState->NegativeId(screenPos);
		if (Internal::MakeHotOrActive(widgetId, screenPos, mouse, 0) || Internal::gState->activeControl == widgetId) {
			float x_delta = (mouse.x + x_offset) - screenPos.x;
			if (mouse.x >= screenPos.x && mouse.x <= screenPos.x + screenPos.w) {
				u32 index = x_delta / s->animationFrameWidth;
				if (index < numFrames) {
					selectedFrame = index;
				}
			}
		}

		Imgui::Rect frameRect = screenPos;
		frameRect.x -= x_offset;

		frameRect.w = s->animationFrameWidth;
		char printBuff[8] = { 0 };

		for (u32 i = 0; i < numFrames; ++i) {
			if (Imgui::Intersects(screenPos, frameRect)) {
				StyleColor frameColor = (i % 2 == 0) ? s->frameEven : s->frameOdd;
				Draw2D::DrawRect(frameRect.x, frameRect.y, frameRect.w, frameRect.h, frameColor.r, frameColor.g, frameColor.b);

				if (selectedFrame >= 0 && selectedFrame == i) {
					Draw2D::DrawRect(frameRect.x, frameRect.y, frameRect.w, frameRect.h, highlight.r, highlight.g, highlight.b);

					if (!((i + 1) % 5 == 0 || i == 0)) {
						Imgui::Rect labelRect = frameRect;
						labelRect.x += 1;
						labelRect.h = 6;
						labelRect.y = frameRect.y + frameRect.h - labelRect.h - 2;
						stbsp_snprintf(printBuff, 8, "%d", i + 1);
						Imgui::Label(labelRect, printBuff, false, false, true);
					}
				}

				if ((i + 1) % 5 == 0 || i == 0) {
					Imgui::Rect labelRect = frameRect;
					labelRect.x += 1;
					labelRect.h = 6;
					labelRect.y = frameRect.y + frameRect.h - labelRect.h - 2;
					stbsp_snprintf(printBuff, 8, "%d", i + 1);
					Imgui::Label(labelRect, printBuff, false, false, true);
				}
			}

			if (frameRect.x > screenPos.x + screenPos.w) {
				break;
			}
			frameRect.x += frameRect.w;
		}

		return selectedFrame;
	}

	void EndComboBox() {
		Internal::PopupMenuEnd();
	}

	void PushComboBoxItem(const char* item) {
		if (Internal::gState->lastModalId != 0 && Internal::gState->numPopupMenus > 0) {
			Internal::PopupMenuInfo* menu = 0;
			for (u32 i = 0; i < Internal::gState->numPopupMenus; ++i) {
				if (Internal::gState->modalPopups[i].id == Internal::gState->lastModalId) {
					menu = &Internal::gState->modalPopups[i];
					PlatformAssert(menu->itemCount <= menu->items.Count(), __LOCATION__);
					break;
				}
			}

			PlatformAssert(menu != 0, __LOCATION__);
			menu->items[menu->itemCount++] = item;
		}
	}

	i32 BeginComboBox(const Rect& screenPos, const char* name, u32 numOptions, i32 selectedOption, const char* label, bool reversed, bool disabled) {
		if (numOptions == 0) {
			disabled = true;
		}

		const StyleSheet* s = Internal::gState->style;
		i32 widgetId = Internal::gState->NegativeId(screenPos);
		i32 contextId = Internal::gState->NegativeId(screenPos);

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		if (selectedOption >= numOptions || selectedOption < 0) {
			selectedOption = -1;
		}

		if (!disabled) {
			if (Internal::MakeHotOrActive(widgetId, screenPos, mouse, 0)) {
				Internal::gState->modalControl = contextId;
			}
		}
		else {
			if (Internal::gState->hotControl == widgetId || Internal::gState->hotControl == contextId) {
				Internal::gState->hotControl = 0;
			}
			if (Internal::gState->activeControl == widgetId || Internal::gState->activeControl == contextId) {
				Internal::gState->activeControl = 0;
			}
		}
		
		StyleColor outline = s->textAreaOutline_Normal;
		StyleColor background = s->textAreaBg_Normal;
		StyleColor lbl = s->textAreaLabel_Normal;
		StyleColor text = s->textAreaFont_Normal;

		if (disabled) {
			outline = s->textAreaOutline_Disabled;
			background = s->textAreaBg_Disabled;
			lbl = s->textAreaLabel_Disabled;
			text = s->textAreaFont_Disabled;
		}
		else if (Internal::gState->hotControl == widgetId || Internal::gState->hotControl == contextId) {
			outline = s->textAreaOutline_Hot;
			background = s->textAreaBg_Hot;
			lbl = s->textAreaLabel_Hot;
			text = s->textAreaFont_Hot;
		}
		else if (Internal::gState->activeControl == widgetId || Internal::gState->activeControl == contextId) {
			outline = s->textAreaOutline_Active;
			background = s->textAreaBg_Active;
			lbl = s->textAreaLabel_Active;
			text = s->textAreaFont_Active;
		}

		const char* selected = name;
		if (selected == 0) {
			selected = "None";
		}

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, outline.r, outline.g, outline.b);
		Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2, background.r, background.g, background.b);
		if (label != 0) {
			Draw2D::DrawString(Internal::gState->labelFont, s->textAreaLabelSize, screenPos.x + 3, screenPos.y, label, lbl.r, lbl.g, lbl.b);
		}

		Draw2D::Size stringSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, selected);
		
		float max_w = screenPos.w - 2;
		max_w -= s->txtAreaHeight;

		if (stringSize.w > max_w) {
			Draw2D::PushClip(screenPos.x + 1, screenPos.y + 1, screenPos.w - 3 - s->textAreaFontSize, screenPos.h - 2);
		}

		Draw2D::DrawString(Internal::gState->interfaceFont, s->textAreaFontSize, screenPos.x + 3, screenPos.y + 1 + s->textAreaFontSize, selected, text.r, text.g, text.b);
		
		if (stringSize.w > max_w) {
			Draw2D::PopClip();
		}

		float rect_x = screenPos.x + screenPos.w - 3 - s->textAreaFontSize;
		Draw2D::DrawCodePoint(Internal::gState->widgetFont, s->textAreaFontSize, rect_x, screenPos.y + 3 + s->textAreaFontSize, 
			IMGUI_ICON_CODEPOINT_ARROW_DOWN, text.r, text.g, text.b);

		if (Internal::gState->activeControl == contextId) {
			i32 selection = Internal::PopupMenuBegin(contextId, Point(screenPos.x, screenPos.y + screenPos.h), screenPos.w, numOptions, reversed);
			
			if (selection >= 0) {
				return selection;
			}
		}

		return selectedOption;
	}

	void DrawColorPicker(const Rect& screenPos, const hsv& color) {
		const StyleSheet* s = Internal::gState->style;
		rgb selectedRgb = hsv2rgb(color);

		Rect colorArea(screenPos.x + 2, screenPos.y + 2, screenPos.w - 4, screenPos.h - 4);
		colorArea.w -= 24;
		colorArea.h -= 20;

		Rect hueueArea = colorArea;
		hueueArea.x = colorArea.x + colorArea.w + 10;
		hueueArea.w = 13;

		hsv topLeft;
		topLeft.h = color.h; // h = hueue slider
		topLeft.s = 0.0f;      // s = x axis
		topLeft.v = 1.0f;      // v = y axis
		rgb tl = hsv2rgb(topLeft);

		hsv topRight;
		topRight.h = color.h; // h = hueue slider
		topRight.s = 1.0f;      // s = x axis
		topRight.v = 1.0f;      // v = y axis
		rgb tr = hsv2rgb(topRight);

		hsv bottomLeft;
		bottomLeft.h = color.h; // h = hueue slider
		bottomLeft.s = 0.0f;      // s = x axis
		bottomLeft.v = 0.0f;      // v = y axis
		rgb bl = hsv2rgb(bottomLeft);

		hsv bottomRight;
		bottomRight.h = color.h; // h = hueue slider
		bottomRight.s = 1.0f;      // s = x axis
		bottomRight.v = 0.0f;      // v = y axis
		rgb br = hsv2rgb(bottomRight);

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h,
			s->textAreaOutline_Normal.r, s->textAreaOutline_Normal.g, s->textAreaOutline_Normal.b);
		Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2,
			s->textAreaBg_Normal.r, s->textAreaBg_Normal.g, s->textAreaBg_Normal.b);

		{ // RGB label
			float fontSize = 14.0f;
			char buffer[24] = { 0 };
			int r = MathMaxI(MathMinI(selectedRgb.r * 255.0f, 255), 0);
			int g = MathMaxI(MathMinI(selectedRgb.g * 255.0f, 255), 0);
			int b = MathMaxI(MathMinI(selectedRgb.b * 255.0f, 255), 0);
			stbsp_snprintf(buffer, 24, "R: %d, G: %d, B: %d", r, g, b);
			Draw2D::DrawString(Internal::gState->interfaceFont, fontSize, colorArea.x + 22, colorArea.y + colorArea.h + fontSize + 2,
				buffer, s->textAreaFont_Normal.r, s->textAreaFont_Normal.g, s->textAreaFont_Normal.b);

			Draw2D::DrawRect(colorArea.x + 1, colorArea.y + colorArea.h + 3, 15, 15, selectedRgb.r, selectedRgb.g, selectedRgb.b);
		}

		{ // Hueue block
			hsv hsvTop;
			hsvTop.h = 359.0f;
			hsvTop.s = hsvTop.v = 1.0f;
			hsv hsvBottom = hsvTop;
			hsvBottom.h = 0.0f;
			Rect hueueSlice = hueueArea;
			float step = hueueArea.h / 10.0f;
			hueueSlice.h = step;

			for (int i = 0; i < 10; ++i) {
				float t0 = (float)(i) / 10.0f;
				float t1 = (float)(i + 1) / 10.0f;

				hsvTop.h = 359.0f * t0;
				hsvBottom.h = 359.0f * t1;

				rgb rgbTop = hsv2rgb(hsvTop);
				rgb rgbBottom = hsv2rgb(hsvBottom);

				Draw2D::DrawRect(hueueSlice.x, hueueSlice.y, hueueSlice.w, hueueSlice.h,
					rgbTop.r, rgbTop.g, rgbTop.b,
					rgbTop.r, rgbTop.g, rgbTop.b,
					rgbBottom.r, rgbBottom.g, rgbBottom.b,
					rgbBottom.r, rgbBottom.g, rgbBottom.b);

				hueueSlice.y += step;
			}
		}

		{ // Hueue indicator
			float ht = color.h / 359.0f;
			float y_pos = ht * hueueArea.h + hueueArea.y;

			Draw2D::DrawRect(hueueArea.x, y_pos, hueueArea.w, 2, 0.0f, 0.0f, 0.0f);
			Draw2D::DrawRect(hueueArea.x, y_pos + 1, hueueArea.w, 1, 1.0f, 1.0f, 1.0f);
		}

		// Main color block
#if 1
		float slice_w = colorArea.w / 5.0f;
		float slice_h = colorArea.h / 5.0f;

		for (int x = 0; x < 5; ++x) {
			float x0 = (float)(x) / 5.0f;
			float x1 = (float)(x + 1) / 5.0f;

			for (int y = 0; y < 5; ++y) {
				float y0 = (float)(y) / 5.0f;
				float y1 = (float)(y + 1) / 5.0f;

				float slice_x = colorArea.x + (float)x * slice_w;
				float slice_y = colorArea.y + (float)y * slice_h;

				rgb _tl = hsv2rgb(lerp(lerp(topLeft, topRight, x0), lerp(bottomLeft, bottomRight, x0), y0));
				rgb _tr = hsv2rgb(lerp(lerp(topLeft, topRight, x1), lerp(bottomLeft, bottomRight, x1), y0));

				rgb _bl = hsv2rgb(lerp(lerp(topLeft, topRight, x0), lerp(bottomLeft, bottomRight, x0), y1));
				rgb _br = hsv2rgb(lerp(lerp(topLeft, topRight, x1), lerp(bottomLeft, bottomRight, x1), y1));

				Draw2D::DrawRect(slice_x, slice_y, slice_w, slice_h,
					_tl.r, _tl.g, _tl.b,
					_tr.r, _tr.g, _tr.b,
					_br.r, _br.g, _br.b,
					_bl.r, _bl.g, _bl.b);
			}
		}
#else
		Draw2D::DrawRect(colorArea.x, colorArea.y, colorArea.w, colorArea.h,
			tl.r, tl.g, tl.b,
			tr.r, tr.g, tr.b,
			bl.r, bl.g, bl.b,
			br.r, br.g, br.b);
#endif

		{ // Color indicator
			float yt = 1.0f - color.v;
			float xt = color.s;
			float x_pos = xt * colorArea.w + colorArea.x;
			float y_pos = yt * colorArea.h + colorArea.y;

			Draw2D::DrawRect(x_pos - 4, y_pos - 1, 8, 2, 0.0f, 0.0f, 0.0f);
			Draw2D::DrawRect(x_pos - 4, y_pos, 8, 1, 1.0f, 1.0f, 1.0f);

			Draw2D::DrawRect(x_pos - 1, y_pos - 4, 2, 8, 0.0f, 0.0f, 0.0f);
			Draw2D::DrawRect(x_pos, y_pos - 4, 1, 8, 1.0f, 1.0f, 1.0f);
		}

	}

	hsv ColorPicker(i32 widgetId, const Rect& screenPos, const hsv& inColor) {
		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);
		Point m(mouse.x, mouse.y);

		Rect colorArea(screenPos.x + 2, screenPos.y + 2, screenPos.w - 4, screenPos.h - 4);
		colorArea.w -= 24;
		colorArea.h -= 20;

		Rect hueueArea = colorArea;
		hueueArea.x = colorArea.x + colorArea.w + 10;
		hueueArea.w = 13;

		hsv selectedHsv = inColor;

		if (Internal::MakeHotOrActive(widgetId, screenPos, mouse, 0)) {

		}
		else if (Internal::gState->activeControl == widgetId) {
			bool down = MouseDown(MouseButtonRight);
			if (!Internal::gState->GetPointerUp() || down) {
				if (Contains(hueueArea, m)) {
					float t = (mouse.y - hueueArea.y) / hueueArea.h;
					if (t < 0.0f) {
						t = 0.0f;
					}
					if (t > 1.0f) {
						t = 1.0f;
					}

					selectedHsv.h = t * 359.0f;
				}
				else if (Contains(colorArea, m)) {
					float tx = (mouse.x - colorArea.x) / colorArea.w;
					float ty = (mouse.y - colorArea.y) / colorArea.h;
					if (tx < 0.0f) {
						tx = 0.0f;
					}
					if (tx > 1.0f) {
						tx = 1.0f;
					}
					if (ty < 0.0f) {
						ty = 0.0f;
					}
					if (ty > 1.0f) {
						ty = 1.0f;
					}

					selectedHsv.s = tx;
					selectedHsv.v = (1.0f - ty);
				}
			}
		}

		if (!Internal::gState->colorPickerActive) {
			DrawColorPicker(screenPos, selectedHsv);
		}

		return selectedHsv;
	}

	hsv ColorPicker(const Rect& screenPos, const hsv& inColor) {
		int widgetId = Internal::gState->NegativeId(screenPos);
		return ColorPicker(widgetId, screenPos, inColor);
	}

	hsv ColorPickerButton(const Rect& screenPos, const hsv& inColor, bool disabled, const StyleColor& tint) {
		Rect colorBox = screenPos;
		colorBox.y += screenPos.h;
		colorBox.x -= 200;
		colorBox.x += screenPos.w;

		colorBox.w = 200;
		colorBox.h = 200;

		int buttonId = Internal::gState->NegativeId(screenPos);
		int colorPicker = Internal::gState->NegativeId(colorBox);

		const StyleSheet* s = Internal::gState->style;

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		hsv displayColor = inColor;
		if (!disabled && Internal::MakeHotOrActive(buttonId, screenPos, mouse, 0)) {
			Internal::gState->modalControl = colorPicker;
			Internal::gState->lastSelectedColor = inColor;
		}

		StyleColor outline = s->textAreaOutline_Normal;
		StyleColor background = s->textAreaBg_Normal;

		if (disabled) {
			outline = s->textAreaOutline_Disabled;
			background = s->textAreaBg_Disabled;
		}
		else if (Internal::gState->hotControl == buttonId || Internal::gState->hotControl == colorPicker) {
			outline = s->textAreaOutline_Hot;
			background = s->textAreaBg_Hot;
		}
		else if (Internal::gState->activeControl == buttonId || Internal::gState->activeControl == colorPicker) {
			outline = s->textAreaOutline_Active;
			background = s->textAreaBg_Active;
		}

		if (Internal::gState->activeControl == colorPicker && Internal::gState->lastActiveControl != colorPicker) {
			displayColor = Internal::gState->lastSelectedColor;
		}
		
		if (Internal::gState->activeControl == colorPicker) {
			Internal::gState->colorPickerActive = true;
			Internal::gState->colorPickerArea = colorBox;

			displayColor = Internal::gState->lastSelectedColor;
			displayColor = ColorPicker(colorPicker, colorBox, displayColor);

			Internal::gState->lastSelectedColor = displayColor;
		}
		
		outline.r += tint.r;
		outline.g += tint.g;
		outline.b += tint.b;
		background.r += tint.r;
		background.g += tint.g;
		background.b += tint.b;

		Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, outline.r, outline.g, outline.b);
		Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2, background.r, background.g, background.b);
		
		if (!disabled) {
			rgb inColorRgb = hsv2rgb(displayColor);
			Draw2D::DrawRect(screenPos.x + 3, screenPos.y + 3, screenPos.w - 6, screenPos.h - 6, inColorRgb.r, inColorRgb.g, inColorRgb.b);
		}

		if (Internal::gState->lastActiveControl == colorPicker && Internal::gState->activeControl != colorPicker) {
			return Internal::gState->lastSelectedColor;
		}
		return inColor;
	}

	void ClearActiveTextAreas() {
		Internal::gState->grabKeyboardOnOrAfter = 0;
	}
	
	const char* TextArea(const Rect& screenPos, const char* inputText, const char* label,
		bool disabled, bool fraction, bool integer, bool animated, bool interpolated, bool dirty) {
		PlatformAssert(inputText != 0, __LOCATION__);

		const StyleSheet* s = Internal::gState->style;
		i32 widgetId = Internal::gState->PositiveId(screenPos);

		bool kbdActivate = false;
		if (Internal::gState->grabKeyboardOnOrAfter > 0) {
			if (widgetId == Internal::gState->grabKeyboardOnOrAfter) {
				Internal::gState->grabKeyboardOnOrAfter = 0;
				kbdActivate = true;
			}
		}

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		vec2 mouse = vec2(_mouseX / dpi, _mouseY / dpi);

		const char* string = inputText;
		u32 inputTextLength = Internal::StrLen(string);
		Draw2D::Size inputTextSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, string);

		bool return_hit = false;

		Internal::TextAreaInstance* textArea = 0;
		if (!disabled &&(Internal::MakeHotOrActive(widgetId, screenPos, mouse, 0) || kbdActivate)) {
			if (Internal::gState->textAreaA.widgetId == widgetId || Internal::gState->textAreaB.widgetId == widgetId) {
				// Claim text area (guaranteed to exist at this point)
				if (Internal::gState->textAreaA.widgetId == widgetId) {
					textArea = &Internal::gState->textAreaA;
				}
				else if (Internal::gState->textAreaB.widgetId == widgetId) {
					textArea = &Internal::gState->textAreaB;
				}
				PlatformAssert(textArea != 0, __LOCATION__);

				// Select which text to work with
				if (textArea != 0) {
					string = textArea->buffer;
				}
				else {
					string = inputText;
				}
				inputTextLength = Internal::StrLen(string);
				inputTextSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, string);

				// Select everything on double click
				if (Internal::gState->lastTextDoubleClickTimer < IMGUI_DOUBLECLICK_SECOND) {
					textArea->karratIndex = 0;
					textArea->selection = inputTextLength;
				}
				else {
					Internal::gState->lastTextDoubleClickTimer = 0.0f;
				}
			}
			else {
				if (Internal::gState->textAreaA.widgetId == 0) {
					textArea = &Internal::gState->textAreaA;

					if (Internal::gState->textAreaB.widgetId != 0) {
						Internal::gState->textAreaB.state = Internal::TextAreaState::Shutdown;
					}
				}
				else {
					Internal::gState->textAreaA.state = Internal::TextAreaState::Shutdown;
					PlatformAssert(Internal::gState->textAreaB.widgetId == 0, __LOCATION__);
					textArea = &Internal::gState->textAreaB;
				}

				textArea->widgetId = widgetId;
				textArea->state = Internal::TextAreaState::Activated;
				textArea->karratIndex = 0;// stringLength > 0 ? stringLength - 1 : 0;
				textArea->selection = Internal::StrLen(inputText); // 0 to string length is select everything
				textArea->Reserve(inputTextLength);
				textArea->Set(inputText, textArea->selection);

				// Select which text to work with
				if (textArea != 0) {
					string = textArea->buffer;
				}
				else {
					string = inputText;
				}
				inputTextLength = Internal::StrLen(string);
				inputTextSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, string);
			}
		}
		else {
			if (Internal::gState->textAreaA.widgetId == widgetId || Internal::gState->textAreaB.widgetId == widgetId) {
				if (Internal::gState->textAreaA.widgetId == widgetId) {
					textArea = &Internal::gState->textAreaA;
				}
				else if (Internal::gState->textAreaB.widgetId == widgetId) {
					textArea = &Internal::gState->textAreaB;
				}
			}

			// Select which text to work with
			if (textArea != 0) {
				string = textArea->buffer;
			}
			else {
				string = inputText;
			}
			inputTextLength = Internal::StrLen(string);
			inputTextSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, string);

			if (textArea != 0) {
				textArea->state = Internal::TextAreaState::Idle;
				if (Internal::gState->GetPointerPressed()) {
					// Shutdown if mouse clicked outside
					if (!Internal::gState->virtualKeyPressed) {
						if (!Contains(screenPos, Imgui::Point(mouse.x, mouse.y))) {
							textArea->state = Internal::TextAreaState::Shutdown;
						}
						// Move karrat if mouse clicked inside
						else {
							vec2 adjustedMouse = mouse;
						
							if (inputTextSize.w >= screenPos.w - 6) { // To keep karrat on screen :(
								Draw2D::Rect distanceToKarrat = Draw2D::MeasureSubString(Internal::gState->interfaceFont, s->textAreaFontSize, string, 0, textArea->karratIndex);
								float string_x = screenPos.x + screenPos.w - 5 - inputTextSize.w;
								float karrat_x = string_x + distanceToKarrat.w;

								if (karrat_x < screenPos.x) {
									float delta = screenPos.x - karrat_x;
									delta += 5; // Arbitrary
									adjustedMouse.x -= delta;
								}
								else if (karrat_x > screenPos.x + screenPos.w) {
									PlatformAssert(false, __LOCATION__);
								}
							}
							i32 index = Internal::GetStringIndexUnderMouse(string, inputTextLength, screenPos, inputTextSize, adjustedMouse);
							textArea->karratIndex = index;
							textArea->selection = 0;
						}
					}
				}
				// If the mouse is being dragged
				else if (!Internal::gState->GetPointerUp()) {
					if (!Internal::gState->virtualKeyPressed) {
						vec2 fakeMouse = mouse;
						fakeMouse.y = screenPos.y;
						if (inputTextSize.w >= screenPos.w - 6) { // To keep karrat on screen :(
							Draw2D::Rect distanceToKarrat = Draw2D::MeasureSubString(Internal::gState->interfaceFont, s->textAreaFontSize, string, 0, textArea->karratIndex);
							float string_x = screenPos.x + screenPos.w - 5 - inputTextSize.w;
							float karrat_x = string_x + distanceToKarrat.w;

							if (karrat_x < screenPos.x) {
								float delta = screenPos.x - karrat_x;
								delta += 5; // Arbitrary
								fakeMouse.x -= delta;
							}
							else if (karrat_x > screenPos.x + screenPos.w) {
								PlatformAssert(false, __LOCATION__);
							}
						}
						i32 index = Internal::GetStringIndexUnderMouse(string, inputTextLength, screenPos, inputTextSize, fakeMouse);
						textArea->selection = index - textArea->karratIndex;
					}
				}
			}
		}
		
		// Handle text input. Text area, karrat and selection are all done by now
		bool shift = KeyboardDown(KeyboardCodeShift);
		if (Internal::gState->fakeShift || Imgui::GetFakeCapsToggle()) {
			shift = true;
		}
		bool control = KeyboardDown(KeyboardCodeControl);
		if (Internal::gState->fakeCtrl) {
			control = true;
		}

		if (textArea != 0) {
			if (textArea->state == Internal::TextAreaState::Activated) {
			}
			else if (textArea->state == Internal::TextAreaState::Idle) {
				{ // Process text input
					u32 keyCode = ConsumeKeyQueue();
					bool processed_key = false;
					while (keyCode != KeyboardCodeLeftMouse) {
						Internal::gState->ateKeyboard = true;
						processed_key = true;
						char ascii = ScanCodeToAscii(keyCode, shift);

						bool skipCharacter = false;
						if (fraction || integer) {
							skipCharacter = true;
							if (ascii >= '0' && ascii <= '9') {
								skipCharacter = false;
							}
							else if (ascii == '.') {
								if (integer) {
									skipCharacter = true;
								}
								else {
									skipCharacter = false;
									for (i32 i = 0; i < textArea->length; ++i) {
										if (textArea->buffer[i] == '.') {
											skipCharacter = true;
											break;
										}
									}
								}
							}
							else if (ascii == '-') {
								/*if (integer) {
									skipCharacter = true;
								}
								else */if (textArea->length == 0) {
									skipCharacter = false;
								}
								else if (textArea->selection != 0) {
									skipCharacter = false;
								}
								else if (textArea->karratIndex == 0) {
									if (textArea->length > 0) {
										if (textArea->buffer[0] != '-') {
											skipCharacter = false;
										}
									}
								}
							}
						}

						// Make sure the text areas buffer has enough room
						if (textArea->length + 2 >= textArea->capacity) {
							if (textArea->capacity < 16) {
								textArea->capacity = 16;
							}
							textArea->capacity *= 2;
							textArea->buffer = (char*)MemRealloc(textArea->buffer, textArea->capacity);
						}

						if (control) {
							if (ascii == 'c' || ascii == 'C') {
								char* toCopy = Internal::GetSubString(textArea->buffer, textArea->karratIndex, textArea->karratIndex + textArea->selection);
								if (toCopy != 0) {
									WriteClipboard(toCopy);
									MemRelease(toCopy);
								}
							}
							else if (ascii == 'v' || ascii == 'V') {
								char* toPaste = ReadClipboard();
								if (toPaste != 0) {
									i32 len = Internal::StrLen(toPaste);

									// Make sure text area has enough room
									if (textArea->capacity <= len) {
										textArea->capacity = len + (len / 2);
										textArea->buffer = (char*)MemRealloc(textArea->buffer, textArea->capacity);
									}

									// Copy text
									for (i32 i = 0; i < len; ++i) {
										textArea->buffer[i] = toPaste[i];
									}
									textArea->buffer[len] = 0;
									textArea->length = len;

									// Update internal numbers
									inputTextLength = Internal::StrLen(string);
									inputTextSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, string);

									// Move karrat to end
									textArea->karratIndex = inputTextLength;
									textArea->selection = 0;
								}

							}
							else if (ascii == 'x' || ascii == 'X') {
								char* toCopy = Internal::GetSubString(textArea->buffer, textArea->karratIndex, textArea->karratIndex + textArea->selection);
								if (toCopy != 0) {
									WriteClipboard(toCopy);
									MemRelease(toCopy);
								}
							}
							else if (ascii == 'a' || ascii == 'A') {
								textArea->karratIndex = 0;
								textArea->selection = inputTextLength;
							}
						}

						// Handle text clear if there is any text selected
						bool clearedText = false;
						if (textArea->selection != 0 && ascii != '\t' && ascii != '\n' && ascii != 0) {
							if (!control || (control && (ascii == 'X' || ascii == 'x'))) {
								if ((fraction || integer) && skipCharacter) {
									// Ignore key. I think that's the right call here
								}
								else {
									i32 startIndex = textArea->karratIndex;
									i32 endIndex = textArea->karratIndex + textArea->selection;
									if (endIndex < startIndex) {
										i32 tmp = endIndex;
										endIndex = startIndex;
										startIndex = tmp;
									}
									PlatformAssert(endIndex != startIndex, __LOCATION__);

									Internal::RemoveRangeFromString(textArea->buffer, textArea->length, startIndex, endIndex);
									i32 lengthRemoved = endIndex - startIndex;
									textArea->length -= lengthRemoved;
									textArea->selection = 0;
									textArea->karratIndex = startIndex;
									clearedText = true;
								}
							} 
						}

						
						// Append character after karrat
						if (control) {
							// Already handled
						}
						else if (ascii == '\n') {
							textArea->karratIndex = 0;
							textArea->selection = inputTextLength;

							return_hit = true;
						}
						else if (ascii == '\t') {
							if (shift) {
								Internal::gState->grabKeyboardOnOrAfter = widgetId - 1; // Let the next box grab it
								if (Internal::gState->grabKeyboardOnOrAfter <= 0) {
									Internal::gState->grabKeyboardOnOrAfter = -1;
								}
								textArea->state = Internal::TextAreaState::Shutdown; // Shut us down
							}
							else {
								Internal::gState->grabKeyboardOnOrAfter = widgetId + 1; // Let the next box grab it
								textArea->state = Internal::TextAreaState::Shutdown; // Shut us down
							}
						}
						else if (ascii == '\b') {
							if (!clearedText) {
								if (textArea->karratIndex > 0) {
									Internal::RemoveRangeFromString(textArea->buffer, textArea->length, textArea->karratIndex - 1, textArea->karratIndex);
									textArea->buffer[--textArea->length] = 0;
									textArea->karratIndex -= 1;
									textArea->selection = 0;
								}
							}
						}
						else if (ascii == '\a') {
							if (!clearedText) {
								if (textArea->length > 0) {
									Internal::RemoveRangeFromString(textArea->buffer, textArea->length, textArea->karratIndex, textArea->karratIndex + 1);
									textArea->buffer[--textArea->length] = 0;
									textArea->selection = 0;
								}
								if (textArea->karratIndex > textArea->length - 1) {
									textArea->karratIndex = textArea->length - 1;
								}
							}
						}
						else if (ascii != 0) {
							if (!skipCharacter) {
								// Shift string to make room for the new character
								if (textArea->karratIndex < textArea->length && textArea->karratIndex >= 0) {
									// Selection here is alweays 0, so we can just use karratINdex
									// Length + 1 so it overrides null?
									for (int i = textArea->length; i >= textArea->karratIndex; --i) {
										if (i > 0) {
											textArea->buffer[i] = textArea->buffer[i - 1];
										}
									}
								}

								// Set karrat to current character
								textArea->buffer[textArea->karratIndex++] = ascii;
								textArea->buffer[++textArea->length] = 0;

								// Show karrat if a character was inserted
								Internal::gState->blink = true;
								Internal::gState->blinkTimer = 0.0f;
							}
						}

						keyCode = ConsumeKeyQueue();
					}

					if (processed_key) {
						inputTextLength = Internal::StrLen(string);
						inputTextSize = Draw2D::MeasureString(Internal::gState->interfaceFont, s->textAreaFontSize, string);
					}
				}
				{ // Process arrow nagivation
					i32 arrowStartIndex = textArea->karratIndex;
					i32 arrowEndIndex = textArea->karratIndex + textArea->selection;
					
					bool leftArrowPressed = KeyboardPressed(KeyboardCodeLeft);
					if (Internal::gState->fakeLeftPressed) {
						leftArrowPressed = true;
					}
					bool leftArrowDown = KeyboardDown(KeyboardCodeLeft);
					
					bool rightArrowPressed = KeyboardPressed(KeyboardCodeRight);
					if (Internal::gState->fakeRightPressed) {
						rightArrowPressed = true;
					}

					bool rightArrowDown = KeyboardDown(KeyboardCodeRight);

					if (leftArrowPressed || rightArrowPressed) {
						Internal::gState->pulseTimer = 0.0f;
					}

					if (leftArrowDown || rightArrowDown) {
						Internal::gState->blink = true;
						Internal::gState->blinkTimer = 0.0f;
					}

					if (leftArrowPressed || leftArrowDown) {
						if (Internal::gState->pulse || leftArrowPressed) {
							if (shift) {
								if (arrowEndIndex < arrowStartIndex) {
									textArea->karratIndex = textArea->karratIndex + textArea->selection;
									textArea->selection *= -1;
								}
								PlatformAssert(textArea->selection >= 0, __LOCATION__);
								if (textArea->karratIndex > 0) {
									textArea->karratIndex -= 1;
									textArea->selection += 1;
								}
							}
							else if (textArea->selection == 0 && inputTextLength > 0) {
								if (--textArea->karratIndex < 0) {
									textArea->karratIndex = 0;
								}
							}
							else if (textArea->selection != 0) {
								if (arrowEndIndex < arrowStartIndex) {
									textArea->karratIndex = textArea->karratIndex + textArea->selection;
								}
								textArea->selection = 0;
							}
						}
					}
					else if (rightArrowPressed || rightArrowDown) {
						 if (Internal::gState->pulse || rightArrowPressed) {
							 if (shift) {
								 if (arrowEndIndex < arrowStartIndex) {
									 textArea->karratIndex = textArea->karratIndex + textArea->selection;
									 textArea->selection *= -1;
								 }
								 PlatformAssert(textArea->selection >= 0, __LOCATION__);
								 if (textArea->karratIndex + textArea->selection < inputTextLength) {
									 textArea->selection += 1;
								 }
							 }
							 else if (textArea->selection == 0 && inputTextLength > 0) {
								if (++textArea->karratIndex >= inputTextLength) {
									textArea->karratIndex = inputTextLength;
								}
							}
							else if (textArea->selection != 0) {
								if (arrowEndIndex > arrowStartIndex) {
									textArea->karratIndex = textArea->karratIndex + textArea->selection;
								}
								textArea->selection = 0;
							}
						}
					}
					else if (KeyboardPressed(KeyboardCodeUp)) {
						if (shift) {
							textArea->karratIndex = 0;
							textArea->selection = inputTextLength;
						}
						else {
							textArea->karratIndex = 0;
							textArea->selection = 0;
						}
					}
					else if (KeyboardPressed(KeyboardCodeDown)) {
						if (shift) {
							textArea->karratIndex = 0;
							textArea->selection = inputTextLength;
						}
						else {
							if (inputTextLength > 0) {
								textArea->karratIndex = inputTextLength;
							}
							else {
								textArea->karratIndex = 0;
							}
							textArea->selection = 0;
						}
					}
				}
			}
			else if (textArea->state == Internal::TextAreaState::Shutdown) {
			}
		}

		{ // Drawing
			StyleColor bgColor = s->textAreaBg_Normal;
			StyleColor txtColor = s->textAreaFont_Normal;
			StyleColor outlinColor = s->textAreaOutline_Normal;
			StyleColor labelColor = s->textAreaLabel_Normal;

			if (disabled) {
				bgColor = s->textAreaBg_Disabled;
				txtColor = s->textAreaFont_Disabled;
				outlinColor = s->textAreaOutline_Disabled;
				labelColor = s->textAreaLabel_Disabled;
			}
			else if (Internal::gState->activeControl == widgetId || textArea != 0) {
				bgColor = s->textAreaBg_Active;
				txtColor = s->textAreaFont_Active;
				outlinColor = s->textAreaOutline_Active;
				labelColor = s->textAreaLabel_Active;
			}
			else if (Internal::gState->hotControl == widgetId) {
				bgColor = s->textAreaBg_Hot;
				txtColor = s->textAreaFont_Hot;
				outlinColor = s->textAreaOutline_Hot;
				labelColor = s->textAreaLabel_Hot;
			}

			if (animated && dirty) {
				bgColor.r += s->textAreaBg_DirtyTint.r;
				bgColor.g += s->textAreaBg_DirtyTint.g;
				bgColor.b += s->textAreaBg_DirtyTint.b;
				outlinColor.r += s->textAreaOutline_DirtyTint.r;
				outlinColor.g += s->textAreaOutline_DirtyTint.g;
				outlinColor.b += s->textAreaOutline_DirtyTint.b;
				txtColor.r += s->textAreaFont_DirtyTint.r;
				txtColor.g += s->textAreaFont_DirtyTint.g;
				txtColor.b += s->textAreaFont_DirtyTint.b;
			}
			else if (animated && interpolated) {
				bgColor.r += s->textAreaBg_InterpolatedTint.r;
				bgColor.g += s->textAreaBg_InterpolatedTint.g;
				bgColor.b += s->textAreaBg_InterpolatedTint.b;
				outlinColor.r += s->textAreaOutline_InterpolatedTint.r;
				outlinColor.g += s->textAreaOutline_InterpolatedTint.g;
				outlinColor.b += s->textAreaOutline_InterpolatedTint.b;
				txtColor.r += s->textAreaFont_InterpolatedTint.r;
				txtColor.g += s->textAreaFont_InterpolatedTint.g;
				txtColor.b += s->textAreaFont_InterpolatedTint.b;
			}
			else if (animated) {
				bgColor.r += s->textAreaBg_AnimatedTint.r;
				bgColor.g += s->textAreaBg_AnimatedTint.g;
				bgColor.b += s->textAreaBg_AnimatedTint.b;
				outlinColor.r += s->textAreaOutline_AnimatedTint.r;
				outlinColor.g += s->textAreaOutline_AnimatedTint.g;
				outlinColor.b += s->textAreaOutline_AnimatedTint.b;
				txtColor.r+= s->textAreaFont_AnimatedTint.r;
				txtColor.g+= s->textAreaFont_AnimatedTint.g;
				txtColor.b+= s->textAreaFont_AnimatedTint.b;
			}

			// Draw bg
			Draw2D::DrawRect(screenPos.x, screenPos.y, screenPos.w, screenPos.h, outlinColor.r, outlinColor.g, outlinColor.b);
			Draw2D::DrawRect(screenPos.x + 1, screenPos.y + 1, screenPos.w - 2, screenPos.h - 2, bgColor.r, bgColor.g, bgColor.b);
			if (label != 0) {
				Draw2D::DrawString(Internal::gState->labelFont, s->textAreaLabelSize, screenPos.x + 3, screenPos.y, label, labelColor.r, labelColor.g, labelColor.b);
			}

			float string_x = screenPos.x + screenPos.w - 5 - inputTextSize.w;
			float string_y = screenPos.y + 2;

			bool clip = false;
			if (inputTextSize.w > screenPos.w - 6) {
				clip = true;
				Draw2D::PushClip(screenPos.x + 3, screenPos.y, screenPos.w - 6, screenPos.h);
			}

			// Adjust string position so karrat is visible
			Draw2D::Rect distanceToKarrat(0, 0, 0, 0);
			if (textArea != 0) {
				distanceToKarrat = Draw2D::MeasureSubString(Internal::gState->interfaceFont, s->textAreaFontSize, string, 0, textArea->karratIndex);
				float karrat_x = string_x + distanceToKarrat.w;

				if (karrat_x < screenPos.x) {
					float delta = screenPos.x - karrat_x;
					delta += 5; // Arbitrary
					string_x += delta;
				}
				else if (karrat_x > screenPos.x + screenPos.w) {
					PlatformAssert(false, __LOCATION__);
				}
			}

			// Draw selection
			if (textArea != 0 && textArea->selection != 0) {
				i32 selectionStart = textArea->karratIndex;
				i32 selectionEnd = selectionStart + textArea->selection;
				if (selectionEnd < selectionStart) {
					i32 tmp = selectionEnd;
					selectionEnd = selectionStart;
					selectionStart = tmp;
				}
				if (selectionStart >= 0) {
					Draw2D::Rect substring = Draw2D::MeasureSubString(Internal::gState->interfaceFont, s->textAreaFontSize, string, selectionStart, selectionEnd - selectionStart);
					Draw2D::DrawRect(string_x + substring.x, string_y + substring.y, substring.w, substring.h, s->textAreaSelectionColor.r, s->textAreaSelectionColor.g, s->textAreaSelectionColor.b);
				}
			}

			// Draw string
			string_y += s->textAreaFontSize;
			Draw2D::DrawString(Internal::gState->interfaceFont, s->textAreaFontSize, string_x, string_y, string, txtColor.r, txtColor.g, txtColor.b);
			string_y -= s->textAreaFontSize;

			// Draw karrat
			if (textArea != 0 && textArea->karratIndex >= 0 && textArea->selection == 0) {
				Draw2D::Rect substring = Draw2D::MeasureSubString(Internal::gState->interfaceFont, s->textAreaFontSize, string, 0, textArea->karratIndex);

				f32 karratPosX = string_x + substring.x + substring.w - 1;
				f32 karratPosY = string_y + 1;
				f32 karratHeight = substring.h - 4;

				if (Internal::gState->blink) {
					Draw2D::DrawRect(karratPosX - 3, karratPosY - 1, 3 + 1 + 3, 1, s->karratColor.r, s->karratColor.g, s->karratColor.b);
					Draw2D::DrawRect(karratPosX, karratPosY, 1, karratHeight, s->karratColor.r, s->karratColor.g, s->karratColor.b);
					Draw2D::DrawRect(karratPosX - 3, karratPosY + karratHeight, 3 + 1 + 3, 1, s->karratColor.r, s->karratColor.g, s->karratColor.b);
				}
			}

			if (clip) {
				Draw2D::PopClip();
			}
		}

		if (textArea != 0 && textArea->state == Internal::TextAreaState::Shutdown) {
			textArea->widgetId = 0;
			return textArea->buffer;
		}

		if (textArea != 0 && return_hit) {
			return textArea->buffer;
		}

		return inputText;
	}

	void Initialize(u32 interfaceFont, u32 widgetFont, u32 labelFont) {
		Internal::gState = (Internal::State*)MemAlloc(sizeof(Internal::State));
		MemClear(Internal::gState, sizeof(Internal::State));
		Internal::gState->widgetFont = widgetFont;
		Internal::gState->interfaceFont = interfaceFont;
		Internal::gState->labelFont = labelFont;
		new (&Internal::gState->modalPopups) Vector<Internal::PopupMenuInfo>();
	}

	void Shutdown() {
		if (Internal::gState->maxPopupMenus != 0) {
			for (u32 i = 0; i < Internal::gState->maxPopupMenus; ++i) {
				Internal::gState->modalPopups[i].items.~Vector();
			}
		}
		Internal::gState->modalPopups.~Vector();

		if (Internal::gState->textAreaA.buffer != 0) {
			MemRelease(Internal::gState->textAreaA.buffer);
		}
		if (Internal::gState->textAreaB.buffer != 0) {
			MemRelease(Internal::gState->textAreaB.buffer);
		}
#if PLATFORM_DEBUG
		Internal::RectBlock* iter = Internal::gState->positiveWidgets.next;
		while (iter != 0) {
			Internal::RectBlock* del = iter;
			iter = iter->next;
			MemRelease(del);
		}
		iter = Internal::gState->negativeWidgets.next;
		while (iter != 0) {
			Internal::RectBlock* del = iter;
			iter = iter->next;
			MemRelease(del);
		}
#endif
		MemRelease(Internal::gState);
	}

	void BeginFrame(float dpi, const struct StyleSheet& style) {
		Internal::gState->dpi = dpi;
		Internal::gState->tooltipIcon = -1;
		Internal::gState->tooltipLabel = 0;
		Internal::gState->style = &style;
		Internal::gState->positiveIdGenerator = 0;
		Internal::gState->negativeIdGenerator = 0;
		if (Internal::gState->lastHotControl != Internal::gState->hotControl) {
			Internal::gState->hotControlTimer = 0.0f;
		}
		Internal::gState->lastHotControl = Internal::gState->hotControl;
		Internal::gState->hotControl = 0;

		// Update pointer state
		if (Internal::gState->activeInputMethod == IMGUI_ACTIVE_INPUT_NONE) {
			if (MousePressed(MouseButtonLeft)) {
				Internal::gState->activeInputMethod = IMGUI_ACTIVE_INPUT_MOUSE;
			}
			else {
				for (u32 i = 0; i < 5; ++i) {
					if (TouchPressed(i)) {
						Internal::gState->activeInputMethod = IMGUI_ACTIVE_INPUT_TOUCH1 + i;
					}
				}
			}
		}
	}

	bool EndFrame(u32 screenWidth, u32 screenHeight) {
		const StyleSheet* s = Internal::gState->style;

		f32 _mouseX = Internal::gState->GetPointerX(); // In screen space, needs to be divided 
		f32 _mouseY = Internal::gState->GetPointerY(); // by dpi to move into design space.
		f32 dpi = Internal::gState->dpi;
		Point mouse(_mouseX / dpi, _mouseY / dpi);
		
		Internal::gState->lastBeingModalPopupContext = 0;
		Internal::gState->lastBeingModalPopupActivation = 0;
		Internal::gState->lastModalId = 0;

		Internal::gState->fakeLeftPressed = false;
		Internal::gState->fakeRightPressed = false;

		while (Internal::gState->numPopupMenus > 0) {
			Internal::PopupMenuRender(Internal::gState->modalPopups[Internal::gState->numPopupMenus - 1]);
			Internal::gState->numPopupMenus -= 1;
		}

		if (Internal::gState->tooltipIcon >= 0 && Internal::gState->tooltipIcon < 256) {
			f32 x = Internal::gState->GetPointerX();
			f32 y = Internal::gState->GetPointerY();
			f32 dpi = Internal::gState->dpi;
			
			const StyleColor& back = s->tooltipIconBGColor;
			const StyleColor& fore = s->tooltipIconFGColor;
			float offset = MathMaxF(1.0f, MathFloor(dpi));
			
			Draw2D::DrawCodePoint(Internal::gState->widgetFont, IMGUI_TOOLTIP_SIZE, (x + (3 * dpi)) / dpi, (y + (3 * dpi)) / dpi, Internal::gState->tooltipIcon, back.r, back.g, back.b, 1.0f);
			Draw2D::DrawCodePoint(Internal::gState->widgetFont, IMGUI_TOOLTIP_SIZE, (x + (3 * dpi) + offset) / dpi, (y + (3 * dpi) + offset) / dpi, Internal::gState->tooltipIcon, fore.r, fore.g, fore.b, 1.0f);
		}

		if (Internal::gState->tooltipLabel != 0) {
			f32 dpi = Internal::gState->dpi;

			f32 x = Internal::gState->GetPointerX() / dpi;
			f32 y = Internal::gState->GetPointerY() / dpi;

			Draw2D::Size stringSize = Draw2D::MeasureString(Internal::gState->interfaceFont, IMGUI_TOOLTIP_SIZE - 2, Internal::gState->tooltipLabel);
			stringSize.w += 2;
			if (x + stringSize.w > (f32)screenWidth / dpi) {
				x -= stringSize.w;
			}

			Draw2D::DrawRect(x - 1, y - IMGUI_TOOLTIP_SIZE - 1, stringSize.w + 2, IMGUI_TOOLTIP_SIZE + 2, s->tooltipIconBGColor.r, s->tooltipIconBGColor.g, s->tooltipIconBGColor.b);
			Draw2D::DrawRect(x, y - IMGUI_TOOLTIP_SIZE, stringSize.w, IMGUI_TOOLTIP_SIZE, s->tooltipTextBGColor.r, s->tooltipTextBGColor.g, s->tooltipTextBGColor.b);
			Draw2D::DrawString(Internal::gState->interfaceFont, IMGUI_TOOLTIP_SIZE - 2, x + 2, y - 2, Internal::gState->tooltipLabel, s->tooltipTextColor.r, s->tooltipTextColor.g, s->tooltipTextColor.b);
		}

		if (Internal::gState->lastActiveControl != Internal::gState->activeControl) {
			Internal::gState->activeControlTimer = 0.0f;
		}
		Internal::gState->lastActiveControl = Internal::gState->activeControl;

		i32 _active = Internal::gState->activeControl;

		if (Internal::gState->grabKeyboardOnOrAfter >= Internal::gState->positiveIdGenerator) {
			Internal::gState->grabKeyboardOnOrAfter = 1;
		}
		else if (Internal::gState->grabKeyboardOnOrAfter < 0) {
			Internal::gState->grabKeyboardOnOrAfter = Internal::gState->positiveIdGenerator;
			if (Internal::gState->grabKeyboardOnOrAfter < 0) {
				Internal::gState->grabKeyboardOnOrAfter = 0;
			}
		}

		if (Internal::gState->GetPointerReleased()) {
			if (Internal::gState->modalControl != 0) {
				Internal::gState->activeControl = Internal::gState->modalControl;
			}
			else {
				if (Internal::gState->colorPickerActive) {
					if (!Contains(Internal::gState->colorPickerArea, mouse)) {
						Internal::gState->activeControl = 0;
					} // else keep the color picker active
				}
				else {
					Internal::gState->activeControl = 0;
				}
			}
			Internal::gState->modalControl = 0;
		}

		if (Internal::gState->colorPickerActive) {
			DrawColorPicker(Internal::gState->colorPickerArea, Internal::gState->lastSelectedColor);
			Internal::gState->colorPickerActive = false;
		}

		// THIS HAS TO HAPPEN LAST!

		if (Internal::gState->activeInputMethod != IMGUI_ACTIVE_INPUT_NONE) {
			if (Internal::gState->activeInputMethod == IMGUI_ACTIVE_INPUT_MOUSE) {
				if (MouseReleased(MouseButtonLeft)) {
					Internal::gState->activeInputMethod = IMGUI_ACTIVE_INPUT_NONE;
				}
			}
			else {
				int touchIndex = Internal::gState->activeInputMethod - IMGUI_ACTIVE_INPUT_TOUCH1;
				if (TouchReleased(touchIndex)) {
					Internal::gState->activeInputMethod = IMGUI_ACTIVE_INPUT_NONE;
				}
			}
		}

#if PLATFORM_DEBUG
		if (Internal::gState->debugOverlayEnabled) {
			if (Internal::gState->hotControl != 0) {
				Rect hotRect = Internal::gState->Debug_GetWidgetRect(Internal::gState->hotControl);
				Draw2D::DrawRect(hotRect.x, hotRect.y, hotRect.w, hotRect.h, 1.0f, 0.0f, 0.0f, 0.3f);
			}

			if (_active != 0) {
				Rect activeRect = Internal::gState->Debug_GetWidgetRect(_active);
				Draw2D::DrawRect(activeRect.x, activeRect.y, activeRect.w, activeRect.h, 0.0f, 1.0f, 0.0f, 0.3f);
			}
		}
#endif

		bool result = Internal::gState->ateKeyboard;
		Internal::gState->ateKeyboard = false;
		ClearKeyQueue();
		return result;
	}

	void TickFrame(float deltaTime) {
		Internal::gState->pulse = false;
		Internal::gState->pulseTimer += deltaTime;
		Internal::gState->lastTextDoubleClickTimer += deltaTime;
		Internal::gState->longPulse = false;
		Internal::gState->longPulseTimer += deltaTime;

		if (Internal::gState->activeControl != 0) {
			Internal::gState->activeControlTimer += deltaTime;
		}
		else {
			Internal::gState->activeControlTimer = 0.0f;
		}

		if (Internal::gState->hotControl != 0) {
			Internal::gState->hotControlTimer += deltaTime;
		}
		else {
			Internal::gState->hotControlTimer = 0.0f;
		}

		if (Internal::gState->pulseTimer > float(IMGUI_PULSE_MS) / 1000.0f) {
			Internal::gState->pulseTimer = 0;
			Internal::gState->pulse = true;
		}

		if (Internal::gState->longPulseTimer > float(IMGUI_LONG_PULSE_MS) / 1000.0f) {
			Internal::gState->longPulseTimer = 0;
			Internal::gState->longPulse = true;
		}

		float blinkLimit = float(IMGUI_BLINK_MS) / 1000.0f;
		if (Internal::gState->blink) {
			Internal::gState->blinkTimer += deltaTime;
			if (Internal::gState->blinkTimer > blinkLimit) {
				Internal::gState->blinkTimer = 0;
				Internal::gState->blink = !Internal::gState->blink;
			}
		}
		else {
			blinkLimit *= -1.0f;
			Internal::gState->blinkTimer -= deltaTime;
			if (Internal::gState->blinkTimer < blinkLimit) {
				Internal::gState->blinkTimer = 0;
				Internal::gState->blink = !Internal::gState->blink;
			}
		}

		if (KeyboardPressed(KeyboardCodeTilde)) {
			Internal::gState->debugOverlayEnabled = !Internal::gState->debugOverlayEnabled;
		}
		Internal::gState->virtualKeyPressed = false;
	}
}