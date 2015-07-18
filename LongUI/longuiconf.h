#pragma once
// THIS FILE IS NONE-LICENSE
// malloc
#include <memory>
// dlmalloc
#define USE_DL_PREFIX
#include "../3rdparty/dlmalloc/dlmalloc.h"
// longui namespace
namespace LongUI {
    // alloc for longui control
    static auto CtrlAlloc(size_t length) noexcept { return ::malloc(length); }
    // free for longui control
    static auto CtrlFree(void* address) noexcept { return ::free(address); }
    // alloc for small space
    static auto SmallAlloc(size_t length) noexcept { return ::dlmalloc(length); }
    // free for small space
    static auto SmallFree(void* address) noexcept { return ::dlfree(address); }
    // template helper
    template<typename T>
    static auto CtrlAllocT(T* pointer, size_t length) noexcept { return reinterpret_cast<T*>(LongUI::CtrlAlloc(length * sizeof(T))); }
    // template helper
    template<typename T>
    static auto SmallAllocT(T* pointer, size_t length) noexcept { return reinterpret_cast<T*>(LongUI::SmallAlloc(length * sizeof(T))); }
}


#ifdef _MSC_VER
// 微软蛋疼
#pragma warning(disable: 4290)
#pragma warning(disable: 4200)
#endif


// 钳住
#define LongUIClamp(x, i, a) { if(x < i) x = i; else if(x > a) x = a;}


#define UIManager (LongUI::CUIManager::s_instance)


// all longui control class' method(not inline), and callback call type
#define __thiscall
//#define __fastcall 

// mainly prop only?
#define LONGUI_EDITCORE_COPYMAINPROP

// using Media Foundation to play video file?
#define LONGUI_VIDEO_IN_MF


#ifndef lengthof
#define lengthof(a) (sizeof(a)/sizeof(*a))
#endif

#ifndef LongUIInline
#define LongUIInline __forceinline
//#define LongUIInline __attribute__()
//#define LongUIInline __inline
#endif

#ifndef LongUINoinline
// MSC
#define LongUINoinline __declspec(noinline)
// GCC
//#define LongUINoinline __attribute__((noinline))
#endif

#ifndef LongUIAlignas
#define LongUIAlignas alignas(sizeof(void*)) 
#endif

#ifndef __fallthrough
#define __fallthrough (void)(0)
#endif

// std::floorf // 负数也看不到
//#define LongUIFloorF(v) static_cast<float>(static_cast<int>(v))

// if you impl a IUIConfigure by yourself, undef it
#define LONGUI_WITH_DEFAULT_CONFIG


// nuclear card first
//#define LONGUI_NUCLEAR_FIRST



#ifdef LONGUI_WITH_DEFAULT_HEADER
#ifndef _DEBUG
#define NDEBUG
#endif
// Windows Header Files:
#include <windows.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <ShObjIdl.h>
// C++ 
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <clocale>
#include <string>
#include <cwchar>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <map>
#include <new>

// RichEdit
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
// DirectComposition 
#include <dcomp.h>

// pugixml
#include "../3rdparty/pugixml/pugixml.hpp"
#endif

// longui 
namespace LongUI {
    // LongUI Default Text Font Family Name
    constexpr static wchar_t* LongUIDefaultTextFontName = L"Arial";
    // LongUI Default Text Font Size
    constexpr static float LongUIDefaultTextFontSize = 22.f;
    // LongUI 常量
    enum EnumUIConstant : uint32_t {
        // LongUI String Static Buffer Length(object)
        LongUIStringLength = 64,
        // LongUI Control Tree Max Depth
        LongUITreeMaxDepth = 256,
        // LongUI String Buffer Length(temp)
        LongUIStringBufferLength = 2048,
        // LongUI Default Window Width 
        LongUIDefaultWindowWidth = 800,
        // LongUI Default Window Height
        LongUIDefaultWindowHeight = 600,
        // LongUI Default Mouse Hover Time
        LongUIDefaultHoverTime = 100,
        // LongUI Default Slider Half Width
        LongUIDefaultSliderHalfWidth = 10,
        // LongUI Default Slider Half Width
        LongUIDefaultCheckBoxWidth = 16,
        // max count of longui window in same time
        LongUIMaxWindow = 16,
        // minimal size in pixel for window by default
        LongUIWindowMiniSize = 128,
        // max count of contrl class
        LongUIMaxControlClass = 64,
        // update message
        LongUIUpdateMessage = WM_USER + 64,
        // max count of longui text renderer
        LongUIMaxTextRenderer = 10,
        // max count of gradient stop
        LongUIMaxGradientStop = 128,
        // PlanToRender total time in sec.
        LongUIPlanRenderingTotalTime = 5,
        // target bitmap unit size, larger than this,
        // will call IDXGISwapChain::ResizeBuffers,
        // but to large will waste some memory
        LongUITargetBitmapUnitSize = 128,
        // dirty control size
        // if dirty control number bigger than this in one frame,
        // will do the full-rendering, not dirty-rendering
        LongUIDirtyControlSize = 15,
        // max count of control in window while creating
        LongUIMaxControlInited = (1024 * 8 - 1),
        // default un-redo stack size
        LongUIDefaultUnRedoCommandSize = 13,
        // LongUI Default Normal Color, without alpha
        LongUIDefaultNormalColor = 0xFFFFFFFF,
        // LongUI Default Hover Color, without alpha
        LongUIDefaultHoverColor = 0xEEEEEEEE,
        // LongUI Default Click Color, without alpha
        LongUIDefaultClickedColor = 0xCCCCCCCC,
        // LongUI Default Disabled Color, without alpha
        LongUIDefaultDisabledColor = 0xDDDDDDDD,
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
    // pre-declare, CUIString is a string class that hold a fixed buffer
    class CUIString;
    // Vector, you can change to other like eastl stl lib, or you own 
    template<typename T> using Vector = std::vector < T >;
    // String, you can change to other like eastl stl lib, or you own 
    using DynamicString = std::wstring;
    // Basic Container 基本容器
    using BasicContainer = LongUI::Vector<void*>;
    // map string to funtion, you can change to other like eastl stl lib, or you own 
    using StringMap = std::map<const CUIString, void*>;
    // null for xmlnode, pugixml hold a handle for pointer
    static const pugi::xml_node null_xml_node = pugi::xml_node(nullptr);
}