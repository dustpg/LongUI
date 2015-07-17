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
    class UIButton final : public UILabel {
        // 父类申明
        using Super = UILabel;
        // ui element
        using ButtonElement = Component::Elements<Element::Meta, Element::BrushRect, Element::Basic>;
    public:
        // Render 渲染 
        virtual void Render(RenderType type) const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
        // recreate 重建
        virtual auto Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // close this control 关闭控件
        virtual void WindUp() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // register click event 注册点击事件
        LongUIInline void RegisterClickEvent(LongUICallBack call, UIControl* target) noexcept { 
            m_eventClick = call; m_pClickTarget = target;
        };
    protected:
        // constructor 构造函数
        UIButton(pugi::xml_node) noexcept;
        // destructor 析构函数
        ~UIButton() noexcept {};
        // deleted function
        UIButton(const UIButton&) = delete;
    protected:
        // event target 
        UIControl*              m_pClickTarget = nullptr;
        // click event
        LongUICallBack          m_eventClick = nullptr;
        // element
        ButtonElement           m_uiElement;
        // target status when clicked
        ControlStatus           m_tarStatusClick = LongUI::Status_Hover;
        /*// effective
        bool                    m_bEffective = false;
        bool                    btnunused[3];*/
    };
}