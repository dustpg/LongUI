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

// super
#include "ui_box.h"
// base
#include <control/ui_image.h>
#include <control/ui_label.h>

// ui namespace
namespace LongUI {
    // image control
    class UIImage;
    // checkbox
    class UICheckBox : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
    public:
        // on commnad event
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
    public:
        // set indeterminate
        void SetIndeterminate() noexcept;
        // set checked
        void SetChecked(bool checked) noexcept;
        // set image source
        void SetImageSource(U8View src) noexcept;
        // get checked
        auto IsChecked() const noexcept { return m_oStyle.state & State_Checked; }
        // get indeterminate
        auto IsIndeterminate() const noexcept { return m_oStyle.state & State_Indeterminate; }
        // toggle this
        void Toggle() noexcept { return this->SetChecked(!this->IsChecked()); }
        // get text
        auto GetText() const noexcept ->const char16_t*;
        // ref text- string object
        auto RefText() const noexcept -> const CUIString&;
        // set text
        void SetText(const CUIString& text) noexcept;
        // set text
        void SetText(CUIString&& text) noexcept;
        // set text
        void SetText(U16View text) noexcept;
    protected:
        // ctor
        UICheckBox(const MetaControl& ) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UICheckBox() noexcept;
        // ctor
        explicit UICheckBox(UIControl* parent = nullptr) noexcept : UICheckBox(UICheckBox::s_meta) { this->final_ctor(parent); }
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // input event
        auto DoInputEvent(InputEventArg e) noexcept->EventAccept override;
    public:
        // update this
        void Update(UpdateReason) noexcept override;
#ifdef LUI_DRAW_FOCUS_RECT
        // trigger
        auto TriggerEvent(GuiEvent event) noexcept->EventAccept override;
        // update
        void UpdateFocusRect() const noexcept;
#endif
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // set indeterminate
        void change_indeterminate(bool) noexcept;
        // change state
        void changed() noexcept;
    private: // private control impl
        // private image
        UIImage                 m_oImage;
        // private label
        UILabel                 m_oLabel;
    private:
        // image child
        UIImage*                m_pImageChild = nullptr;
        // new image added
        bool                    m_bNewImage = false;
    };
    // get meta info for UICheckBox
    LUI_DECLARE_METAINFO(UICheckBox);
}