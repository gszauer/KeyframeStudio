export default class UIGlobals {
    static Atlas = "Atlas";
    static Solid = "solid.png";

    static Hot = null;
    static Active = null;

    static Icons = {
        ChevronDown: "down.png"
    };
    // TODO: Move the icons in there^
    static IconHand = "IcondHand.png";
    static IconGrid = "IconGrid.png";
    static IconHome = "IconHome.png";
    static IconMove = "IconMove.png";
    static IconRotate = "IconRotate.png";
    static IconScale = "IconScale.png";
    static IconZoomIn = "IconZoomIn.png";
    static IconZoomOut = "IconZoomOut.png";

    static Fonts = {

    };
    // TODO: Make a container for these
    static Font50 = "AdobeClean13";
    static Font75 = "AdobeClean15";
    static Font100 = "AdobeClean17";
    static Font200 = "AdobeClean19";
    static Font300 = "AdobeClean22";
    static Font400 = "AdobeClean24";
    static Font500 = "AdobeClean27";

    static Layers = {
        // TODO: Move the below elements into this container
    };
    static WidgetLayer = 50;
    static OverlayLayer = 100;

    static Sizes = {
        TopMenuHeight: 32,
        TopMenuMargin: 5,
        TopMenuPadding: 1,
        PopupMenuMargin: 10,
        EditorBarSeperatorHeight: 2,
        PopupMenuItemHeight: 25,
        PopupMenuDividerHeight: 2,
        PopupMenuDividerPadding: 3,
        PopupMenuBorderSize: 2,
        EditorBarHeight: 40,
        DropdownMenuHeight: 24,
        DrowdownBorderSize: 1,
        DropdownTextIndent: 5,
        CheckboxSize:21,
        CheckboxBorder: 1,
        CheckboxMargin: 4,
        TextboxHeight: 24,
        TextboxTextMargin: 4,
        TextBoxBorderSize: 1,
        ToolboxWidth: 42,
        ToolboxPadding: 1,
        ToolboxButtonSize: 28,
        ToolboxButtonIconSize: 20,
        ToolboxButtonPadding: 8,
        ToolboxButtonDividerHeight: 2,
        SplitViewSplitterSize: 3,
        TabHeight: 30,
        TabBorderSize: 2,
        TabMargin: 5,
        ScrollTrackSize: 20,
        ScrollTrackMinGripSize: 30,
        ScrollBorderSize: 2,

        InspectorTitleMargin: 10,
        InspectorTitleSkip: 5,
        ColorButtonBorderSize: 2,
        ColorButtonMarginSize: 3,
        ColorPickerDefaultSize: 204,
        ColorPickerInnerMargin: 10,
        ColorPickerHueTrackWidth: 20,
    };

    // https://spectrum.adobe.com/page/color-system/#Static-color-palette
    static Colors = {
        BackgroundLayer0: 0x1d1d1d, // Gray50,
        BackgroundLayer1: 0x262626, // Gray75
        BackgroundLayer2: 0x323232, // Gray 100
        BackgroundLayerAux: 0x3f3f3f, // Gray 200
        BorderDecorative: 0x3f3f3f, // Gray 200
        BorderFraming: 0x545454, // Gray 300
        BorderField: 0x707070, // Gray 400
        IconDisabled: 0x707070, // Gray 400
        TextDisabled: 0x909090, // Gray 500
        IconIllustration: 0x909090, // Gray 500
        ElementBorderTintIdle: 0xb2b2b2, // Gray 600
        ElementBorderTintHot: 0xd1d1d1, // Gray 700
        ElementBorderTintActive: 0xebebeb, // Gray 800
        TextSub: 0xd1d1d1, // Gray 700
        IconSub: 0xd1d1d1, // Gray 700
        Text: 0xebebeb, // Gray 800
        Icon: 0xebebeb, // Gray 800
        TextHeading: 0xffffff, // Gray 900
        TopMenuButtonIdle: 0x262626, // Gray75, Background1
        TopMenuButtonHot: 0x3f3f3f, // Gray 200
        TopMenuButtonActive: 0x545454, // Gray 300

        // https://spectrum.adobe.com/page/color-palette/
        Dark: {
            Gray50: 0x1d1d1d,
            Gray75: 0x262626,
            Gray100: 0x323232,
            Gray200: 0x3f3f3f,
            Gray300: 0x545454,
            Gray400: 0x707070,
            Gray500: 0x909090,
            Gray600: 0xb2b2b2,
            Gray700: 0xd1d1d1,
            Gray800: 0xebebeb,
            Gray900: 0xffffff,
            Blue100: 0x003877,
            Blue200: 0x00418a,
            Blue300: 0x004da3,
            Blue400: 0x0059c2,
            Blue500: 0x0367e0,
            Blue600: 0x1379f3,
            Blue700: 0x348ff4,
            Blue800: 0x54a3f6,
            Blue900: 0x72b7f9,
            Blue1000: 0x8fcafc,
            Blue1100: 0xaedbfe,
            Blue1200: 0xcce9ff,
            Blue1300: 0xe8f6ff,
            Green100: 0x044329,
            Green200: 0x004e2f,
            Green300: 0x005c38,
            Green400: 0x006c43,
            Green500: 0x007d4e,
            Green600: 0x008f5d,
            Green700: 0x12a26c,
            Green800: 0x2bb47d,
            Green900: 0x43c78f,
            Green1000: 0x5ed9a2,
            Green1100: 0x81e9b8,
            Green1200: 0xb1f4d1,
            Green1300: 0xdffaea
        }
    };
}