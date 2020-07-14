#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
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

#include "../core/ui_string.h"
#include "../graphics/ui_cursor.h"
#include "../core/ui_popup_window.h"
#include "../core/ui_control_control.h"
#include "../graphics/ui_graphics_decl.h"

#if 1
namespace LongUI {
    // viewport
    class UIViewport;
    // window
    class CUIWindow;
    // mouse
    struct MouseEventArg;
    // dirty rect
    struct DirtyRect {
        // control
        UIControl*          control;
        // rectangle
        RectF               rectangle;
    };
    // longui base platform
    class CUIPlatform  {
        // friend class
        friend CUIWindow;
    protected:
        // get window
        auto window() noexcept->CUIWindow*;
        // close window
        void close_window() noexcept;
        // get viewport
        auto viewport() noexcept->UIViewport*;
        // resize viewport
        void resize_viewport(Size2F) noexcept;
        // toggle ACCESS key
        void toggle_access_key_display() noexcept;
        // draw basic caret
        void basic_caret(I::Renderer2D& renderer) const noexcept;
        // draw basic focus
        void basic_focus(I::Renderer2D& renderer) const noexcept;
        // full-rendering this frame?
        bool is_fr_for_render() const noexcept { return dirty_count_presenting == DIRTY_RECT_COUNT + 1; }
        // will render in this frame?
        auto is_r_for_render() const noexcept { return dirty_count_presenting; }
        // mark as full-rendering
        void mark_fr_for_render() noexcept { dirty_count_presenting = DIRTY_RECT_COUNT + 1; }
        // mark as full-rendering
        void clear_fr_for_render() noexcept { dirty_count_presenting = 0; }
        // full-rendering this frame?
        bool is_fr_for_update() const noexcept { return full_rendering_update; }
        // mark as full-rendering
        void mark_fr_for_update() noexcept { full_rendering_update = true; }
        // clear full-rendering 
        void clear_fr_for_update() noexcept { full_rendering_update = false; }
    public:
        // ctor
        CUIPlatform() noexcept;
        // dtor
        ~CUIPlatform() noexcept;
    public:
        // get first of weak ref pointer list
        auto GetFirst() noexcept { return &this->focused; }
        // get last of weak ref pointer list
        auto GetLast() noexcept { return &this->wnd_default; }
        // [thread safe] mouse event
        void DoMouseEventTs(const MouseEventArg & args) noexcept;
        // close popup window
        void ClosePopup() noexcept;
        // when key down/ up
        void OnKey(InputEventArg arg) noexcept;
        // [thread safe]when menu key
        void OnMenuTs() noexcept;
        // on access key
        void OnAccessKey(uintptr_t) noexcept;
        // when input a utf-16 char
        void OnChar(char16_t ch) noexcept;
        // when input a utf-32 char[thread safe]
        void OnCharTs(char32_t ch) noexcept;
        // on dpi changed
        void OnDpiChanged(Size2F zoom, const RectL& rect) noexcept;
        // before render
        void PrepareRender(Size2F) noexcept;
        // mark one dirty rect
        void MarkDirtyRect(const DirtyRect& rect) noexcept;
        //// map to screen
        //void MapToScreen(RectF& rect) const noexcept;
        //// map to screen
        //void MapToScreen(RectL& rect) const noexcept;
        //// map from screen
        //void MapFromScreen(Point2F& pos) const noexcept;
    public:
        // init
        void Init(CUIWindow* parent, uint16_t flag) noexcept;
        // render
        auto Render() noexcept->Result;
        // dispose
        void Dispose() noexcept;
        // release device data
        void ReleaseDeviceData() noexcept;
        // recreate
        auto Recreate() noexcept->Result;
        // after titlename set
        void AfterTitleName() noexcept;
        // after position set
        void AfterPosition() noexcept;
        // after absrect set
        void AfterAbsRect() noexcept;
        // close window
        void CloseWindow() noexcept;
        // show window
        void ShowWindow(int) noexcept;
        // resize
        void ResizeAbsolute(Size2L) noexcept;
        // work area that this window worked
        auto GetWorkArea() const noexcept->RectL;
        // get raw handle
        auto GetRawHandle() const noexcept->uintptr_t;
        // enable the window
        void EnableWindow(bool enable) noexcept;
        // active window(the focus to window self)
        void ActiveWindow() noexcept;
    public:
        // title name
        CUIString       titlename;
        // window clear color
        ColorF          clear_color /*= ColorF::FromRGBA_CT<RGBA_TianyiBlue>()*/;
        // color of caret
        ColorF          caret_color/* = ColorF::FromRGBA_CT<RGBA_Black>()*/;
        // rect of window
        RectWHL         rect = {};
        // adjust(border) rect of window 
        RectL           adjust = { };
        // rect of caret
        RectF           caret = {};
        // rect of foucs area
        RectF           foucs = {};
        // now cursor
        CUICursor       cursor = { CUICursor::Cursor_Arrow };
        // focused control
        UIControl*      focused = nullptr;
        // captured control
        UIControl*      captured = nullptr;
        // caret display control
        UIControl*      careted = nullptr;
        // now default control
        UIControl*      now_default = nullptr;
        // window default control
        UIControl*      wnd_default = nullptr;
        // focus list
        ControlNode     focus_list = { nullptr };
        // named list
        ControlNode     named_list = { nullptr };
        // popup window
        CUIWindow*      popup = nullptr;
        // common tooltip viewport
        UIViewport*     common_tooltip = nullptr;
        // ime count
        uint16_t        ime_count = 0;
        // saved utf16 char
        char16_t        saved_utf16 = 0;
        // popup type
        PopupType       popup_type = PopupType::Type_Exclusive;
        // visible
        bool            visible = false;
        // draw focus
        bool            draw_focus = false;
        // draw caret
        bool            draw_caret = false;
        // full renderer in update
        bool            full_rendering_update = false;
        // draw caret
        //bool            draw_caret = false;
        // access key display
        bool            access_key_display = false;
        // dirty count for recording
        uint16_t        dirty_count_recording = 0;
        // dirty count for presenting
        uint16_t        dirty_count_presenting = 0;
        // dirty rect for recording
        DirtyRect       dirty_rect_recording[LongUI::DIRTY_RECT_COUNT];
        // dirty rect for presenting [+ 2 for safty]
        RectF           dirty_rect_presenting[LongUI::DIRTY_RECT_COUNT + 2];
        // access key map
        UIControl*      access_key_map['Z' - 'A' + 1];
    };
}
#endif