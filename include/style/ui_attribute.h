#pragma once

#include "../core/ui_core_type.h"
#include "../core/ui_string_view.h"


namespace LongUI {
    /// <summary>
    /// orientation
    /// </summary>
    enum AttributeOrient : bool {
        /// <summary>
        /// The orient horizontal
        /// </summary>
        Orient_Horizontal = false,
        /// <summary>
        /// The orient vertical
        /// </summary>
        Orient_Vertical = true,
    };
    /// <summary>
    /// attachment
    /// </summary>
    enum AttributeAttachment : bool {
        /// <summary>
        /// The attachment scroll
        /// </summary>
        Attachment_Scroll = false,
        /// <summary>
        /// The attachment fixed
        /// </summary>
        Attachment_Fixed = true,
    };
    /// <summary>
    /// direction
    /// </summary>
    enum AttributeDir : bool {
        /// <summary>
        /// The normal direction
        /// </summary>
        Dir_Normal = false,
        /// <summary>
        /// The reverse direction
        /// </summary>
        Dir_Reverse = true,
    };
    /// <summary>
    /// pack
    /// </summary>
    enum AttributePack : uint8_t {
        /// <summary>
        /// Child elements are placed starting from the left or top edge 
        /// of the box.If the box is larger than the total size of the 
        /// children, the extra space is placed on the right or bottom side.
        /// </summary>
        Pack_Start = 0,

        /// <summary>
        /// Extra space is split equally along each side of the child 
        /// elements, resulting the children being placed in the center of the box.
        /// </summary>
        Pack_Center,

        /// <summary>
        /// Child elements are placed on the right or bottom edge of 
        /// the box.If the box is larger than the total size of the 
        /// children, the extra space is placed on the left or top side.
        /// </summary>
        Pack_End
    };
    /// <summary>
    /// align
    /// </summary>
    enum AttributeAlign : uint8_t {
        /// <summary>
        /// 尽可能扩大侧轴
        /// This is the default value.The child elements are stretched to
        /// fit the size of the box.For a horizontal box, the children are
        /// stretched to be the height of the box.For a vertical box, the
        /// children are stretched to be the width of the box.
        ///
        /// if child element was fixed in other orientation, same as 'start'
        /// </summary>
        Align_Stretcht = 0,
        /// <summary>
        /// 向主轴起点对齐
        /// Child elements are aligned starting from the left or top 
        /// edge of the box.If the box is larger than the total size of 
        /// the children, the extra space is placed on the right or bottom side.
        /// </summary>
        Align_Start,
        /// <summary>
        /// 向主轴中央对齐
        /// Extra space is split equally along each side of the child elements, 
        /// resulting in the children being placed in the center of the box.
        /// </summary>
        Align_Center,
        /// <summary>
        /// 向主轴终点对齐
        /// Child elements are placed on the right or bottom edge of the 
        /// box.If the box is larger than the total size of the children, 
        /// the extra space is placed on the left or top side.
        /// </summary>
        Align_End,
        /// <summary>
        /// 向主轴基线对齐
        /// This value applies to horizontally oriented
        /// boxes only.It  causes the child elements to be aligned so that
        /// their text labels are lined up.
        /// </summary>
        Align_Baseline,
    };
    /// <summary>
    /// repeat 2-attr: [high-4bit(y) low-4bit(x)]
    /// </summary>
    enum AttributeRepeat : uint8_t {
        Repeat_Repeat = 0,                  // *-repeat
        Repeat_Space,                       // *-repeat
        Repeat_Round,                       // *-repeat
        Repeat_NoRepeat,                    // background-repeat
        Repeat_Stretch = Repeat_NoRepeat,   // border-image-repeat
        // ---------------
        Repeat_RepeatXOnly = Repeat_NoRepeat << 4 | Repeat_Repeat,
        Repeat_RepeatYOnly = Repeat_Repeat << 4 | Repeat_NoRepeat,
        Repeat_Stretch2 = Repeat_Stretch << 4 | Repeat_Stretch,
    };
    /// <summary>
    /// repeat for image
    /// </summary>
    enum AttributeImageRepeat : uint8_t {
        IRepeat_Stretch = 0,
        IRepeat_Repeat,
        IRepeat_Round,
        IRepeat_Space,
    };
    /// <summary>
    /// aligned box 
    /// </summary>
    enum AttributeBox : uint8_t {
        Box_BorderBox = 0,
        Box_PaddingBox,
        Box_ContentBox,
        BOX_COUNT,
    };
    /// <summary>
    /// overflow
    /// </summary>
    enum AttributeOverflow : uint8_t {
        Overflow_Auto = 0,      // & 1 == 0
        Overflow_Hidden,        // & 1 == 1
        Overflow_Scroll,        // & 1 == 0
        Overflow_Visible,       // & 1 == 1
    };
    /// <summary>
    /// border style 
    /// </summary>
    enum AttributeBStyle : uint8_t {
        Style_Node = 0,
        Style_Hidden,
        Style_Dotted,
        Style_Dashed,
        Style_Solid,
        Style_Double,
        Style_Groove,
        Style_Ridge,
        Style_Inset,
        Style_OutSet,
    };
    /// <summary>
    /// appearance attr
    /// </summary>
    enum AttributeAppearance : uint8_t {
        Appearance_NotSet = uint8_t(-1),    // init value
        Appearance_None = 0,                // [none]
        Appearance_Radio,                   // [radio]
        Appearance_Button,                  // [button]
        Appearance_Resizer,                 // [resizer]
        Appearance_CheckBox,                // [checkbox]
        Appearance_MenuRadio,               // [menuradio]
        Appearance_MenuCheckBox,            // [menucheckbox]
        Appearance_ToolBarButton,           // [toolbarbutton]
        Appearance_ScrollBarButtonUp,       // [scrollbarbutton-up]
        Appearance_ScrollBarButtonRight,    // [scrollbarbutton-right]
        Appearance_ScrollBarButtonDown,     // [scrollbarbutton-down]
        Appearance_ScrollBarButtonLeft,     // [scrollbarbutton-left]
        Appearance_ScrollbarThumbH,         // [scrollbarthumb-horizontal]
        Appearance_ScrollbarThumbV,         // [scrollbarthumb-vertical]
        Appearance_ScrollbarTrackH,         // [scrollbartrack-horizontal]
        Appearance_ScrollbarTrackV,         // [scrollbartrack-vertical]
        Appearance_MenuSeparator,           // [menuseparator]
        Appearance_MenuArrow,               // [menuarrow]
        Appearance_Separator,               // [separator]
        Appearance_ScaleH,                  // [scale-horizontal]
        Appearance_ScaleV,                  // [scale-vertical]
        Appearance_ScaleThumbH,             // [scalethumb-horizontal]
        Appearance_ScaleThumbV,             // [scalethumb-vertical]
        Appearance_ProgressBarH,            // [progressbar]
        Appearance_ProgressBarV,            // [progressbar-vertical]
        Appearance_ProgressChunkH,          // [progresschunk]
        Appearance_ProgressChunkV,          // [progresschunk-vertical]
        Appearance_ListBox,                 // [listbox]
        Appearance_StatusBar,               // [statusbar]
        Appearance_StatusBarPanel,          // [statusbarpanel]
        Appearance_GroupBox,                // [groupbox]
        Appearance_Caption,                 // undefined in xul
        Appearance_TreeHeaderCell,          // [treeheadercell]
        Appearance_TreeTwisty,              // [treetwisty]
        Appearance_DropDownMarker,          // used for menu list drop down marker
        Appearance_ListItem,                // [listitem]
        Appearance_MenuItem,                // [menuitem]
        Appearance_Tab,                     // [tab]
        Appearance_TabPanels,               // [tabpanels]
        Appearance_TextField,               // [textfield]
    }; 
    /// <summary>
    /// select type
    /// </summary>
    enum AttributeSeltype : uint8_t {
        Seltype_Single = 0,
        Seltype_Multiple,
        Seltype_Cell,
        Seltype_Text,
    };
    /// <summary>
    /// pseudo
    /// </summary>
    enum class PseudoType : uint8_t {
        Type_NoPseudo = 0,
        Type_Disabled,
        Type_Active,
        Type_Hover,
        Type_Focus,
        Type_Checked,
        Type_Indeterminate,
    };
    /// <summary>
    /// unit of value
    /// </summary>
    enum class ValueUnit : uint32_t {
        // unknown
        Unit_Unknown = 0,
        // auto
        Unit_Auto,
        // argb
        Unit_ARGB,
        // pixels
        Unit_Pixels,
        // percentage%
        Unit_Percentage,
    };
    // BKDR Hash Function
    auto BKDRHash(const char* a, const char* b) noexcept->uint32_t;
    // Hash find index
    auto Uint32FindIndex(const uint32_t[], uint32_t len, uint32_t code) noexcept->uint32_t;
    // parse the attribute
    struct AttrParser {
        // box
        static auto Box(U8View) noexcept->AttributeBox;
        // view to align
        static auto Align(U8View) noexcept->AttributeAlign;
        // view to pack
        static auto Pack(U8View) noexcept->AttributePack;
        // view to seltype
        static auto Seltype(U8View) noexcept->AttributeSeltype;
        // view to overflow
        static auto Overflow(U8View) noexcept->AttributeOverflow;
        // view to appearance
        static auto Appearance(U8View) noexcept->AttributeAppearance;
        // view to repeat
        static auto Repeat2(U8View, U8View) noexcept ->AttributeRepeat;
    };
}