#pragma once
// 
// modify this file for your project
// **include this file in source-file at first**
// 

// resize immediately on drag-sizing, but maybe like "Jelly" on win7
#define LUI_RESIZE_IMMEDIATELY

// style support, disable to make bin-file smaller, default look only
//#define LUI_DISABLE_STYLE_SUPPORT

// windows 10 support only, make bin-file smaller, faster
//#define LUI_WIN10_ONLY

// no accessible? make bin-file smaller
//#define LUI_NO_ACCESSIBLE

// no script support
//#define LUI_NO_SCRIPT

// longui will draw focus rect
#define LUI_DRAW_FOCUS_RECT

// give every control intptr_t c++: user_data/xml: data-user(int);
//#define LUI_USER_INIPTR_DATA

// give every control CUIString c++: user_u16str/xml: data-u16(str);
//#define LUI_USER_U16STR_DATA

// give every control CUIConstShortString c++: user_u8str/xml: data-u8(str);
//#define LUI_USER_U8STR_DATA

// [unused yet]LongUI::POD::Vector::size_type use uint32_t
//#define LUI_VECTOR_SIZE_USE_UINT32



// [BETA] NOT IMPL
//#define LUI_BETA_CTOR_LOCKER

// ----------------------------------------------------------------------------

// LongUI Need Win7-PlatformUpdate
#define _WIN7_PLATFORM_UPDATE



// MinGW unsupported yet?
// Accessible
#ifndef LUI_NO_ACCESSIBLE
#define LUI_ACCESSIBLE
#endif

// Constant Setting
namespace LongUI {
    // int Constant
    enum EnumUIConstant : long {
        // dirty rect count                 : for optimization
        DIRTY_RECT_COUNT = 16,
        // transparent window buffer unit   : for optimization
        TRANSPARENT_WIN_BUFFER_UNIT = 64,
        // default Window Width             : for init
        DEFAULT_WINDOW_WIDTH = 400,
        // default Window Height            : for init
        DEFAULT_WINDOW_HEIGHT = 300,
        // default Control Width            : for init
        DEFAULT_CONTROL_WIDTH = 100,
        // default Control Height           : for init
        DEFAULT_CONTROL_HEIGHT = 30,
        // empty height per row             : for minsize
        EMPTY_HEIGHT_PER_ROW = 24,
        // min scrollbar display size       : for scrollbar
        MIN_SCROLLBAR_DISPLAY_SIZE = 64,
        // default control max width/height :
        DEFAULT_CONTROL_MAX_SIZE = 100000,

#ifdef NDEBUG
        // window auto into sleep mode time after window hide (in ms)
        WINDOW_AUTOSLEEP_TIME = 1000 * 60 * 1,
        // global common-buf length in byte for ime-input optimization
        IME_COMMON_BUF_LENGTH = 64 * 2,
#else
        // window auto into sleep mode time after window hide (in ms)
        WINDOW_AUTOSLEEP_TIME = 1000 * 1,
        // global common-buf length in byte for ime-input optimization
        IME_COMMON_BUF_LENGTH = 4 * 2,
#endif
        // Bitmap Bank bitmap size
        BITBANK_BITMAP_SIZE = 1024,
        // Bitmap Bank rect max size
        //BITBANK_BITMAP_RMAX = 512,
        BITBANK_BITMAP_RMAX = 196,
        // Bitmap Bank bitmap move threshold
        BITBANK_BITMAP_MOVE_THRESHOLD = BITBANK_BITMAP_SIZE * BITBANK_BITMAP_SIZE / 2,


        // invalid minsize
        INVALID_MINSIZE = -1,
        // short memory length
        SHORT_MEMORY_LENGTH = 127,
        // invalid control width/height
        INVALID_CONTROL_SIZE = -1,
        // small buffer length
        SMALL_BUFFER_LENGTH = 32,
        // max number of graphics adapters
        MAX_GRAPHICS_ADAPTERS = 16,
        // window class name buffer
        WNDCLASS_NAME_BUFFER_LENGTH = 48,

        // LongUI Max Control Tree Depth(level in byte for 256)
        MAX_CONTROL_TREE_DEPTH = 256,
    };
    // attribute [pseudo] namespace
    struct Attribute {
        // tool window class name zhuangbilty
        static constexpr const wchar_t* WindowClassNameT = L"Windows.UI.LongUI.SystemInvoke";
        // default normal window class name zhuangbilty
        static constexpr const wchar_t* WindowClassNameN = L"Windows.UI.LongUI.DirectWindow";
        // default popup window class name zhuangbilty
        static constexpr const wchar_t* WindowClassNameP = L"Windows.UI.LongUI.PopupWindow";
        // default window icon name
        static constexpr const wchar_t* WindowIconName = nullptr; // (const wchar_t*)1
        // bug dump file path, set nullptr to disable dump-file
        static constexpr const char16_t*BugDumpFilePath = u"longui-v0.2.2.dmp";
    };
}
