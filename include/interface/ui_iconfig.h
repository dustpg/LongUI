#pragma once

#include "ui_interface.h"
#include "../core/ui_core_type.h"

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
    // UI Configure Interface
    struct PCN_NOVTABLE IUIConfigure /*: IUIRefCount*/ {
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
            // only one system window, like game(all child window will be logic window)
            Flag_OnlyOneSystemWindow = 1 << 3,
            // no hi-dpi auto scale
            Flag_NoAutoScaleOnHighDpi = 1 << 4,
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
        };
    public:
#ifndef NDEBUG
        /// <summary>
        /// Gets the name of the simple log file.
        /// </summary>
        /// <returns></returns>
        virtual auto GetSimpleLogFileName() noexcept ->CUIString = 0;
#endif

        /// <summary>
        /// Get flags for configure
        /// </summary>
        /// <returns>flags for configure</returns>
        virtual auto GetConfigureFlag() noexcept->ConfigureFlag = 0;

        /// <summary>
        /// Gets the locale name
        /// </summary>
        /// <param name="name">The locale name buffer</param>
        /// <remarks>L"" for local locale name</remarks>
        virtual void GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept = 0;

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
        virtual auto ChooseAdapter(const GraphicsAdapterDesc adapters[/*length*/], const size_t length /*<=64*/) noexcept->size_t = 0;

        /// <summary>
        /// Shows the error.
        /// </summary>
        /// <param name="str_a">String A</param>
        /// <param name="str_b">String B</param>
        /// <returns>return false if user ignore this error</returns>
        virtual bool ShowError(const wchar_t* str_a, const wchar_t* str_b) noexcept = 0;
    public:
        /*// run a section script for event
        virtual auto Evaluation(ScriptUI, UIControl&) noexcept ->bool = 0;
        // alloc the script memory and copy into(may be compiled into byte code)
        virtual auto AllocScript(U8View) noexcept-> ScriptUI = 0;
        // free the script memory
        virtual void FreeScript(ScriptUI) noexcept = 0;*/
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
        /// Mains the loop.
        /// </summary>
        /// <returns></returns>
        virtual void MainLoop() noexcept = 0;
        /// <summary>
        /// exit the loop.
        /// </summary>
        /// <returns></returns>
        virtual void Exit() noexcept = 0;
    };
    // operator |
    inline constexpr IUIConfigure::ConfigureFlag operator|(
        IUIConfigure::ConfigureFlag a,
        IUIConfigure::ConfigureFlag b) noexcept {
        return static_cast<IUIConfigure::ConfigureFlag>(uint32_t(a) | b);
    }
    // operator &
    inline constexpr IUIConfigure::ConfigureFlag operator&(
        IUIConfigure::ConfigureFlag a,
        IUIConfigure::ConfigureFlag b) noexcept {
        return static_cast<IUIConfigure::ConfigureFlag>(uint32_t(a) & b);
    }
}