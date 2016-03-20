#pragma once
// THIS FILE IS NONE-LICENSE
#ifndef LongUIAPI
#define LongUIAPI 
#endif

// malloc
#include <memory>
// dlmalloc
#define USE_DL_PREFIX
#include "../3rdparty/dlmalloc/dlmalloc.h"
// Windows Header Files:
#include <windows.h>

// longui namespace
namespace LongUI {
    // alloc for normal space
    static inline auto NormalAlloc(size_t length) noexcept { return std::malloc(length); }
    // free for normal space
    static inline auto NormalFree(void* address) noexcept { return std::free(address); }
#ifndef _DEBUG
    // alloc for small space
    static inline auto SmallAlloc(size_t length) noexcept { return ::dlmalloc(length); }
    // free for small space
    static inline auto SmallFree(void* address) noexcept { return ::dlfree(address); }
#else
    // alloc for small space
    static inline auto SmallAlloc(size_t length) noexcept { return std::malloc(length); }
    // free for small space
    static inline auto SmallFree(void* address) noexcept { return std::free(address); }
#endif
    // template helper
    template<typename T> static inline auto NormalAllocT(size_t length) noexcept {
        return reinterpret_cast<T*>(LongUI::NormalAlloc(length * sizeof(T))); 
    }
    // template helper
    template<typename T> static inline auto SmallAllocT(size_t length) noexcept { 
        return reinterpret_cast<T*>(LongUI::SmallAlloc(length * sizeof(T))); 
    }
    // error beep
    inline void BeepError() noexcept { ::MessageBeep(MB_ICONERROR); }
}


#ifdef _MSC_VER
// 微软蛋疼
#pragma warning(disable: 4290)
#pragma warning(disable: 4200)
// 无视部分警告等级4
#pragma warning(disable: 4505) // unused function
#pragma warning(disable: 4201) // nameless struct/union
#pragma warning(disable: 4706) // assignment within conditional expression
#pragma warning(disable: 4127) // assignment within constant expression
#endif

// singleton for ui manager
#define UIManager (LongUI::CUIManager::s_instance)

// retain debug infomation within UIControl::debug_do_event
#ifdef _DEBUG
#   define LongUIDebugEvent
#endif

// main property only?
#define LONGUI_EDITCORE_COPYMAINPROPERTYONLY

// using Media Foundation to play video file?
#define LONGUI_WITH_MMFVIDEO


#ifndef LongUIInline
#define LongUIInline __forceinline
//#define LongUIInline inline __attribute__((__always_inline__))
//#define LongUIInline __inline
#endif

#ifndef LongUINoinline
// MSC
#define LongUINoinline __declspec(noinline)
// GCC
//#define LongUINoinline __attribute__((?????))
#endif


#ifndef __fallthrough
#define __fallthrough (void)(0)
#endif

// if you implement a standalone IUIConfigure by yourself, undef it
#define LONGUI_WITH_DEFAULT_CONFIG

// nuclear card first
//#define LONGUI_NUCLEAR_FIRST

#ifdef LONGUI_WITH_DEFAULT_HEADER
#ifndef _DEBUG
#define NDEBUG
#endif
//#include <commctrl.h>
#include <Shlobj.h>
//#include <Shlwapi.h>
#include <ShObjIdl.h>
// C++ 
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cwchar>
#include <atomic>
#include <new>

// TSF
#include <Msctf.h>

// RichEdit for EditEx
#include <Richedit.h>
#include <Textserv.h>

// Basic Lib
#undef DrawText
#include <strsafe.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <D3D11SDKLayers.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <d2d1effects.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#include <dwrite_1.h>
// DirectComposition , less header files
interface IDCompositionDevice;
interface IDCompositionTarget;
interface IDCompositionVisual;
// pugixml
#include "../3rdparty/pugixml/pugixml.hpp"
#endif

// longui 
namespace LongUI {
    // LongUI Default Text Font Family Name
    static constexpr wchar_t* const LongUIDefaultTextFontName = L"Arial";
    // LongUI Default Text Font Size
    static constexpr float          LongUIDefaultTextFontSize = 16.f;
    // LongUI Default Text V-Align
    static constexpr uint32_t       LongUIDefaultTextVAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
    // LongUI Default Text H-Align
    static constexpr uint32_t       LongUIDefaultTextHAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
    // LongUI 常量
    enum EnumUIConstant : uint32_t {
        // LongUI CUIString Fixed Buffer Length [fixed buffer length]
        LongUIStringFixedLength = 64,
        // LongUI Control Tree Max Depth [fixed buffer length]
        LongUITreeMaxDepth = 256,
        // LongUI String Buffer Length [fixed buffer length]
        LongUIStringBufferLength = 2048,
        // max count of control in window while in init [fixed buffer length]
        LongUIMaxControlInited = (512 - 1),
        // default un-redo stack size [fixed buffer length]
        LongUIDefaultUnRedoCommandSize = 13,
        // max count of longui window in same time [fixed buffer length]
        //LongUIMaxWindow = 32,
        // max count of longui text renderer [fixed buffer length]
        LongUITextRendererCountMax = 10,
        // max length of longui text renderer length [fixed buffer length]
        LongUITextRendererNameMaxLength = 32,
        // max count of gradient stop [fixed buffer length]
        LongUIMaxGradientStop = 128,
        // dirty control size [fixed buffer length]
        // if dirty control number bigger than this in one frame,
        // will do the full-rendering, not dirty-rendering
        LongUIDirtyControlSize = 15,
        // PlanToRender total time in sec. [fixed buffer length]
        LongUIPlanRenderingTotalTime = 5,
        // LongUI Default Window Width 
        LongUIDefaultWindowWidth = 800,
        // LongUI Default Window Height
        LongUIDefaultWindowHeight = 600,
        // minimal size in pixel for window by default
        LongUIWindowMinSize = 128,
        // minimal size for auto-size control
        LongUIAutoControlMinSize = 8,
        // target bitmap unit size, larger than this,
        // will call IDXGISwapChain::ResizeBuffers,
        // but to large will waste some memory
        LongUITargetBitmapUnitSize = 128,
        // max number of video adapters
        LongUIMaxAdaptersSize = 32,
        // ------- Be Careful When Modify ---------
        // LongUI Common Solid Color Brush Index
        LongUICommonSolidColorBrushIndex = 0,
        // LongUI Default Text Format Index(Arial@22px)
        LongUIDefaultTextFormatIndex = 0,
        // LongUI Default Bitmap Index
        LongUIDefaultBitmapIndex = 0,
        // LongUI Default Bitmap Size(256x256)
        LongUIDefaultBitmapSize = 256,
        // LongUI Default Bitmap Options(CANNOT draw, Orz.....)
        LongUIDefaultBitmapOptions = (uint32_t)D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
    };
    // make as unit
    template<typename T>
    static inline auto MakeAsUnit(T value) noexcept ->T {
        return (((value)+(LongUITargetBitmapUnitSize - 1)) / LongUITargetBitmapUnitSize * LongUITargetBitmapUnitSize);
    }
    // text renderer name
    struct NameTR { char name[LongUITextRendererNameMaxLength]; };
    // UI Locker
    class CUILocker {
    public:
#if 1
        // ctor
        CUILocker() noexcept { ::InitializeCriticalSection(&m_cs); }
        // dtor
        ~CUILocker() noexcept { ::DeleteCriticalSection(&m_cs); }
        // lock
        auto Lock() noexcept { 
#ifdef _DEBUG
            if (!::TryEnterCriticalSection(&m_cs)) {
                void longui_dbg_locked(const LongUI::CUILocker&);
                longui_dbg_locked(*this);
                ::EnterCriticalSection(&m_cs);
            }
#else
            ::EnterCriticalSection(&m_cs);
#endif
        }
        // unlock
        auto Unlock() noexcept { ::LeaveCriticalSection(&m_cs); }
    private:
        // cs
        CRITICAL_SECTION        m_cs;
#else
        // lock
        auto Lock() noexcept { m_mux.lock(); }
        // unlock
        auto Unlock() noexcept { m_mux.unlock(); }
    private:
        // mutex
        std::recursive_mutex    m_mux;
#endif
    };
}


#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef LONGUI_WITH_MMFVIDEO
#include <mfapi.h>
#include <Mfmediaengine.h>
#pragma comment(lib, "mfplat")
#endif


#ifdef _MSC_VER
#define LONGUI_NOVTABLE __declspec(novtable)
#else
#define LONGUI_NOVTABLE
#endif

namespace LongUI {
    // pre-declare
    class UIControl;
    // pre-declare
    class UIContainer;
    // pre-declare
    class UIScrollBar;
    // pre-declare
    class CUIManager;
    // pre-declare
    class UIViewport;
    // pre-declare, LongUI::CUIString is a string class that hold a small fixed buffer
    class CUIString;
    // pre-declare
    class IUIScript;
    // base window
    class XUIBaseWindow;
}

// XML Node Attribute/Value constexpr char* const Setting
namespace LongUI {
    // default window class name zhuangbilty
    static constexpr wchar_t* const WindowClassName = L"Windows.UI.LongUI.DirectWindow";
    // Invisible window class name zhuangbilty
    static constexpr wchar_t* const InvisibleName = L"Windows.UI.LongUI.SystemInvoke";
    // attribute namespace
    namespace XmlAttribute {
        // name of control
        static constexpr char* const ControlName            = "name";
        // script data
        static constexpr char* const Script                 = "script";
        // weight for layout
        static constexpr char* const LayoutWeight           = "weight";
        // context for layout
        static constexpr char* const LayoutContext          = "context";
        // visible
        static constexpr char* const Visible                = "visible";
        // user defined string                      [invalid yet]
        static constexpr char* const UserDefinedString      = "userstring";
        // backgroud brush, 0 for null not default brush
        static constexpr char* const BackgroudBrush         = "bgbrush";
        // size of control: float2
        static constexpr char* const AllSize                = "size";
        // margin: float4
        static constexpr char* const Margin                 = "margin";
        // width of border: float1
        static constexpr char* const BorderWidth            = "borderwidth";
        // round of border: float2
        static constexpr char* const BorderRound            = "borderround";
        // template id for easy building: int
        static constexpr char* const TemplateID             = "templateid";
        // the priority for (dirty) rendering
        static constexpr char* const RenderingPriority      = "priority";
        // is render parent container?              [valid] for normal control
        static constexpr char* const IsRenderParent         = "renderparent";
        // is clip strictly                         [invalid yet]
        static constexpr char* const IsClipStrictly         = "strictclip";
        // enabled
        static constexpr char* const Enabled                = "enabled";

        // marginal control direction
        static constexpr char* const MarginalDirection      = "marginal";

        // template size
        static constexpr char* const TemplateSize           = "templatesize";
        // is always host children rendering?       [valid] for container
        static constexpr char* const IsHostPosterityAlways  = "hostposterity";
        // marginal control will be zoomed?
        static constexpr char* const IsZoomMarginalControl  = "zoommarginal";

        // window clear color
        static constexpr char* const WindowClearColor       = "clearcolor";
        // window titlebar name
        static constexpr char* const WindowTitleName        = "titlename";
        // antimode for text
        static constexpr char* const WindowTextAntiMode     = "textantimode";

    }
}
