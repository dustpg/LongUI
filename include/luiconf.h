#pragma once
// 
// modify this file for your project
// **include this file in source-file at first**
// 

// windows 10 support only, make bin-file smaller, faster
//#define LUI_WIN10_ONLY

// give every control intptr_t c++: user_data/xml: data-user;
#define LUI_USER_INIPTR_DATA

// give every control CUIString c++: user_u16str/xml: data-u16;
// [[Not-Recommended]]
//#define LUI_USER_U16STR_DATA

// give every control CUIConstShortString c++: user_u8str/xml: data-u8;
//#define LUI_USER_U8STR_DATA

// no accessible? make bin-file smaller
//#define LUI_NO_ACCESSIBLE



// ----------------------------------------------------------------------------

// Accessible
#if !defined(NDEBUG) || !defined(LUI_NO_ACCESSIBLE)
#define LUI_ACCESSIBLE
#endif


#include <cstdlib>
#include <cstdint>

// Constant Setting
namespace LongUI {
    // int Constant
    enum EnumUIConstant : int32_t {
        // dirty rect count
        DIRTY_RECT_COUNT = 16,
        // Default Window Width 
        DEFAULT_WINDOW_WIDTH = 400,
        // Default Window Height
        DEFAULT_WINDOW_HEIGHT = 300,
        // Default Control Width 
        DEFAULT_CONTROL_WIDTH = 100,
        // Default Control Height
        DEFAULT_CONTROL_HEIGHT = 30,
        // min scrollbar display size
        MIN_SCROLLBAR_DISPLAY_SIZE = 64,
        // default control max width/height
        DEFAULT_CONTROL_MAX_SIZE = 100000,

        // invalid minsize
        INVALID_MINSIZE = -1,
        // short memory length
        SHORT_MEMORY_LENGTH = 127,
        // invalid control width/height
        INVALID_CONTROL_SIZE = -1,
        // max number of graphics adapters
        MAX_GRAPHICS_ADAPTERS = 32,
        // window class name buffer
        WNDCLASS_NAME_BUFFER_LENGTH = 48,
        // LongUI Control Tree Max Depth [fixed buffer length]
        /*TreeMaxDepth = 256,
        // LongUI String Buffer Length [fixed buffer length]
        StringBufferLength = 256,
        // max count of control in window while in init [fixed buffer length]
        MaxControlInited = (512 - 1),
        // default un-redo stack size [fixed buffer length]
        DefaultUnRedoCommandSize = 13,
        // max count of longui text renderer [fixed buffer length]
        TextRendererCountMax = 10,
        // max count of gradient stop [fixed buffer length]
        MaxGradientStop = 128,
        // dirty control size [fixed buffer length]
        // if dirty control number bigger than this in one frame,
        // will do the full-rendering, not dirty-rendering
        DirtyControlSize = 15,
        // PlanToRender total time in sec. [fixed buffer length]
        PlanRenderingTotalTime = 5,
        // minimal size in pixel for window by default
        WindowMinSize = 128,
        // minimal size for auto-size control
        AutoControlMinSize = 8,
        // target bitmap unit size, larger than this,
        // will call IDXGISwapChain::ResizeBuffers,
        // but to large will waste some memory
        TargetBitmapUnitSize = 128,*/
    };
    // attribute [pseudo] namespace
    struct Attribute {
        // default normal window class name zhuangbilty
        static constexpr wchar_t* const WindowClassNameN = L"Windows.UI.LongUI.DirectWindow";
        // default popup window class name zhuangbilty
        static constexpr wchar_t* const WindowClassNameP = L"Windows.UI.LongUI.PopupWindow";
        // tool window class name zhuangbilty
        static constexpr wchar_t* const WindowClassNameT = L"Windows.UI.LongUI.SystemInvoke";
    };
}
