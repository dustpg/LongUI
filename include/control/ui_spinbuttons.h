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
#include "ui_image.h"

// ui namespace
namespace LongUI {
    // test control
    class UISpinButtons final : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    protected:
        // ctor
        UISpinButtons(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UISpinButtons() noexcept override;
        // ctor
        explicit UISpinButtons(UIControl* parent = nullptr) noexcept : UISpinButtons(UISpinButtons::s_meta) { this->final_ctor(parent); }
    public:
        // when decrease button pressed if vaild
        static constexpr auto _onDecrease() noexcept { return GuiEvent::Event_OnDecrease; }
        // when increase button pressed if vaild
        static constexpr auto _onIncrease() noexcept { return GuiEvent::Event_OnIncrease; }
        // set decrease button enable
        void SetDecreaseDisabled(bool value) noexcept { m_oDecreaseButton.SetDisabled(value); }
        // set increase button enable
        void SetIncreaseDisabled(bool value) noexcept { m_oIncreaseButton.SetDisabled(value); }
    public:
        // do normal event
        //auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        //void Update(UpdateReason reason) noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        //void Render() const noexcept override;
        // recreate/init device(gpu) resource
        //auto Recreate(bool release_only) noexcept->Result override;
    protected:
        // add child
        //void add_child(UIControl&) noexcept override;
    private: // private impl
        // increase button
        UIImage             m_oIncreaseButton;
        // decrease button
        UIImage             m_oDecreaseButton;
    };
    // get meta info for UISpinButtons
    LUI_DECLARE_METAINFO(UISpinButtons);
}
