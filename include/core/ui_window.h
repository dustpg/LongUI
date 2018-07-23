#pragma once

// ui
#include "ui_node.h"
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
    // Cursor
    class CUICursor;
    // window manager
    class CUIWndMgr;
    // color
    struct ColorF;
    // popup type
    enum class PopupType : uint32_t;
    /// <summary>
    /// window base class
    /// </summary>
    class CUIWindow final : public CUIWindowEvent, public CUINoMo {
        // private impl
        class Private;
        // friend class
        friend class UIViewport;
        // friend class
        friend class CUIWndMgr;
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
        enum WindowConfig : uint8_t {
            // popup window
            Config_Popup = 1 << 0,
            // frameless window
            Config_Frameless = 1 << 1,
            // quit on close
            Config_QuitOnClose = 1 << 2,
            // delete on close
            Config_DeleteOnClose = 1 << 3,
            // tool window, no included in "quit on close"
            Config_ToolWindow = 1 << 4,
            // modal window, cannot control parent until closed
            Config_ModalWindow = 1 << 5,
            // fixed size, cannot drag to resize but invoke Resize()
            Config_FixedSize = 1 << 6,
            // layered window[support for Win8.1 and higher]
            Config_LayeredWindow = 1 << 7,
            // default config
            Config_Default = 0,
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
        // active window
        void ActiveWindow() noexcept;
        // is visible
        bool IsVisible() const noexcept;
        // is in dtor
        bool IsInDtor() const noexcept { return m_inDtor; }
        // is ctor failed?
        bool IsCtorFailed() const noexcept { return m_bCtorFaild; }
        // mark full rendering
        void MarkFullRendering() noexcept;
        // is auto sleep?
        bool IsAutpSleep() const noexcept { return config & Config_Popup; }
        // is in sleep mode?
        auto IsInSleepMode() const noexcept { return !m_hwnd; }
        // into sleep mode immediately
        void IntoSleepImmediately() noexcept;
        // try sleep
        void TrySleep() noexcept;
        // wake up
        void WakeUp() noexcept;
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
        void MapToScreen(Point2F& pos) const noexcept;
        // map from screen
        //void MapFromScreen(RectF& rect) const noexcept;
        // map from screen
        void MapFromScreen(Point2F& pos) const noexcept;
        // hi-dpi support
        void HiDpiSupport() noexcept;
    public:
        // show popup window
        void PopupWindow(CUIWindow& wnd, Point2F pos, PopupType type) noexcept;
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
        void SetTitleName(const wchar_t*) noexcept;
        // set title name
        void SetTitleName(CUIString&&) noexcept;
        // get title name
        auto GetTitleName() const noexcept->WcView;
        // set pos of window
        void SetPos(Point2L pos) noexcept;
        // get pos of window
        auto GetPos() const noexcept->Point2L;
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
    public:
        // register access key
        void RegisterAccessKey(UIControl& ctrl) noexcept;
        // find control, return null if notfound
        auto FindControl(const char* id) noexcept ->UIControl*;
        // control attached
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
        // set focus of control,
        bool SetFocus(UIControl& ctrl) noexcept;
        // kill focus of control,
        void KillFocus(UIControl& ctrl) noexcept;
        // set now default control
        void SetDefault(UIControl& ctrl) noexcept;
        // reset window default control(set window-default control to default)
        void ResetDefault() noexcept;
        // mark dirty rect
        void MarkDirtRect(const RectF& rect) noexcept;
        // will do full render this frame?
        bool IsFullRenderThisFrame() const noexcept;
    public:
        // delete later
        void DeleteLater() noexcept;
        // delete now
        void Delete() noexcept;
        // set control world changed
        void SetControlWorldChanged(UIControl&) noexcept;
        // before render
        void BeforeRender() noexcept;
        // render
        auto Render() noexcept->Result;
        // Recreate
        auto RecreateDeviceData() noexcept->Result;
        // release window device data
        void ReleaseDeviceData() noexcept;
        // get viewport
        auto RefViewport() noexcept ->UIViewport&;
        // get viewport
        auto RefViewport() const noexcept->const UIViewport& { 
            return const_cast<CUIWindow*>(this)->RefViewport(); }
        // get parent
        auto GetParent() const noexcept { return m_parent; }
        // get window handle
        HWND GetHwnd() const { return m_hwnd; }
        // is top level window
        bool IsTopLevel() const noexcept { return !m_parent; }
        // is inline window
        bool IsInlineWindow() const noexcept { return false; }
    protected:
        // add child
        void add_child(CUIWindow& child) noexcept;
        // remove child
        void remove_child(CUIWindow& child) noexcept;
        // recreate_device window
        auto recreate_window() noexcept->Result;
        // release window only device data
        void release_window_only_device() noexcept;
        // ctor
        CUIWindow(CUIWindow* parent, WindowConfig cfg) noexcept;
        // no copy
        CUIWindow(const CUIWindow&) noexcept = delete;
        // dtor
        ~CUIWindow() noexcept;
    private:
        // private data
        Private*            m_private = nullptr;
    protected:
        // window handle
        HWND                m_hwnd = nullptr;
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // style sheet
        CUIStyleSheet*      m_pStyleSheet = nullptr;
#endif
        // parent window
        CUIWindow*          m_parent = nullptr;
        // topest world changed control
        UIControl*          m_pTopestWcc = nullptr;
#ifdef LUI_ACCESSIBLE
        // accessible
        CUIAccessibleWnd*   m_pAccessible = nullptr;
#endif
    public:
        // config
        WindowConfig  const config;
    protected:
        // in dtor
        bool                m_inDtor = false;
        // ctor failed
        bool                m_bCtorFaild = false;
        // state: under "minsize changed" list
        bool                m_bMinsizeList = false;
    };
    // WindowConfig | WindowConfig
    inline CUIWindow::WindowConfig operator|(
        CUIWindow::WindowConfig a, CUIWindow::WindowConfig b) noexcept {
        using target_t = uint8_t;
        static_assert(sizeof(target_t) == sizeof(CUIWindow::WindowConfig), "same!");
        const auto aa = static_cast<target_t>(a);
        const auto bb = static_cast<target_t>(b);
        return CUIWindow::WindowConfig(aa | bb);
    }
}