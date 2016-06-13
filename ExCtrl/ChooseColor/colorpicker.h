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

struct _D3DCOLORVALUE;
using D2D1_COLOR_F = _D3DCOLORVALUE;
#include <Platless/luiPlUtil.h>
#include <cstdint>
#include <cassert>

// longui namespace
namespace LongUI {
    // color data
    struct ColorPickerF { float color[4]; };
    // control
    class UIControl;
    // viewport
    class UIViewport;
    // color button
    class UIColorButton;
    // window
    class XUIBaseWindow;
    // color picked callback; color -> null -> on cancel, not null -> on ok
    using CUIColorPicked = CUIFunction<bool(const D2D1_COLOR_F* color)>;
    // color picker
    class CUIColorPicker final {
    public:
        // frend class
        friend class UIPickerView;
        // saved color count
        enum : unsigned { SAVED_COLOR_COUNT = 12 };
        // index for text-ed control
        enum ControlIndex : unsigned {
            // Cancel
            Index_OK = 0,
            // OK
            Index_Cancel,
            // Reset
            Index_Reset,
            // now
            Index_Now,
            // old
            Index_Old,
            // html format
            Index_Html,
            // count of this
            INDEX_COUNT,
        };
    public:
        // ctor
        CUIColorPicker() noexcept;
        // dtor
        ~CUIColorPicker() noexcept;
        // to bool
        operator bool() const noexcept { return !!m_pWindow; }
        // set color picked callback
        void SetCallback(CUIColorPicked&& onok) noexcept {
            m_uiCallback = std::move(onok);
        }
        // set color picked callback
        template<typename T>void SetCallback(T onok) noexcept {
            this->SetCallback(CUIColorPicked(onok));
        }
        // create window, return false if failed
        bool Create(XUIBaseWindow* parent) noexcept;
        // set window title name
        void SetTitleName(const wchar_t* str) noexcept;
        // set text
        void SetText(ControlIndex index, const wchar_t* text) noexcept;
        // show window
        void ShowWindow(int nCmdShow) noexcept;
        // pick color
        void PickColor() noexcept { this->ShowWindow(1); }
        // preview the color
        //void Preview(D2D1_COLOR_F&) noexcept;
        // close window
        void CloseWindow() noexcept;
        // set init color
        void SetInitColor(const D2D1_COLOR_F&) noexcept;
        // set old color
        void SetOldColor(const D2D1_COLOR_F&) noexcept;
        // get saved color
        void SetSavedColor(int i, D2D1_COLOR_F& c) const noexcept {
            assert(i >= 0 && i < SAVED_COLOR_COUNT && "out of range");
            reinterpret_cast<ColorPickerF&>(c) = m_aSaveColor[i];
        }
        // set saved color
        void SetSavedColor(int i, const D2D1_COLOR_F& c) noexcept {
            assert(i >= 0 && i < SAVED_COLOR_COUNT && "out of range");
            m_aSaveColor[i] = reinterpret_cast<const ColorPickerF&>(c);
            this->refresh_saved_color(i);
        }
    private:
        // set old color
        void set_old_color(const D2D1_COLOR_F&) noexcept;
        // refresh saved color
        void refresh_saved_color(int index) noexcept;
    private:
        // ok callback
        CUIColorPicked          m_uiCallback;
        // window
        XUIBaseWindow*          m_pWindow = nullptr;
        // old color
        ColorPickerF            m_colorOld = { 0.f, 0.f, 0.f, 1.f };
        // texted control
        UIControl*              m_apTexted[INDEX_COUNT];
        // saved color control
        UIColorButton*          m_apColorCtrl[SAVED_COLOR_COUNT];
        // save color
        ColorPickerF            m_aSaveColor[SAVED_COLOR_COUNT];
    };
}
