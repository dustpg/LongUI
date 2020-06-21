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

// ui header
#include "ui_label.h"
#include "ui_textfield.h"
#include "ui_scrollarea.h"
#include "../style/ui_text.h"
#include "../core/ui_string.h"
#include "../core/ui_core_type.h"
#include "../text/ui_text_layout.h"
#include "../core/ui_const_sstring.h"
#include "../util/ui_named_control.h"

// RichED
#include "../../RichED/ed_txtplat.h"
// TextBC
//#include <../TextBC/bc_txtplat.h>

// cursor
#include <graphics/ui_cursor.h>
// C++
#include <type_traits>

// longui namespace
namespace LongUI {
    // spin buttons
    class UISpinButtons;
    // textbox
    class UITextBox : public UIScrollArea {
        // super class
        using Super = UIScrollArea;
        // private impl
        struct Private;
    protected:
        // ctor
        UITextBox(const MetaControl&) noexcept;
    public:
        // When [pressed enter key, or killed-focus] if text changed
        static constexpr auto _onChange() noexcept { return GuiEvent::Event_OnChange; }
        // This event is sent when a user enters text in a textbox
        static constexpr auto _onInput() noexcept { return GuiEvent::Event_OnInput; }
        // selection changed
        //static inline constexpr auto _selectionChanged() noexcept { return GuiEvent::Event_Select; }
    public:
        // Gui OP - SelectAll
        bool GuiSelectAll() noexcept { return m_oTextField.GuiSelectAll(); }
        // Gui OP - Undo
        bool GuiUndo() noexcept { return m_oTextField.GuiUndo(); }
        // Gui OP - Redo
        bool GuiRedo() noexcept { return m_oTextField.GuiRedo(); }
        // Gui OP - Copy to Clipboard
        bool GuiCopyCut(bool cut) noexcept { return m_oTextField.GuiCopyCut(cut); }
        // Gui OP - Paste from Clipboard
        bool GuiPaste() noexcept { return m_oTextField.GuiPaste(); }
    public:
        // can do 'Copy'
        bool CanCopy() const noexcept { return m_oTextField.CanCopy(); }
        // can do 'Cut'
        bool CanCut() const noexcept { return m_oTextField.CanCut(); }
#if 0
        // can do 'Redo'
        bool CanRedo() const noexcept { return m_oTextField.CanRedo(); }
        // can do 'Redo'
        bool CanUndo() const noexcept { return m_oTextField.CanUndo(); }
#endif
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITextBox() noexcept;
        // ctor
        explicit UITextBox(UIControl* parent = nullptr) noexcept : UITextBox(UITextBox::s_meta) { this->final_ctor(parent); }
        // type
        enum TextBoxType : uint8_t {
            // type:    normal
            Type_Normal     = 0,
            // type:    number
            Type_Number,
            // type:    password
            Type_Password,
        };
    public:
        // fire event
        auto FireEvent(GuiEvent event) noexcept->EventAccept override;
        // normal event
        auto DoEvent(UIControl*, const EventArg& e) noexcept->EventAccept override;
        // update this
        void Update(UpdateReason reason) noexcept override;
        // render
        //void Render() const noexcept override;
        // mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept override;
        // input event
        auto DoInputEvent(InputEventArg e) noexcept->EventAccept override;
    protected:
        // initialize
        void initialize() noexcept override;
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
#ifdef LUI_ACCESSIBLE
        // accessible api
        auto accessible(const AccessibleEventArg& args) noexcept->EventAccept override;
#endif
        // need relayout
        auto is_need_relayout() const noexcept { return m_state.reason & Reason_BasicRelayout; }
        // event from textfield
        auto event_from_textfield(GuiEvent) noexcept->EventAccept;
        // relayout textbox
        void relayout_textbox() noexcept;
        // do the wheel event
        auto do_wheel(int index, float wheel) noexcept->EventAccept;
        // make UISpinButtons
        void make_spin() noexcept;
        // init minsize
        void update_minsize() noexcept;
    public:
        // set text
        void SetText(CUIString&& text) noexcept;
        // set text
        void SetText(const CUIString& text) noexcept;
        // set text
        void SetText(U16View view) noexcept;
        // request text, not const method
        auto RequestText() noexcept -> const CUIString&;
        // get double value
        auto GetValueAsDouble() noexcept -> double;
        // get double value
        auto SetValueAsDouble(double, bool increase = false) noexcept ->EventAccept;
    public:
        // max value
        double                  max_value/* = INFINITY*/;
        // min value
        double                  min_value = 0;
        // increment value
        double                  increment = 1;
    private:
        // text field
        UITextField             m_oTextField;
        // place holder
        UILabel                 m_oPlaceHolder;
        // to save memory
        UISpinButtons*          m_pSpinButtons = nullptr;
        // cols/size
        uint32_t                m_uCols = 20;
        // rows
        uint32_t                m_uRows = 1;
        // need update minsize
        bool                    m_bNeedMinsize = true;
        // decimal symbol
        char                    m_chDecimal = '.';
        // decimal places
        uint8_t                 m_uDecimalPlaces = 0;
        // type
        TextBoxType             m_type = UITextBox::Type_Normal;

    };
    // get meta info for UITextBox
    LUI_DECLARE_METAINFO(UITextBox);
}