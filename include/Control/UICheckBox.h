#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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

// LongUI namespace
namespace LongUI {
    // default checkBox control 默认复选框控件
    class UICheckBox final : public UIText {
        // super class
        using Super = UIText ;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // box size
        static constexpr float BOX_SIZE = 16.f;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const EventArgument& arg) noexcept override;
        // do mouse event 鼠标事件处理
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept;
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // set control state
        void SetControlState(ControlState state) noexcept { m_pWindow->StartRender(m_uiElement.SetBasicState(state), this); }
        // get control state
        auto GetControlState() const noexcept { return m_uiElement.GetNowBaiscState(); }
        // set checkbox state
        void SetCheckBoxState(CheckBoxState state) noexcept { m_pWindow->StartRender(m_uiElement.SetExtraState(state), this); }
        // get checkbox state
        auto GetCheckBoxState() const noexcept { return m_uiElement.GetNowExtraState(); }
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // constructor 构造函数
        UICheckBox(UIContainer* cp) noexcept : Super(cp) {}
    protected:
        // init
        void initialize(pugi::xml_node node) noexcept;
        // destructor 析构函数
        ~UICheckBox() noexcept;
        // deleted function
        UICheckBox(const UICheckBox&) = delete;
    protected:
        // hand cursor
        HCURSOR                     m_hCursorHand = ::LoadCursor(nullptr, IDC_HAND);
        // check box's box size
        D2D1_SIZE_F                 m_szCheckBox = D2D1::SizeF(16, 16);
        // element
        Component::Element4Checkbox m_uiElement;
    };
}