#pragma once

// ui
#include "ui_object.h"
#include "ui_core_type.h"
#include "ui_basic_type.h"
#include "ui_window_event.h"
#include "../util/ui_ostype.h"
#include "../accessible/ui_accessible.h"
// c++
#include <cstddef>

namespace LongUI {
    // style sheet
    class CUIStyleSheet;
    // platform
    class CUIPlatform;
    // Cursor
    class CUICursor;
    // window manager
    class CUIWndMgr;
    // color
    struct ColorF;
    // WindowsAlList
    struct AllWindows;
    // popup type
    enum class PopupType : uint16_t;
    // detail namespace
    namespace impl {
        // private data for manager
        template<size_t> struct platform;
        // 32bit
        template<> struct platform<4> { enum { size = 964, align = 4 }; };
        // 64bit
        template<> struct platform<8> { enum { size = 1224, align = 8 }; };
    }
    /// <summary>
    /// window base class
    /// </summary>
    class CUIWindow final : public CUIWindowEvent, public CUINoMo {
        // itr
        using Iterator = Node<CUIWindow>::Iterator;
        // private impl
        class Private;
        // friend class
        friend class UIViewport;
        // friend class
        friend class CUIWndMgr;
        // friend class
        friend class CUIPlatform;
#ifdef LUI_ACCESSIBLE
        // friend class
        friend class CUIAccessibleWnd;
#endif
        // show type
        enum TypeShow : int32_t {
            Show_Hide   = 0,
            Show_Show   = 1,
            Show_Min    = 2,
            Show_Max    = 3,
            Show_NA     = 8,    // not activated
            Show_Restore= 9,
        };
        // show window
        void show_window(TypeShow) noexcept;
        // close window
        void close_window() noexcept;
    public:
        // window config
        enum WindowConfig : uint16_t {
            // press esc to close window
            Config_EscToCloseWindow = 1 << 0,
            // press alt+f4 to close window
            Config_AltF4ToCloseWindow = 1 << 1,
            // quit on close
            Config_QuitOnClose = 1 << 2,
            // delete on close
            Config_DeleteOnClose = 1 << 3,

            // popup window
            Config_Popup = 1 << 8,
            // frameless window
            Config_Frameless = 1 << 9,
            // tool window, no included in "quit on close"
            Config_ToolWindow = 1 << 10,
            // fixed size, cannot drag to resize but invoke Resize()
            Config_FixedSize = 1 << 11,
            // layered window[support for Win8.1 and higher]
            Config_LayeredWindow = 1 << 12,

            // default config
            Config_Default = Config_AltF4ToCloseWindow,
        };
    public:
        // hide the window
        void HideWindow() noexcept { this->show_window(Show_Hide); }
        // show the window
        void ShowWindow() noexcept { this->show_window(Show_Show); }
        // min the window
        void MinWindow() noexcept { this->show_window(Show_Min); }
        // max the window
        void MaxWindow() noexcept { this->show_window(Show_Max); }
        // show the window - not activated.
        void ShowNoActivate() noexcept { this->show_window(Show_NA); }
        // restore the window
        void RestoreWindow() noexcept { this->show_window(Show_Restore); }
        // close window
        void CloseWindow() noexcept;
        // enable window
        void EnableWindow(bool) noexcept;
        // set as active
        void ActiveWindow() noexcept;
        // is visible
        bool IsVisible() const noexcept;
        // is in dtor
        bool IsInDtor() const noexcept { return m_inDtor; }
        // mark full rendering
        void MarkFullRendering() noexcept;
#if 0
        // is auto sleep?
        bool IsAutoSleep() const noexcept { return !!(config & Config_Popup); }
        // is in sleep mode?
        //auto IsInSleepMode() const noexcept { return !m_hwnd; }
#endif
        // into sleep mode immediately
        void SleepImmediately() noexcept;
        // try sleep
        void TrySleep() noexcept;
        // wake up
        void WakeUp() noexcept;
        // set result to exit Exec()
        void SetResult(uintptr_t) noexcept;
        // execute, as modal window if parent window exist
        auto Exec() noexcept -> uintptr_t;
        // is in exec
        bool IsInExec() const noexcept { return m_bInExec; }
#ifndef LUI_DISABLE_STYLE_SUPPORT
    public:
        // load css file
        void LoadCssFile(U8View file) noexcept;
        // load css string
        void LoadCssString(U8View string) noexcept;
        // get style sheet
        auto GetStyleSheet() const noexcept { return m_pStyleSheet; }
#endif
    public:
        // map to screen
        void MapToScreen(RectF& rect) const noexcept;
        // map to screen
        void MapToScreen(RectL& rect) const noexcept;
        // map to screen
        auto MapToScreenEx(Point2F pos) const noexcept ->Point2F;
        // map to screen
        //auto MapToScreenEx(Point2L pos) const noexcept->Point2L;
        // map from screen
        //void MapFromScreen(RectF& rect) const noexcept;
        // map from screen
        void MapFromScreen(Point2F& pos) const noexcept;
    public:
        // show popup window
        void PopupWindow(CUIWindow& wnd, Point2L pos, PopupType type) noexcept;
        // set tooltip text, return tooltip viewport pointer
        auto TooltipText(CUIString&&) noexcept ->UIViewport*;
        // close all popupwindow
        void ClosePopup() noexcept;
        // get now popupwindow
        auto GetNowPopup() const noexcept->CUIWindow*;
        // get now popupwindow with type
        auto GetNowPopup(PopupType type) const noexcept->CUIWindow*;
        // close tooltip
        void CloseTooltip() noexcept;
        // set title name
        void SetTitleName(U16View) noexcept;
        // set title name
        void SetTitleName(CUIString&&) noexcept;
        // get title name
        auto GetTitleName() const noexcept->U16View;
        // set pos of window
        void SetPos(Point2L pos) noexcept;
        // get pos of window
        auto GetPos() const noexcept->Point2L;
        // get absolute size of window
        auto GetAbsoluteSize() const noexcept->Size2L;
        // set absolute rect(= set pos + resize)
        void SetAbsoluteRect(const RectL& rect) noexcept;
        // resize window  : absolute
        void ResizeAbsolute(Size2L size) noexcept;
        // resize window : relative
        void ResizeRelative(Size2F size) noexcept;
        // set color color
        void SetClearColor(const ColorF&) noexcept;
        // set now cursor
        void SetNowCursor(const CUICursor&) noexcept;
        // set now cursor to default
        void SetNowCursor(std::nullptr_t) noexcept;
        // work area that this window worked
        auto GetWorkArea() const noexcept->RectL;
        // get raw handle
        auto GetRawHandle() const noexcept->uintptr_t;
    public:
        // show caret
        void ShowCaret(UIControl&ctrl, const RectF& rect) noexcept;
        // set caret color
        void SetCaretColor(const ColorF&) noexcept;
        // hide caret
        void HideCaret() noexcept;
        // find control, return null if notfound
        auto FindControl(const char* id) noexcept->UIControl*;
        // find control, return null if notfound
        auto FindControl(U8View view) noexcept ->UIControl*;
        // find control with UID-String, return null if notfound
        auto FindControl(ULID) noexcept->UIControl*;
        // control attached                 [null this ptr acceptable]
        void ControlAttached(UIControl& ctrl) noexcept;
        // control disattached              [null this ptr acceptable]
        void ControlDisattached(UIControl& ctrl) noexcept;
        // add named control                [null this ptr acceptable]
        void AddNamedControl(UIControl& ctrl) noexcept;
        // close popupwindow until not popup[null this ptr acceptable]
        void ClosePopupHighLevel() noexcept;
        // set captured control
        void SetCapture(UIControl& ctrl) noexcept;
        // release captured control, return true if released
        bool ReleaseCapture(UIControl& ctrl) noexcept;
        // release captured control - force
        void ForceReleaseCapture() noexcept;
        // set focus of control,
        bool SetFocus(UIControl& ctrl) noexcept;
        // kill focus of control,
        void KillFocus(UIControl& ctrl) noexcept;
        // set now default control
        void SetDefault(UIControl& ctrl) noexcept;
        // reset window default control(set window-default control to default)
        void ResetDefault() noexcept;
        // reset window default control to null
        void NullDefault() noexcept;
        // Invalidate control
        void InvalidateControl(UIControl&, const RectF* r=nullptr) noexcept;
        // will do full render this frame?
        bool IsFullRenderThisFrame() const noexcept;
        // focus prev
        bool FocusPrev() noexcept;
        // focus next
        bool FocusNext() noexcept;
        // after control invisible
        void DoControlInvisible(UIControl&) noexcept;
    public:
        // end iterator
        auto end()noexcept->Iterator { return{ static_cast<CUIWindow*>(&m_oTail) }; }
        // begin iterator
        auto begin()noexcept->Iterator { return{ static_cast<CUIWindow*>(m_oHead.next) }; }
    public:
        // update focus rect
        void UpdateFocusRect(const RectF& rect) noexcept;
        // delete later
        void DeleteLater() noexcept;
        // delete now
        void Delete() noexcept;
        // set control world changed
        void SetControlWorldChanged(UIControl&) noexcept;
        // prepare render
        void PrepareRender() noexcept;
        // render
        auto Render() noexcept->Result;
        // Recreate
        auto RecreateDeviceData() noexcept->Result;
        // release window device data
        void ReleaseDeviceData() noexcept;
        // get const platform
        auto&RefPlatform() const noexcept { return reinterpret_cast<const CUIPlatform&>(m_platform); }
        // get viewport
        auto RefViewport() noexcept ->UIViewport&;
        // get viewport
        auto RefViewport() const noexcept->const UIViewport& { return const_cast<CUIWindow*>(this)->RefViewport(); }
        // get parent
        auto GetParent() const noexcept { return m_pParent; }
        // set native icon data [MUST CALL AFTER SHOUWINDOW]
        void SetNativeIconData(const wchar_t*, uintptr_t big=0) noexcept;
        // is inline window
        bool IsInlineWindow() const noexcept { return false; }
    protected:
        // get platform
        auto&platform() noexcept { return reinterpret_cast<CUIPlatform&>(m_platform); }
        // get screen lt
        template<typename T>auto screen_lt() const noexcept -> Point<T>;
        // init
        void init() noexcept;
        // recursive set result
        void recursive_set_result(uintptr_t result) noexcept;
        // add child
        void add_child(CUIWindow& child) noexcept;
        // remove child
        //void remove_child(CUIWindow& child) noexcept;
        // ctor
        CUIWindow(CUIWindow* parent, WindowConfig cfg) noexcept;
        // no copy
        CUIWindow(const CUIWindow&) noexcept = delete;
        // dtor
        ~CUIWindow() noexcept;
    private:
        // head
        Node<CUIWindow>     m_oHead;
        // tail
        Node<CUIWindow>     m_oTail;
        // all list node
        Node<AllWindows>    m_oListNode;
    protected:
        // window handle
        //HWND                m_hwnd = nullptr;
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // style sheet
        CUIStyleSheet*      m_pStyleSheet = nullptr;
#endif
        // parent window
        CUIWindow*          m_pParent = nullptr;
        // topest world changed control
        UIControl*          m_pMiniWorldChange = nullptr;
#ifdef LUI_ACCESSIBLE
        // accessible
        CUIAccessibleWnd*   m_pAccessible = nullptr;
#endif
    public:
        // custom script data
        void*               custom_script = nullptr;
        // config
        WindowConfig  const config;
    protected:
        // in dtor
        bool                m_inDtor : 1;
        // in exec
        bool                m_bInExec : 1;
        // state: under "minsize changed" list
        //bool                m_bMinsizeList = false;
    protected:
        // private data
        std::aligned_storage<
            impl::platform<sizeof(void*)>::size,
            impl::platform<sizeof(void*)>::align
        >::type                 m_platform;
    };
    // WindowConfig | WindowConfig
    inline CUIWindow::WindowConfig operator|(
        CUIWindow::WindowConfig a, CUIWindow::WindowConfig b) noexcept {
        using target_t = uint16_t;
        static_assert(sizeof(target_t) == sizeof(CUIWindow::WindowConfig), "same!");
        const auto aa = static_cast<target_t>(a);
        const auto bb = static_cast<target_t>(b);
        return CUIWindow::WindowConfig(aa | bb);
    }
}