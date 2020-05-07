#pragma once

//#include "ui_interface.h"
#include "../util/ui_unimacro.h"
#include "../core/ui_basic_type.h"
#include "../core/ui_core_type.h"
#include <cstdint>

// ui namespace
namespace LongUI {
    // script
    struct ScriptUI;
    // argument
    struct EventArgument;
    // control info list
    struct ControlInfoList;
    // Font arg
    struct FontArg;
    // Handle code : OOM
    enum CodeOOM : uint32_t {
        // ignore
        OOM_Ignore = 0,
        // retry
        OOM_Retry,
        // noreturn
        OOM_NoReturn,
    };
    // ConfigureFlag decl
    enum ConfigureFlag : uint32_t;
    // operator |
    constexpr ConfigureFlag operator|(ConfigureFlag a, ConfigureFlag b) noexcept {
        return static_cast<ConfigureFlag>(uint32_t(a) | uint32_t(b));
    }
    // operator &
    constexpr ConfigureFlag operator&(ConfigureFlag a, ConfigureFlag b) noexcept {
        return static_cast<ConfigureFlag>(uint32_t(a) & uint32_t(b));
    }
    // flag
    enum ConfigureFlag : uint32_t {
        // no flag
        Flag_None = 0,
        // all flags
        Flag_All = uint32_t(-1),
        // flag for CPU rendering, if not, will call IUIInterface::ChooseAdapter
        Flag_RenderByCPU = 1 << 0,
        // output debug string?
        Flag_OutputDebugString = 1 << 1,
        // quit on last window closed
        Flag_QuitOnLastWindowClosed = 1 << 2,
        // only one system window, like game(all child window will be inline window)
        Flag_OnlyOneSystemWindow = 1 << 3,
        // no hi-dpi auto scale
        Flag_NoAutoScaleOnHighDpi = 1 << 4,
        // subpixel text rendering as default value for window/viewport
        Flag_SubpixelTextRenderingAsDefault = 1 << 5,
        // -------------------------------------------------------------
        // [debug flag under DEBUG MODE] output font family infomation
        Flag_DbgOutputFontFamily = 1 << 10,
        // [debug flag under DEBUG MODE] output time-took to NONE
        Flag_DbgOutputTimeTook = 1 << 11,
        // [debug flag under DEBUG MODE] draw colored dirty rect
        Flag_DbgDrawDirtyRect = 1 << 12,
        // [debug flag under DEBUG MODE] draw textbox cell rect
        Flag_DbgDrawTextCell = 1 << 13,
        // [debug flag under DEBUG MODE] display debug window
        Flag_DbgDebugWindow = 1 << 14,
        // [debug flag under DEBUG MODE] Dnt link style sheet
        Flag_DbgNoLinkStyle = 1 << 15,
        // -------------------------------------------------------------
        // defualt flag
        Flag_Default = Flag_None
#ifndef NDEBUG
        | Flag_RenderByCPU
#endif
        | Flag_OutputDebugString
        | Flag_QuitOnLastWindowClosed
        //| Flag_NoAutoScaleOnHighDpi
        //| Flag_DbgOutputTimeTook 
        //| Flag_DbgDrawDirtyRect
        | Flag_DbgDrawTextCell
        | Flag_DbgDebugWindow
    };
    // error occasion
    enum ErrorOccasion : uint32_t {
        // unknown
        Occasion_Unknown = 0,
        // when recreate
        Occasion_Recreate,
        // when resize window
        Occasion_Resizing,
        // when frame rendered
        Occasion_Frame,
    };
    // UI Configure Interface
    struct PCN_NOVTABLE IUIConfigure /*: IUIRefCount*/ {
#ifndef NDEBUG
        /// <summary>
        /// Gets the name of the simple log file.
        /// </summary>
        /// <returns></returns>
        virtual auto GetSimpleLogFileName() noexcept ->CUIString = 0;
#endif

        /// <summary>
        /// Loads the data from URL.
        /// </summary>
        /// <param name="url_in_utf8">The URL in UTF8.</param>
        /// <param name="url_in_utf16">The URL in UTF16.</param>
        /// <param name="buffer">The output buffer.</param>
        /// <returns></returns>
        virtual void LoadDataFromUrl(U8View url_in_utf8, const CUIString& url_in_utf16, POD::Vector<uint8_t>& buffer) noexcept = 0;

        /// <summary>
        /// Gets the locale name
        /// </summary>
        /// <param name="name">The locale name buffer</param>
        /// <remarks>empty for local locale name</remarks>
        virtual void GetLocaleName(char16_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept = 0;

        /// <summary>
        /// Defaults the font arg
        /// </summary>
        /// <param name="arg">The argument.</param>
        /// <param name="family">The font family name.</param>
        virtual void DefaultFontArg(FontArg& arg) noexcept = 0;

        /// <summary>
        /// Adds the control class info.
        /// </summary>
        /// <remarks>call CUIManager::RegisterControl to add control class</remarks>
        virtual void RegisterControl(ControlInfoList& list) noexcept = 0;

        /// <summary>
        /// Chooses the video adapter.
        /// </summary>
        /// <param name="adapters">The adapter array</param>
        /// <param name="length">The length of adapters</param>
        /// <remarks>
        /// if set "Flag_RenderByCPU", you should choose a video card,return the index.
        /// if return code out of range, will set by default(null pointer adapter)
        /// btw, in the adapter list, also include the WARP-adapter
        /// </remarks>
        /// <returns>index of adapters</returns>
        virtual auto ChooseAdapter(const GraphicsAdapterDesc adapters[/*length*/], const uint32_t length /*<=64*/) noexcept->uint32_t = 0;

        /// <summary>
        /// Called when [error information lost].
        /// </summary>
        /// <param name="">The .</param>
        /// <param name="occ">The occ.</param>
        /// <returns></returns>
        virtual void OnErrorInfoLost(Result, ErrorOccasion occ) noexcept = 0;
        /// <summary>
        /// Called when out of memory, won't be called on ctor
        /// </summary>
        /// <param name="retry_count">The retry count.</param>
        /// <param name="try_alloc">The try alloc.</param>
        /// <returns></returns>
        virtual auto HandleOOM(size_t retry_count, size_t try_alloc) noexcept->CodeOOM = 0;
    public:
        // run a section script for event
        virtual bool Evaluation(ScriptUI, UIControl&) noexcept = 0;
        // alloc the script memory and copy into(may be compiled into byte code)
        virtual auto AllocScript(U8View) noexcept-> ScriptUI = 0;
        // free the script memory
        virtual void FreeScript(ScriptUI) noexcept = 0;
        // eval script for window init, may be called over once for single window
        virtual void Evaluation(U8View, CUIWindow&) noexcept =0;
        // finalize window script if has script
        virtual void FinalizeScript(CUIWindow&) noexcept =0;
    public:
        // alloc for normal space
        virtual void*NormalAlloc(size_t length) noexcept = 0;
        // free for normal space
        virtual void NormalFree(void* address) noexcept = 0;
        // realloc for normal space
        virtual void*NormalRealloc(void* address, size_t length) noexcept = 0;
        // alloc for small space
        virtual void*SmallAlloc(size_t length) noexcept = 0;
        // free for small space
        virtual void SmallFree(void* address) noexcept = 0;
    public:
        /// <summary>
        /// Begins the render thread.
        /// </summary>
        /// <returns>return true if ok</returns>
        virtual auto BeginRenderThread() noexcept ->Result = 0;
        /// <summary>
        /// Ends the render thread.
        /// </summary>
        virtual void EndRenderThread() noexcept = 0;
        /// <summary>
        /// Recursions the MSG loop.
        /// </summary>
        virtual auto RecursionMsgLoop() noexcept ->uintptr_t = 0;
        /// <summary>
        /// break current msg-loop
        /// </summary>
        virtual void BreakMsgLoop(uintptr_t code) noexcept = 0;
    };
}