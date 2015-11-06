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
namespace LongUI{
    // default button control 默认按钮控件
    class UIButton : public UIText {
        // 父类申明
        using Super = UIText;
        // ui element
        using ButtonElement = Component::Elements<Element_Meta, Element_BrushRect, Element_Basic>;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event
        virtual bool DoEvent(const EventArgument& arg) noexcept override;
        // do mouse event
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // ui call
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // render chain -> background
        void render_chain_background() const noexcept;
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // constructor 构造函数
        UIButton(UIContainer* cp, pugi::xml_node) noexcept;
    protected:
        // destructor 析构函数
        ~UIButton() noexcept {};
        // deleted function
        UIButton(const UIButton&) = delete;
    protected:
        // callback
        UICallBack              m_event;
        // color of border
        D2D1_COLOR_F            m_aBorderColor[STATUS_COUNT];
        // element
        ButtonElement           m_uiElement;
        // target status when clicked
        ControlStatus           m_tarStatusClick = LongUI::Status_Hover;
        /*// effective
        bool                    m_bEffective = false;
        bool                    btnunused[3];*/
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIButton>() {
        // {90098AB1-4C9E-4F16-BF5E-9179B2B29570}
        static const GUID IID_LongUI_UIButton = {
            0x90098ab1, 0x4c9e, 0x4f16,{ 0xbf, 0x5e, 0x91, 0x79, 0xb2, 0xb2, 0x95, 0x70 } 
        };
        return IID_LongUI_UIButton;
    }
#endif
}