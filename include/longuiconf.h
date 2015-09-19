#pragma once
// THIS FILE IS NONE-LICENSE

// malloc
#include <memory>
// dlmalloc
#define USE_DL_PREFIX
#include "../3rdparty/dlmalloc/dlmalloc.h"
// longui namespace
namespace LongUI {
    // alloc for normal space
    static auto NormalAlloc(size_t length) noexcept { return ::malloc(length); }
    // free for normal space
    static auto NormalFree(void* address) noexcept { return ::free(address); }
#ifndef _DEBUG
    // alloc for small space
    static auto SmallAlloc(size_t length) noexcept { return ::dlmalloc(length); }
    // free for small space
    static auto SmallFree(void* address) noexcept { return ::dlfree(address); }
#else
    // alloc for small space
    static auto SmallAlloc(size_t length) noexcept { return ::malloc(length); }
    // free for small space
    static auto SmallFree(void* address) noexcept { return ::free(address); }
#endif
    // template helper
    template<typename T>
    static auto NormalAllocT(T*, size_t length) noexcept { return reinterpret_cast<T*>(LongUI::NormalAlloc(length * sizeof(T))); }
    // template helper
    template<typename T>
    static auto SmallAllocT(T*, size_t length) noexcept { return reinterpret_cast<T*>(LongUI::SmallAlloc(length * sizeof(T))); }
    // get dpi for x
    static auto GetDpiX() noexcept { return 96.f; }
    // get dpi for y
    static auto GetDpiY() noexcept { return 96.f; }
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
#define LONGUI_VIDEO_IN_MF

// get length of array even class
#ifndef lengthof
#define lengthof(a) (sizeof(a)/sizeof(*a))
#endif

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
// Windows Header Files:
#include <windows.h>
//#include <commctrl.h>
#include <Shlobj.h>
//#include <Shlwapi.h>
#include <ShObjIdl.h>
// C++ 
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cwchar>
#include <vector>
#include <atomic>
#include <map>
#include <new>

// RichEdit for EditEx
#include <Richedit.h>
#include <Textserv.h>

// Basic Lib
#undef DrawText
#include <strsafe.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <D3D11SDKLayers.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
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
    static constexpr float          LongUIDefaultTextFontSize = 22.f;
    // LongUI 常量
    enum EnumUIConstant : uint32_t {
        // LongUI CUIString Fixed Buffer Length [fixed buffer length]
        LongUIStringFixedLength = 64,
        // LongUI Control Tree Max Depth [fixed buffer length]
        LongUITreeMaxDepth = 256,
        // LongUI String Buffer Length [fixed buffer length]
        LongUIStringBufferLength = 2048,
        // max count of control in window while in init [fixed buffer length]
        LongUIMaxControlInited = (1024 * 1 - 1),
        // default un-redo stack size [fixed buffer length]
        LongUIDefaultUnRedoCommandSize = 13,
        // max count of longui window in same time [fixed buffer length]
        LongUIMaxWindow = 32,
        // max count of contrl class [fixed buffer length]
        LongUIMaxControlClass = 64,
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
        // LongUI Default Mouse Hover Time
        LongUIDefaultHoverTime = 100,
        // minimal size in pixel for window by default
        LongUIWindowMinSize = 128,
        // minimal size for auto-size control
        LongUIAutoControlMinSize = 8,
        // target bitmap unit size, larger than this,
        // will call IDXGISwapChain::ResizeBuffers,
        // but to large will waste some memory
        LongUITargetBitmapUnitSize = 128,
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
    static inline auto MakeAsUnit(T vaule) noexcept ->T {
        return (((vaule)+(LongUITargetBitmapUnitSize - 1)) / LongUITargetBitmapUnitSize * LongUITargetBitmapUnitSize);
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
        auto Lock() noexcept { ::EnterCriticalSection(&m_cs); }
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

#ifdef LONGUI_VIDEO_IN_MF
#include <mfapi.h>
#include <Mfmediaengine.h>
#pragma comment(lib, "mfplat")
#endif


#ifndef DECLSPEC_NOVTABLE
#define DECLSPEC_NOVTABLE
#endif


#ifndef CATCH_HRESULT
#define CATCH_HRESULT(hr) catch (std::bad_alloc& ) { hr = E_OUTOFMEMORY; } catch (...)  { hr=E_FAIL; }
#endif

#define LongUINullXMLNode LongUI::null_xml_node


namespace LongUI {
    // pre-declare
    class UIControl;
    // pre-declare
    class UIContainer;
    // pre-declare
    class UIScrollBar;
    // pre-declare
    class UIWindow;
    // pre-declare, LongUI::CUIString is a string class that hold a small fixed buffer
    class CUIString;
    // pre-declare
    class IUIScript;
    // Vector, you can change to other like eastl stl lib, or you own
    template<typename T> using Vector = std::vector<T>;
    // String, you can change to other like eastl stl lib, or you own 
    //using DynamicString = std::wstring;
    // Basic Container
    using BasicContainer = LongUI::Vector<void*>;
    // map longui string to pointer, you can change to other like eastl stl lib, or you own 
    using StringMap = std::map<const CUIString, void*>;
    // null for xmlnode, pugixml hold a handle for pointer
    static const pugi::xml_node null_xml_node = pugi::xml_node(nullptr);
}

// XML Node Attribute/Value constexpr char* const Setting
namespace LongUI {
    // window class name: zhuangbilty
    static constexpr wchar_t* const WindowClassName = L"Windows.UI.LongUI.NormalWindow";
    // attribute namespace
    namespace XMLAttribute {
        // name of control
        static constexpr char* const ControlName            = "name";
        // script data
        static constexpr char* const Script                 = "script";
        // weight for layout
        static constexpr char* const LayoutWeight           = "weight";
        // visible
        static constexpr char* const Visible                = "visible";
        // user defined string
        static constexpr char* const UserDefinedString      = "userstring";
        // backgroud brush, 0 for null not default brush
        static constexpr char* const BackgroudBrush         = "bgbrush";
        // position of control left-top: float2
        static constexpr char* const LeftTopPosotion        = "pos";
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

        // template size
        static constexpr char* const TemplateSize           = "templatesize";
        // is always host children rendering?       [valid] for container
        static constexpr char* const IsHostChildrenAlways   = "hostchild";
        // is always host children rendering?       [valid] for container
        static constexpr char* const IsHostPosterityAlways  = "hostposterity";
        // left control class name and template id in "marginal control"
        static constexpr char* const LeftMarginalControl    = "leftcontrol";
        // top control class name and template id in "marginal control"
        static constexpr char* const TopMarginalControl     = "topcontrol";
        // right control class name and template id in "marginal control"
        static constexpr char* const RightMarginalControl   = "rightcontrol";
        // bottom control class name and template id in "marginal control"
        static constexpr char* const BottomMarginalControl  = "bottomcontrol";
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
