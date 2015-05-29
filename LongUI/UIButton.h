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
    class LongUIAPI UIButton final : public UILabel{
        // 父类申明
        using Super = UILabel ;
    public:
        // Render 渲染 --- 放在第一位!
        virtual auto LongUIMethodCall Render() noexcept ->HRESULT override;
        // do event 事件处理
        virtual bool LongUIMethodCall DoEvent(LongUI::EventArgument&) noexcept override;
        // 预渲染
        virtual void LongUIMethodCall PreRender() noexcept override {};
        // recreate 重建
        virtual auto LongUIMethodCall Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // close this control 关闭控件
        virtual void LongUIMethodCall Close() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(pugi::xml_node) noexcept ->UIControl*;
        // register click event 注册点击事件
        LongUIInline void RegisterClickEvent(LongUICallBack call, UIControl* target) noexcept{ 
            m_eventClick = call; m_pClickTarget = target;
        };
    protected:
        // constructor 构造函数
        UIButton(pugi::xml_node) noexcept;
        // destructor 析构函数
        ~UIButton() noexcept;
        // deleted function
        UIButton(const UIButton&) = delete;
    protected:
        // backgroud color
        ID2D1SolidColorBrush*   m_pBGBrush = nullptr;
        // event target 
        UIControl*              m_pClickTarget = nullptr;
        // click event
        LongUICallBack          m_eventClick = nullptr;
        // btn node
        CUIElement              m_uiElement;
        // target status when clicked
        ControlStatus           m_tarStatusClick = LongUI::Status_Hover;
        /*// effective
        bool                    m_bEffective = false;
        bool                    btnunused[3];*/
    };
}