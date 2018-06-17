#pragma once

// ui
#include "ui_node.h"
#include "ui_object.h"
#include "ui_basic_type.h"
#include "../util/ui_ostype.h"
#include "../accessible/ui_accessible.h"
// c++
#include <cstddef>

namespace LongUI {
    // Cursor
    class CUICursor;
    // control
    class UIControl;
    // window manager
    class CUIWndMgr;
    // color
    struct ColorF;
    /// <summary>
    /// window base class
    /// </summary>
    class CUIWindow final : public CUISmallObject/*, protected Node*/ {
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
            // borderless window
            Config_Borderless = 1 << 1,
            // quit on close
            Config_QuitOnClose = 1 << 2,
            // delete on close
            Config_DeleteOnClose = 1 << 3,
            // tool window, no included in [quit on close]
            Config_ToolWindow = 1 << 4,
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
        // is ctor failed?
        bool IsCtorFailed() const noexcept { return m_bCtorFaild; }
        // mark full rendering
        void MarkFullRendering() noexcept;
    public:
        // map to screen
        void MapToScreen(RectF& rect) const noexcept;
        // map to screen
        void MapToScreen(Point2F& pos) const noexcept;
        // map from screen
        //void MapFromScreen(RectF& rect) const noexcept;
        // map from screen
        void MapFromScreen(Point2F& pos) const noexcept;
    public:
        // show popup window
        void PopupWindow(CUIWindow& wnd, Point2F pos) noexcept;
        // close popupwindow
        void ClosePopup() noexcept;
        // set title name
        void SetTitleName(const wchar_t*) noexcept;
        // set pos of window
        void SetPos(Point2L pos) noexcept;
        // get pos of window
        auto GetPos() const noexcept->Point2L;
        // resize widow
        void Resize(Size2L size) noexcept;
        // set color color
        void SetClearColor(const ColorF&) noexcept;
        // set now cursor
        void SetNowCursor(const CUICursor&) noexcept;
        // set now cursor to defualt
        void SetNowCursor(std::nullptr_t) noexcept;
    public:
        // find control, return null if notfound
        auto FindControl(const char* id) noexcept ->UIControl*;
        // control attached
        void ControlAttached(UIControl& ctrl) noexcept;
        // control disattached              [null this ptr acceptable]
        void ControlDisattached(UIControl& ctrl) noexcept;
        // add named control                [null this ptr acceptable]
        void AddNamedControl(UIControl& ctrl) noexcept;
        // set captured control
        void SetCapture(UIControl& ctrl) noexcept;
        // release captured control, return true if released
        bool ReleaseCapture(UIControl& ctrl) noexcept;
        // set focus of control,
        bool SetFocus(UIControl& ctrl) noexcept;
        // kill focus of control,
        void KillFocus(UIControl& ctrl) noexcept;
        // set now defualt control
        void SetDefualt(UIControl& ctrl) noexcept;
        // reset window defualt control(set window-defualt control to defualt)
        void ResetDefualt() noexcept;
        // mark dirty rect
        void MarkDirtRect(const RectF& rect) noexcept;
        // will do full render this frame?
        bool IsFullRenderThisFrame() const noexcept;
    public:
        // delete
        void Delete() noexcept;
        // set control world changed
        void SetControlWorldChanged(UIControl&) noexcept;
        // before render
        void BeforeRender() noexcept;
        // render
        auto Render() const noexcept->Result;
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
        // recreate
        auto recreate() noexcept->Result;
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
}