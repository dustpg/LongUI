#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../LongUI/luiUiStrAl.h"
#include "../Platless/luiPlEzC.h"
#include "../Platonly/luiPoHlper.h"
#include "../Platless/luiPlHlper.h"

#include <cstdint>
#include <../3rdParty/pugixml/pugixml.hpp>
#include <Control/UIViewport.h>
//#include <Core/luiManager.h>


// LongUI namespace
namespace LongUI {
    // vector for window
    using WindowVector = EzContainer::PointerVector<XUIBaseWindow>;
    // vector for system window
    using SystemWindowVector = EzContainer::PointerVector<XUIBaseWindow>;
    // config for create window
    namespace Config { struct Window {
        // node of window, maybe null
        pugi::xml_node      node;
        // parent for window, maybe null
        XUIBaseWindow*      parent;
        // width of window
        uint32_t            width;
        // height of window
        uint32_t            height;
        // is popup?
        bool                popup;
        // create system window first?
        bool                system;
        // unused
        //bool                unused[2];
    };}
    // window for longui
    class XUIBaseWindow {
        // super class
        using Super = void;
        // create ui viewport call back
        using callback_create_viewport = auto(*)(
            pugi::xml_node node, XUIBaseWindow* container
            ) ->UIViewport*;
        // create child window
        static auto create_child_window(
            pugi::xml_node node,
            XUIBaseWindow* parent,
            callback_create_viewport func
        ) noexcept->XUIBaseWindow*;
    public:
        // rect
        using RectLTWH_L = RectLTWH<LONG>;
        // string allocator
        using StrAllocator = CUIShortStringAllocator;
        // ctor
        XUIBaseWindow(const Config::Window& config) noexcept;
        // dtor
        ~XUIBaseWindow() noexcept;
        // create child window with xml string, include UIViewport.h first
        auto CreateChildWindow(const char* xml) noexcept ->XUIBaseWindow* { 
            return this->CreateChildWindow<UIViewport>(xml); 
        }
        // create child window with xml string
        template<class T> 
        auto CreateChildWindow(const char* xml) noexcept ->XUIBaseWindow* {
            pugi::xml_document doc;
            auto code = doc.load_string(xml); assert(code && "bad xml"); 
            if (code.status) return nullptr;
            return this->CreateChildWindow<T>(doc.first_child());
        }
        // create child window with xml node, include UIViewport.h first
        template<class T> 
        auto CreateChildWindow(pugi::xml_node node) noexcept ->XUIBaseWindow* {
            auto create_func = UIViewport::CreateFunc<T>;
            return this->create_child_window(node, this, create_func);
        }
        // create popup window
        auto CreatePopup(
            const D2D1_RECT_L& pos, 
            uint32_t height, 
            UIControl* child
        ) noexcept ->XUIBaseWindow*;
    public:
        // index of BitArray
        enum BitArrayIndex : uint32_t {
            // [UN]
            //Index_ExitOnClose = 0,
            Index_Sep = 0,
            // [RO] close when focus killed
            Index_CloseOnFocusKilled,
            // [RO] popup window
            Index_PopupWindow,
            // [RO] Hi-Dpi supported
            Index_HiDpiSupported,
            // [RW] in draging?
            Index_InDraging,
            // [RW] new size?
            Index_NewSize,
            // [RW] skip render
            Index_SkipRender,
            // [RW] do full-render this frame?
            Index_FullRenderThisFrame,
            // [RW] caret in
            Index_CaretIn,
            // [RW] do caret
            //Index_DoCaret,
            // [RW] do full-render this frame in render?
            Index_FullRenderThisFrameRender,
            // [XX] count of this
            INDEX_COUNT,
        };
    public:
        // dispose
        virtual void Dispose() noexcept = 0;
        // recreate: call UIControl::Render
        virtual auto Recreate() noexcept ->HRESULT;
        // render: call UIControl::Render
        virtual void Render() const noexcept;
        // update: call UIControl::AfterUpdate
        virtual void Update() noexcept;
        // move window relative to parent
        virtual void MoveWindow(int32_t x, int32_t y) noexcept = 0;
        // close window
        virtual void Close() noexcept = 0;
        // resize window
        virtual void Resize(uint32_t w, uint32_t h) noexcept = 0;
        // set cursor
        virtual void SetCursor(LongUI::Cursor cursor) noexcept = 0;
        // show window
        virtual void ShowWindow(int nCmdShow) noexcept = 0;
        // set caret
        virtual void SetCaret(UIControl* ctrl, const RectLTWH_F* rect) noexcept = 0;
        // set title name
        virtual void SetTitleName(const wchar_t* name) noexcept = 0;
    protected:
        // add inset window
        void add_inset_window(XUIBaseWindow*) noexcept;
        // remove inset window
        void remove_inset_window(XUIBaseWindow*) noexcept;
    public:
        // check control focused
        bool IsControlFocused(const UIControl* c) const noexcept { return m_pFocusedControl == c; }
        // is mouse captured control?
        auto IsControlCaptured(const UIControl* c) const noexcept { return m_pCapturedControl == c; };
        // get parent window
        auto GetParent() const noexcept { return m_pParent; }
        // is popup window?
        bool IsPopup() const noexcept { return this->is_popup_window(); }
        // hide caret
        void HideCaret(UIControl* ctrl) noexcept { this->SetCaret(ctrl, nullptr); }
        // hide window
        void HideWindow() noexcept { this->ShowWindow(SW_HIDE); }
        // reset cursor
        void ResetCursor() noexcept { this->SetCursor(Cursor::Cursor_Default); }
        // focused control
        auto GetFocused() const noexcept { return m_pFocusedControl; }
        // get window handle
        auto GetHwnd() const noexcept { return m_hwnd; }
        // get top
        auto GetTop() const noexcept { return m_rcWindow.top; }
        // get left
        auto GetLeft() const noexcept { return m_rcWindow.left; }
        // get width of window client zone
        auto GetWidth() const noexcept { return m_rcWindow.width; }
        // get height of window client zone
        auto GetHeight() const noexcept { return m_rcWindow.height; }
        // get viewport
        auto GetViewport() const noexcept { return m_pViewport; }
        // get text anti-mode 
        auto GetTextAntimode() const noexcept { return static_cast<D2D1_TEXT_ANTIALIAS_MODE>(m_textAntiMode); }
        // get text anti-mode 
        void SetTextAntimode(D2D1_TEXT_ANTIALIAS_MODE mode) noexcept { m_textAntiMode = static_cast<decltype(m_textAntiMode)>(mode); }
        // copystring for control in this winddow
        auto CopyString(const char* str) noexcept { return m_oStringAllocator.CopyString(str); }
        // copystring for control in this winddow in safe way
        auto CopyStringSafe(const char* str) noexcept { auto s = this->CopyString(str); return s ? s : ""; }
        // render window in next frame
        void InvalidateWindow() noexcept { this->set_full_render_this_frame(); }
    public:
        // add tabstop control
        void AddTabstop(UIControl* ctrl) noexcept;
        // remove tabstop control
        void RemoveTabstop(UIControl* ctrl) noexcept;
        // find control
        auto FindControl(const char* name) noexcept ->UIControl*;
        // find next tabstop control
        auto FindNextTabstop(UIControl* ctrl) const noexcept->UIControl*;
        // find prev tabstop control
        auto FindPrevTabstop(UIControl* ctrl) const noexcept->UIControl*;
        // initialize viewport
        void InitializeViewport(UIViewport* viewport) noexcept;
        // do event
        bool DoEvent(const EventArgument& arg) noexcept;
        // render control in next frame
        void Invalidate(UIControl* ctrl) noexcept;
        // set focus control, set null to kill focused-control's focus
        void SetFocus(UIControl* ctrl) noexcept;
        // set hover track control
        void SetHoverTrack(UIControl* ctrl) noexcept;
        // add control with name
        void AddNamedControl(UIControl* ctrl) noexcept;
        // set mouse capture
        void SetCapture(UIControl* control) noexcept;
        // release mouse capture
        void ReleaseCapture() noexcept;
    protected:
        // on window closed
        void on_close() noexcept;
    protected:
        // is NewSize
        bool is_new_size() const noexcept { return m_baBoolWindow.Test<Index_NewSize>(); }
        // is CaretIn
        bool is_caret_in() const noexcept { return m_baBoolWindow.Test<Index_CaretIn>(); }
        // is DoCaret
        //bool is_do_caret() const noexcept { return m_baBoolWindow.Test<Index_DoCaret>(); }
        // is SkipRender
        bool is_skip_render() const noexcept { return m_baBoolWindow.Test<Index_SkipRender>(); }
        // is Popup
        bool is_popup_window() const noexcept { return m_baBoolWindow.Test<Index_PopupWindow>(); }
        // is HiDpiSupported
        bool is_hidpi_supported() const noexcept { return m_baBoolWindow.Test<Index_HiDpiSupported>(); }
        // is ExitOnClose
        //bool is_exit_on_close() const noexcept { return m_baBoolWindow.Test<Index_ExitOnClose>(); }
        // is CloseOnFocusKilled
        bool is_close_on_focus_killed() const noexcept { return m_baBoolWindow.Test<Index_CloseOnFocusKilled>(); }
        // is FullRenderingThisFrame
        bool is_full_render_this_frame() const noexcept { return m_baBoolWindow.Test<Index_FullRenderThisFrame>(); }
        // is FullRenderingThisFrameRender
        bool is_full_render_this_frame_render() const noexcept { return m_baBoolWindow.Test<Index_FullRenderThisFrameRender>(); }
    protected:
        // set PopupWindow to true
        void set_popup_window(bool p) noexcept { m_baBoolWindow.SetTo<Index_PopupWindow>(p); }
        // set ExitOnClose to true
        //void set_exit_on_close() noexcept { m_baBoolWindow.SetTrue<Index_ExitOnClose>(); }
        // is HiDpiSupported
        void set_hidpi_supported() noexcept { m_baBoolWindow.SetTrue<Index_HiDpiSupported>(); }
        // set CloseOnFocusKilled to true
        void set_close_on_focus_killed() noexcept { m_baBoolWindow.SetTrue<Index_CloseOnFocusKilled>(); }
    protected:
        // set NewSize to true
        void set_new_size() noexcept { m_baBoolWindow.SetTrue<Index_NewSize>(); }
        // set CaretIn to true
        void set_caret_in() noexcept { m_baBoolWindow.SetTrue<Index_CaretIn>(); }
        // set DoCaret to true
        //void set_do_caret() noexcept { m_baBoolWindow.SetTrue<Index_DoCaret>(); }
        // clear CaretIn
        void clear_caret_in() noexcept { m_baBoolWindow.SetNot<Index_CaretIn>(); }
        // change CaretIn
        void change_caret_in() noexcept { m_baBoolWindow.SetNot<Index_CaretIn>(); }
        // clear NewSize
        void clear_new_size() noexcept { m_baBoolWindow.SetFalse<Index_NewSize>(); }
        // clear DoCaret
        //void clear_do_caret() noexcept { m_baBoolWindow.SetFalse<Index_DoCaret>(); }
        // set SkipRender to true
        void set_skip_render() noexcept { m_baBoolWindow.SetTrue<Index_SkipRender>(); }
        // clear SkipRender
        void clear_skip_render() noexcept { m_baBoolWindow.SetFalse<Index_SkipRender>(); }
        // set FullRenderingThisFrame to true
        void set_full_render_this_frame() noexcept { m_baBoolWindow.SetTrue<Index_FullRenderThisFrame>(); }
        // clear FullRenderingThisFrame
        void clear_full_render_this_frame() noexcept { m_baBoolWindow.SetFalse<Index_FullRenderThisFrame>();  }
    protected:
        // resized, called from child-class
        void resized() noexcept;
    protected:
        // longui viewport
        UIViewport*             m_pViewport = nullptr;
        // parent window
        XUIBaseWindow*          m_pParent = nullptr;
        // inset-children
        WindowVector            m_vInsets;
        // window handle
        HWND                    m_hwnd = nullptr;
        // TODO: mini size
        POINT                   m_miniSize;
        // tabstop controls
        ControlVector           m_vTabstops;
        // now hover track control(only one)
        UIControl*              m_pHoverTracked = nullptr;
        // now focused control (only one)
        UIControl*              m_pFocusedControl = nullptr;
        // now dragdrop control (only one)
        UIControl*              m_pDragDropControl = nullptr;
        // now captured control (only one)
        UIControl*              m_pCapturedControl = nullptr;
        // timer for caret
        Helper::Timer           m_tmCaret = Helper::Timer(::GetCaretBlinkTime());
        // window rect
        RectLTWH_L              m_rcWindow;
        // string allocator
        StrAllocator            m_oStringAllocator;
        // will use BitArray instead of them
        Helper::BitArray16      m_baBoolWindow;
        // mode for text anti-alias
        uint16_t                m_textAntiMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
        // data length of m_apUnits
        uint16_t                m_uUnitLength = 0;
        // data length of m_apUnits, in render
        uint16_t                m_uUnitLengthRender = 0;
        // data for unit
        UIControl*              m_apUnit[LongUIDirtyControlSize];
        // data for unit, in render
        UIControl*              m_apUnitRender[LongUIDirtyControlSize];
        // dirty rects
        //RECT                    m_dirtyRects[LongUIDirtyControlSize];
        // current STGMEDIUM: begin with DWORD
        STGMEDIUM               m_curMedium;
        // control name ->map-> control pointer
        StringTable             m_hashStr2Ctrl;
    public:
        // debug info
#ifdef _DEBUG
        bool                    test_D2DERR_RECREATE_TARGET = false;
        bool                    debug_show = false;
        bool                    debug_unused[6];
        uint32_t                full_render_counter = 0;
        uint32_t                dirty_render_counter = 0;
#endif
        // last mouse point
        D2D1_POINT_2F           last_point = D2D1_POINT_2F{-1.f, -1.f};
        // clear color
        D2D1::ColorF            clear_color = D2D1::ColorF(D2D1::ColorF::White);
    };
    // system window
    class XUISystemWindow : public XUIBaseWindow {
        // super class
        using Super = XUIBaseWindow;
    public:
        // ctor
        XUISystemWindow(const Config::Window& config) noexcept;
        // dtor
        ~XUISystemWindow() noexcept;
    public:
        // move window
        virtual void MoveWindow(int32_t x, int32_t y) noexcept override;
        // resize window
        //virtual void Resize(uint32_t w, uint32_t h) noexcept override;
        // set title name
        virtual void SetTitleName(const wchar_t* name) noexcept override;
    protected:
        // message id for TaskbarBtnCreated
        static const UINT s_uTaskbarBtnCreatedMsg;
    };
    // create builtin window
    auto CreateBuiltinWindow(const Config::Window& config) noexcept ->XUIBaseWindow*;
}